/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FINITESTRAINUOBASEDCP_H
#define FINITESTRAINUOBASEDCP_H

#include "ComputeStressBase.h"

#include "CrystalPlasticitySlipRate.h"
#include "CrystalPlasticitySlipResistance.h"
#include "CrystalPlasticityStateVariable.h"
#include "CrystalPlasticityStateVariableEvolutionRateComponent.h"


/**
 * FiniteStrainUObasedCP uses the multiplicative decomposition of deformation gradient
 * and solves the PK2 stress residual equation at the intermediate configuration to evolve the material state.
 * The internal variables are updated using an interative predictor-corrector algorithm.
 * Backward Euler integration rule is used for the rate equations.
 *
 * Involves 4 different types of user objects that calculates:
 * State variables - update state variable (derive from CrystalPlasticityStateVariable)
 * State variable evolution compoment - individual component of state variable incremental rate (derive from CrystalPlasticityStateVariableEvolutionRateComponent)
 * Slip resistance - calcuate slip resistances (derive from CrystalPlasticitySlipResistances)
 * Slip rates - calcuate flow direction and slip rates (derive from CrystalPlasticitySlipRates)
 */
class FiniteStrainUObasedCP;

template<>
InputParameters validParams<FiniteStrainUObasedCP>();

class FiniteStrainUObasedCP : public ComputeStressBase
{
public:
  FiniteStrainUObasedCP(const InputParameters & parameters);

protected:
  /**
   * This function updates the stress at a quadrature point.
   */
  virtual void computeQpStress();

  /**
   * This function updates the elasticity tensor at a quadrature point.
   * Presently void.
   */
  virtual void computeQpElasticityTensor();

  /**
   * This function initializes the stateful properties such as
   * stress, plastic deformation gradient, slip system resistances, etc.
   */
  virtual void initQpStatefulProperties();

  /**
   * This function calls the residual and jacobian functions used in the
   * stress update algorithm.
   */
  virtual void calc_resid_jacob(RankTwoTensor &, RankFourTensor &);

  //Override to modify slip system resistance evolution
  /**
   * This function updates the slip system resistances.
   */
  virtual void update_slip_system_resistance();

  /**
   * This function set variables for stress and internal variable solve.
   */
  virtual void preSolveQp();

  /**
   * This function solves stress and internal variables.
   */
  virtual void solveQp();

  /**
   * This function update stress and internal variable after solve.
   */
  virtual void postSolveQp();

  /**
   * This function set variables for internal variable solve.
   */
  virtual void preSolveStatevar();

  /**
   * This function solves internal variables.
   */
  virtual void solveStatevar();

  /**
   * This function update internal variable after solve.
   */
  virtual void postSolveStatevar();

  /**
   * This function set variables for stress solve.
   */
  virtual void preSolveStress();

  /**
   * This function solves for stress, updates plastic deformation gradient.
   */
  virtual void solveStress();

  /**
   * This function update stress and plastic deformation gradient after solve.
   */
  virtual void postSolveStress();

  /**
   * This function calculate stress residual.
   */
  virtual void calcResidual( RankTwoTensor & );

  /**
   * This function calculate jacobian.
   */
  virtual void calcJacobian( RankFourTensor & );

  /**
   * This function updates the slip increments.
  */
  virtual void getSlipIncrements();

  /**
   * This function calculate the tangent moduli for preconditioner.
   * Default is the elastic stiffness matrix.
   * Exact jacobian is currently implemented.
   * tan_mod_type can be modified to exact in .i file to turn it on.
   */
  virtual ElasticityTensorR4 calcTangentModuli();

  /**
   * This function calculate the elastic tangent moduli for preconditioner.
   */
  virtual ElasticityTensorR4 elasticTangentModuli();

  /**
   * This function calculate the exact tangent moduli for preconditioner.
   */
  virtual ElasticityTensorR4 elastoPlasticTangentModuli();

  /**
   * This function perform RU decomposition to obtain the rotation tensor.
   */
  RankTwoTensor get_current_rotation(const RankTwoTensor & a);

  ////Old function: Kept to avoid code break in computeQpStress
  /**
   * This function perform RU decomposition to obtain the rotation tensor.
   */
  RankTwoTensor getMatRot(const RankTwoTensor & a);

  /**
   * This function performs the line search update
   */
  bool line_search_update(const Real rnorm_prev, const RankTwoTensor);

  /**
   * This function updates internal variables after each NewTon Raphson iteration (_fp_inv)
   */
  void internalVariableUpdateNRiteration();

  /**
   * This function evaluates convergence of state variables.
   */
  virtual bool getIterFlagVar();

