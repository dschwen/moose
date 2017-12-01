
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <list>

#include "parse.h"

using namespace hit;

using DeleteList = std::list<Node *>;
using MatchedParams = std::map<std::string, std::string>;

/**
 * 1. Section names may contain up to one placeholder.
 *    valid section match paths are
 *      - 'fixed_section_name'
 *      - '<entirely_variable_name>'
 *      - 'prefixed_<variable_name>'
 *      - '<variable_name>_postfixes'
 *      - 'both_sides_<variable_name>_bracketed'
 *
 * 2. A variable is set as soon as the first section match is fount at the
 *    level the variable is used.
 */

Node *
loadAndParse(const std::string & fname)
{
  std::ifstream f(fname);
  std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return parse(fname, input);
}

struct PlaceHolderPattern
{
  std::string pre, post, symbol;
};

bool
parsePlaceholder(PlaceHolderPattern & pattern, const std::string & path)
{
  // find both '<' and '>'
  auto open_pos = path.find('<');
  auto close_pos = path.find('>');

  if ((open_pos == std::string::npos) != (close_pos == std::string::npos) || close_pos < open_pos)
    throw std::invalid_argument("malformed placeholder in path segment");

  if (open_pos == std::string::npos)
    return false;

  pattern.pre = path.substr(0, open_pos);
  pattern.post = path.substr(close_pos + 1);
  pattern.symbol = path.substr(open_pos + 1, close_pos - open_pos - 1);
  return true;
}

bool
matchPlaceholder(const PlaceHolderPattern & pattern, const std::string & path, std::string & match)
{
  auto pre_len = pattern.pre.length();
  auto post_len = pattern.post.length();
  auto path_len = path.length();

  if ((pre_len + post_len) <= path_len && path.substr(0, pre_len) == pattern.pre &&
      path.substr(path_len - post_len, post_len) == pattern.post)
  {
    match = path.substr(pre_len, path_len - pre_len - post_len);
    return true;
  }
  else
    return false;
}

bool
matchSection(Node * section, Node * input, DeleteList & delete_list, MatchedParams & matched_params)
{
  DeleteList new_deletes(delete_list);
  MatchedParams new_matches(matched_params);

  // first check if subsections match
  for (auto subsection : section->children(NodeType::Section))
  {
    // parse the pattern input section
    std::string pre, post, symbol;

    // if the subsection path in the match rule does not contain a placeholder
    // we can use find to pick it out of the input directly
    if (true)
    {
      // find subsection by name in input
      auto input_section = input->find(subsection->path());
      if (!input_section)
        return false;

      // check if the structure matches
      if (!matchSection(subsection, input_section, new_deletes, new_matches))
        return false;

      // subsection matches, if it has parameters it will be deleted
      new_deletes.push_back(input_section);
    }
    else
    {
      // loop over all childeren in the input and accept first candidate for
      // which the structure matches
      bool match_found = false;
      for (auto input_section : input->children(NodeType::Section))
      {
        if (matchSection(subsection, input_section, new_deletes, new_matches))
        {
          match_found = true;
          new_deletes.push_back(input_section);
          continue;
        }
      }
      if (!match_found)
        return false;
    }
  }

  // now check if parameters in the current section match
  for (auto field : section->children(NodeType::Field))
  {
    // find the field
    auto input_field = input->find(field->path());
    if (!input_field)
      return false;

    new_deletes.push_back(input_field);
  }

  // this section matched, commit all
  delete_list = new_deletes;
  matched_params = new_matches;
  return true;
}

int
main(int argc, char ** argv)
{
  // {
  //   PlaceHolderPattern pattern;
  //   std::cout << parsePlaceholder(pattern, "testomat") << '\t';
  //   std::cout << '\'' << pattern.pre << "'\t'" << pattern.symbol << "'\t'" << pattern.post <<
  //   "'\n";
  // }
  //
  // {
  //   PlaceHolderPattern pattern;
  //   std::cout << parsePlaceholder(pattern, "mutha<blank>") << '\t';
  //   std::cout << '\'' << pattern.pre << "'\t'" << pattern.symbol << "'\t'" << pattern.post <<
  //   "'\n";
  // }
  //
  // {
  //   PlaceHolderPattern pattern;
  //   std::cout << parsePlaceholder(pattern, "mutha_<blank>_fucking") << '\t';
  //   std::cout << '\'' << pattern.pre << "'\t'" << pattern.symbol << "'\t'" << pattern.post <<
  //   "'\n";
  // }
  //
  // return 0;

  if (argc < 3)
  {
    std::cerr << "Usage: rewrite inputfile.i rules1.i [rules2.i ...]\n";
    return 1;
  }

  // load the input file that is to be rewritten
  auto input = loadAndParse(argv[1]);

  // load first rule file
  auto rules_root = loadAndParse(argv[2]);

  // load additional rule files
  for (unsigned int i = 3; i < argc; ++i)
  {
    auto more_rules = loadAndParse(argv[i]);
    merge(more_rules, rules_root);
    delete more_rules;
  }

  // find ReplacementRules block
  auto rules = rules_root->find("ReplacementRules");
  if (!rules)
  {
    std::cerr << "No [ReplacementRules] block found in the rule files\n";
    return 1;
  }

  // loop over all rules
  for (auto rule : rules->children(NodeType::Section))
  {
    // Match
    auto match = rule->find("Match");
    if (!match)
    {
      std::cerr << "No [./Match] block found in rule '" << rule->path() << "'\n";
      return 1;
    }

    // Replace
    auto replace = rule->find("Replace");
    if (!replace)
    {
      std::cerr << "No [./Replace] block found in rule '" << rule->path() << "'\n";
      return 1;
    }

    // Try to apply the rule until it doesn't match anymore
    while (true)
    {
      // list of nodes to delete
      DeleteList delete_list;

      // matched parameters
      MatchedParams matched_params;

      // output current rule name
      std::cout << rule->path() << "\n";

      // try to match rule (advance to next rule if no match is found)
      if (!matchSection(match, input, delete_list, matched_params))
        break;

      // delete what can be deleted

      // insert replacement
    }
  }

  return 0;
}
