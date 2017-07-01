#include <fstream>
#include <iostream>
#include <cstdlib>
#include "tool.hpp"

void
tool::find_opts::insert_opts()
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
  opt_map.insert(make_pair<string, opt_def>
                               ("h", (opt_def)h_def));  // NOTE: Explicit typecast required here.
  opt_map.insert(make_pair<string, opt_def>
                               ("v", (opt_def)v_def));
};

string
tool::find_opts::check_opts() {
  // Generate the hashmap opt_map.
  insert_opts();

  // An example utility under test: ln(1).
  // TODO: Walk the entire source tree.
  tool::find_opts::utility = "ln";
  // TODO: Section number cannot be hardcoded.
  ifstream infile(tool::find_opts::utility + ".1");

  // Search for all the options accepted by the
  // utility and collect those present in `opt_map`.
  while (getline(infile, line)) {
    if ((opt_pos = tool::find_opts::line.find(opt_ident)) != string::npos) {
      opt_pos += opt_ident.length() + 1;    // Locate the position of option name.
      opt_name = line.substr(opt_pos);

      // Check if the identified option matches the identifier.
      // `opt_list.back()` is the previously checked option, the
      // description of which is now stored in `buffer`.
      if ((opt_map_iter = opt_map.find(string(1, opt_list.back())))
                       != opt_map.end() &&
          buffer.find( (opt_map_iter->second).keyword) != string::npos)
        identified_opt_list.push_back(opt_map_iter->second);

      // Update the string of valid options.
      if (opt_name.size() == 1)
        opt_list.append(opt_name);

      // Empty the buffer.
      buffer.clear();
    }
    else {
      // Collect the option description until next
      // valid option definition is encountered.
      buffer.append(line);
    }
  }

  string opt_string;
  for (const auto& it : identified_opt_list) {
    opt_string.append(it.value);
  }

  return opt_string;
}