  ///User objects that define the slip rate
  std::vector<const CrystalPlasticitySlipRate *> _uo_slip_rates;

  ///User objects that define the slip resistance
  std::vector<const CrystalPlasticitySlipResistance *> _uo_slip_resistances;

  ///User objects that define the state variable
  std::vector<const CrystalPlasticityStateVariable *> _uo_state_vars;

  ///User objects that define the state variable evolution rate component
  std::vector<const CrystalPlasticityStateVariableEvolutionRateComponent *> _uo_state_var_evol_rate_comps;

  ///Slip rates material property
  std::vector<MaterialProperty<std::vector<Real> > * > _mat_prop_slip_rates;

  ///Slip resistance material property
  std::vector<MaterialProperty<std::vector<Real> > * > _mat_prop_slip_resistances;

  ///State variable material property
  std::vector<MaterialProperty<std::vector<Real> > * > _mat_prop_state_vars;

  ///Old state variable material property
  std::vector<MaterialProperty<std::vector<Real> > * > _mat_prop_state_vars_old;

  ///State variable evolution rate component material property
  std::vector<MaterialProperty<std::vector<Real> > * > _mat_prop_state_var_evol_rate_comps;

  ///Number of slip rate user objects
  unsigned int _num_uo_slip_rates;

  ///Number of slip resistance user objects
  unsigned int _num_uo_slip_resistances;

  ///Number of state variable user objects
  unsigned int _num_uo_state_vars;

  ///Number of state variable evolution rate component user objects
  unsigned int _num_uo_state_var_evol_rate_comps;

  ///Local state variable
  std::vector<std::vector<Real> > _state_vars_old;

  ///Local old state variable
  std::vector<std::vector<Real> > _state_vars_prev;

  ///Stress residual equation relative tolerance
  Real _rtol;
  ///Stress residual equation absolute tolerance
  Real _abs_tol;
  ///Internal variable update equation tolerance
  Real _stol;
  ///Residual tolerance when variable value is zero. Default 1e-12.
  Real _zero_tol;

  ///Maximum number of iterations for stress update
  unsigned int _maxiter;
  ///Maximum number of iterations for internal variable update
  unsigned int _maxiterg;

  ///Type of tangent moduli calculation
  MooseEnum _tan_mod_type;

  ///Flag to save euler angle as Material Property
  bool _save_euler_angle;

  ///Maximum number of substep iterations
  unsigned int _max_substep_iter;

  ///Flag to activate line serach
  bool _use_line_search;

  ///Minimum line search step size
  Real _min_lsrch_step;

  ///Line search bisection method tolerance
  Real _lsrch_tol;

  ///Line search bisection method maximum iteration number
  unsigned int _lsrch_max_iter;

  //Line search method
  MooseEnum _lsrch_method;

  MaterialProperty<RankTwoTensor> & _fp;
  MaterialProperty<RankTwoTensor> & _fp_old;
  MaterialProperty<RankTwoTensor> & _pk2;
  MaterialProperty<RankTwoTensor> & _pk2_old;
  MaterialProperty<RankTwoTensor> & _lag_e;
  MaterialProperty<RankTwoTensor> & _lag_e_old;
  MaterialProperty<RankTwoTensor> & _update_rot;
  MaterialProperty<RankTwoTensor> & _update_rot_old;

  const MaterialProperty<RankTwoTensor> & _deformation_gradient;
  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;

  ///Save Euler angles for output only when save_euler_angle = true in .i file
  MaterialProperty< std::vector<Real> > * _euler_ang;
  MaterialProperty< std::vector<Real> > * _euler_ang_old;

  ///Crystal rotation
  const MaterialProperty<RankTwoTensor> & _crysrot;

  ///Crystal Euler angles
  const MaterialProperty<RealVectorValue> & _Euler_angles;

  RankTwoTensor _pk2_tmp_old;
  RankTwoTensor _dfgrd_tmp;
  RankTwoTensor _fe, _fp_old_inv, _fp_inv, _fp_prev_inv;
  DenseVector<Real> _tau;
  std::vector<MaterialProperty<std::vector<RankTwoTensor> > * > _flow_direction;

  bool _err_tol;///Flag to check whether convergence is achieved

  ///Used for substepping; Uniformly divides the increment in deformation gradient
  RankTwoTensor _delta_dfgrd, _dfgrd_tmp_old;
  ///Scales the substepping increment to obtain deformation gradient at a substep iteration
  Real _dfgrd_scale_factor;
  ///Flags to reset variables and reinitialize variables
  bool _first_step_iter, _last_step_iter, _first_substep;
};

#endif //FINITESTRAINUOBASEDCP_H

