
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <set>

#include "parse.h"

using namespace hit;

int
main(int argc, char ** argv)
{
  if (argc < 2)
  {
    std::cerr << "must specify a filename\n";
    return 1;
  }

  std::string fname(argv[1]);
  std::ifstream f(fname);
  std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

  // parse file
  hit::Node * root = nullptr;
  try
  {
    root = hit::parse(fname, input);
  }
  catch (std::exception & err)
  {
    std::cerr << err.what() << "\n";
    return 1;
  }

  // find materials block
  auto material_block = root->find("Materials");
  if (!material_block)
    return 0;

  auto materials = material_block->children(NodeType::Section);
  for (auto material : materials)
  {
    std::string val = material->paramOptional<std::string>("type", "");
    if (val == "DerivativeParsedMaterial")
    {
      std::string selected = material->render();
      std::cout << selected << "\n";
    }
  }

  return 0;
}
