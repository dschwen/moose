#include "StrongFormLowerer.h"
#include "MooseError.h"
#include "MooseUtils.h"

namespace moose
{
namespace automatic_weak_form
{

StrongFormLowerer::Lowered StrongFormLowerer::lowerToWeak(const NodePtr & expr)
{
  if (_verbose)
  {
    trace(std::string("Lowering expression to weak form: ") + (expr ? expr->toString() : std::string("<null>")));
  }
  auto out = lower(expr);
  // Ensure shapes: c0 scalar, c1 vector
  if (!out.c0)
    out.c0 = constant(0.0);
  if (!out.c1)
  {
    // Build zero vector in requested dimension
    std::vector<NodePtr> comps(_dim, constant(0.0));
    out.c1 = std::make_shared<VectorAssemblyNode>(comps, Shape(VectorShape{_dim}));
  }
  return out;
}

StrongFormLowerer::Lowered StrongFormLowerer::lower(const NodePtr & node)
{
  Lowered res;
  if (!node)
  {
    res.c0 = constant(0.0);
    return res;
  }

  switch (node->type())
  {
    case NodeType::Negate:
    {
      // Handle unary minus explicitly so patterns like -div(V) and -laplacian(u)
      // are lowered without leaving differential operators in c0/c1.
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      const auto & op = unary->operand();

      // -div(V) => c1 += V (since we lower -∫div(V) v = ∫ V·grad(v))
      if (op->type() == NodeType::Divergence)
      {
        trace("Rule: -div(V) => c1 += V; c0 += 0");
        auto u = std::static_pointer_cast<UnaryOpNode>(op);
        res.c1 = u->operand();
        res.c0 = constant(0.0);
        return res;
      }

      // -laplacian(u) => c1 += grad(u)
      if (op->type() == NodeType::Laplacian)
      {
        trace("Rule: -laplacian(u) => c1 += grad(u); c0 += 0");
        auto u = std::static_pointer_cast<UnaryOpNode>(op);
        res.c1 = grad(u->operand(), _dim);
        res.c0 = constant(0.0);
        return res;
      }

      // -(a * div(V)) => treat as scale= a, vec=V
      if (op->type() == NodeType::Multiply)
      {
        auto bin = std::static_pointer_cast<BinaryOpNode>(op);
        const auto & a = bin->left();
        const auto & b = bin->right();

        if (b->type() == NodeType::Divergence)
        {
          trace("Rule: -(a*div(V)) => c1 += a*V; c0 += V·grad(a) (skipped unless a constant)");
          auto divu = std::static_pointer_cast<UnaryOpNode>(b);
          // IBP: -∫ a*div(V) v = ∫ (a*V)·grad(v) + ∫ (V·grad(a)) v
          res.c1 = mulSafe(a, divu->operand());
          if (!dependsOnField(a))
            res.c0 = constant(0.0);
          else
            res.c0 = constant(0.0);
          return res;
        }

        // -(div(V) * a)
        if (a->type() == NodeType::Divergence)
        {
          trace("Rule: -(div(V)*a) => c1 += a*V; c0 += V·grad(a) (skipped unless a constant)");
          auto divu = std::static_pointer_cast<UnaryOpNode>(a);
          res.c1 = mulSafe(b, divu->operand());
          if (!dependsOnField(b))
            res.c0 = constant(0.0);
          else
            res.c0 = constant(0.0);
          return res;
        }
      }

      // Generic: lower the operand and flip signs of contributions
      {
        auto inner = lower(op);
        // Negating the term flips both c0 and c1 contributions
        if (inner.c0)
          res.c0 = neg(inner.c0);
        if (inner.c1)
          res.c1 = mulSafe(constant(-1.0), inner.c1);
        trace("Rule: -(g) => c0 += -c0(g); c1 += -c1(g)");
        return res;
      }
    }

    case NodeType::Add:
    case NodeType::Subtract:
    {
      auto bin = std::static_pointer_cast<BinaryOpNode>(node);
      auto L = lower(bin->left());
      auto R = lower(bin->right());
      if (node->type() == NodeType::Add)
      {
        res.c0 = addSafe(L.c0, R.c0);
        res.c1 = addSafe(L.c1, R.c1);
      }
      else
      {
        res.c0 = subSafe(L.c0, R.c0);
        res.c1 = subSafe(L.c1, R.c1);
      }
      return res;
    }

    case NodeType::Multiply:
    {
      // Handle a * div(V) and a * laplacian(u)
      auto bin = std::static_pointer_cast<BinaryOpNode>(node);
      auto a = bin->left();
      auto b = bin->right();

      auto handle_scaled_div = [&](const NodePtr & scale, const NodePtr & vec_expr) -> Lowered {
        Lowered tmp;
        // IBP for -∫ a·div(V) v = ∫ (a·V)·∇v + ∫ (V·∇a) v
        // Our lowerer builds contributions for -expr*v, so c1 += a*V; c0 += V·grad(a)
        trace("Rule: a*div(V) => c1 += a*V; c0 += V·grad(a) (grad(a) skipped unless a constant)");
        tmp.c1 = mulSafe(scale, vec_expr);

        // grad(scale) available only if scale independent of fields (then zero) – otherwise skip
        if (!dependsOnField(scale))
        {
          // grad(constant/parameter) = 0 => no c0 term
          tmp.c0 = constant(0.0);
        }
        else
        {
          // We cannot evaluate grad(scale) generically yet; skip safely.
          // Users should keep coefficients constant for now.
          tmp.c0 = constant(0.0);
        }
        return tmp;
      };

      // a * div(V)
      if (b->type() == NodeType::Divergence)
      {
        auto unary = std::static_pointer_cast<UnaryOpNode>(b);
        return handle_scaled_div(a, unary->operand());
      }
      // div(V) * a
      if (a->type() == NodeType::Divergence)
      {
        auto unary = std::static_pointer_cast<UnaryOpNode>(a);
        return handle_scaled_div(b, unary->operand());
      }

      // a * laplacian(u)
      if (b->type() == NodeType::Laplacian)
      {
        auto unary = std::static_pointer_cast<UnaryOpNode>(b);
        // Treat as a * div(grad(u))
        trace("Rule: a*laplacian(u) => treat as a*div(grad(u)) and apply scaled-div rule");
        auto gu = grad(unary->operand(), _dim);
        return handle_scaled_div(a, gu);
      }
      // laplacian(u) * a
      if (a->type() == NodeType::Laplacian)
      {
        auto unary = std::static_pointer_cast<UnaryOpNode>(a);
        auto gu = grad(unary->operand(), _dim);
        trace("Rule: laplacian(u)*a => treat as a*div(grad(u)) and apply scaled-div rule");
        return handle_scaled_div(b, gu);
      }

      // Fallback: treat as bulk term g: contributes -g*v (we lower -expr*v)
      trace("Rule: bulk term g => c0 += -g");
      res.c0 = neg(node);
      return res;
    }

    case NodeType::Divergence:
    {
      // -∫ div(V) v = ∫ V · ∇v
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      auto op = unary->operand();
      // Product rule: div(a V) = a div(V) + V·grad(a)
      if (op->type() == NodeType::Multiply)
      {
        trace("Rule: div(a*V) => c1 += a*V; c0 += V·grad(a) (grad(a) skipped unless a constant)");
        auto bin = std::static_pointer_cast<BinaryOpNode>(op);
        auto a = bin->left();
        auto V = bin->right();
        // c1 += a*V
        res.c1 = mulSafe(a, V);
        // c0 term V·grad(a) — zero for constants; skipped otherwise
        res.c0 = constant(0.0);
        return res;
      }
      res.c1 = op;
      trace("Rule: div(V) => c1 += V; c0 += 0");
      res.c0 = constant(0.0);
      return res;
    }

    case NodeType::Laplacian:
    {
      // -∫ Δu v = ∫ ∇u·∇v
      auto unary = std::static_pointer_cast<UnaryOpNode>(node);
      trace("Rule: laplacian(u) => c1 += grad(u); c0 += 0");
      res.c1 = grad(unary->operand(), _dim);
      res.c0 = constant(0.0);
      return res;
    }

    default:
      // Generic bulk: contributes -node * v
      trace("Rule: bulk term g => c0 += -g");
      res.c0 = neg(node);
      return res;
  }
}

bool StrongFormLowerer::isZero(const NodePtr & n) const
{
  if (!n)
    return true;
  if (n->type() == NodeType::Constant)
  {
    auto c = std::static_pointer_cast<ConstantNode>(n);
    if (c->value().isScalar())
      return std::abs(c->value().asScalar()) == 0.0;
  }
  return false;
}

NodePtr StrongFormLowerer::addSafe(const NodePtr & a, const NodePtr & b) const
{
  if (!a)
    return b;
  if (!b)
    return a;
  return add(a, b);
}

NodePtr StrongFormLowerer::subSafe(const NodePtr & a, const NodePtr & b) const
{
  if (!a)
    return b ? neg(b) : nullptr;
  if (!b)
    return a;
  return subtract(a, b);
}

NodePtr StrongFormLowerer::mulSafe(const NodePtr & a, const NodePtr & b) const
{
  if (!a || !b)
    return nullptr;
  return multiply(a, b);
}

NodePtr StrongFormLowerer::neg(const NodePtr & a) const
{
  if (!a)
    return nullptr;
  return negate(a);
}

bool StrongFormLowerer::dependsOnField(const NodePtr & n) const
{
  if (!n)
    return false;
  // Walk AST for field/variable nodes
  if (n->type() == NodeType::FieldVariable || n->type() == NodeType::Variable)
    return true;
  for (const auto & child : n->children())
    if (dependsOnField(child))
      return true;
  return false;
}

void StrongFormLowerer::trace(const std::string & msg) const
{
  if (!_verbose)
    return;
  if (_tracer)
  {
    _tracer(msg);
    return;
  }
  // Fallback to console output
  Moose::out << "[IBP] " << msg << std::endl;
}

}
}
