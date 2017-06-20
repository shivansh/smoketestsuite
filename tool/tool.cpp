#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include "tool.hpp"

tool::insert_opts::insert_opts()
{
  // Option definitions.
  opt_def h_def;        // '-h'
  h_def.type = 's';
  h_def.value = "h";
  h_def.keyword = "help";
  opt_def v_def;        // '-v'
  v_def.type = 's';
  v_def.value = "v";
  v_def.keyword = "version";

  // `opt_map` contains all the options
  // which can be "easily" tested.
  opt_map.insert(std::make_pair<std::string, opt_def>
                               ("h", (opt_def)h_def));  // NOTE: Explicit typecast required here.
  opt_map.insert(std::make_pair<std::string, opt_def>
                               ("v", (opt_def)v_def));

};

int
main()
{
  using namespace tool;

  // An example man-page for ln(1).
  std::ifstream infile("ln.1");

  // Search for all the options accepted by the
  // utility and collect those present in `opt_map`.
  while (std::getline(infile, line)) {
    if ((opt_pos = tool::line.find(opt_ident)) != std::string::npos) {
      opt_pos += opt_ident.length() + 1;    // Locate the position of option name.
      opt_name = line.substr(opt_pos);
      if ((opt_map_iter = opt_map.find(opt_name)) != opt_map.end()) {
        opt_list.push_back(opt_map_iter->second);
      }
    }
  }

  for (auto it : opt_list)
    std::cout << it.value << ": "
              << it.keyword << std::endl;   // NOTE: Test operation, will be updated.

  return 0;
}
