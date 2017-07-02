#include <fstream>
#include <iostream>
#include <cstdlib>
#include "tool.hpp"

void
tool::find_opts::insert_opts()
{
  // Option definitions.
  opt_rel h_def;        // '-h'
  h_def.type = 's';
  h_def.value = "h";
  h_def.keyword = "help";

  opt_rel v_def;        // '-v'
  v_def.type = 's';
  v_def.value = "v";
  v_def.keyword = "version";

  // `opt_map` contains all the options
  // which can be "easily" tested.
  opt_map.insert(make_pair<string, opt_rel>
                               ("h", (opt_rel)h_def));  // NOTE: Explicit typecast required here.
  opt_map.insert(make_pair<string, opt_rel>
                               ("v", (opt_rel)v_def));
};

string
tool::find_opts::check_opts() {
  string line;                      // An individual line in a man-page.
  string opt_name;                  // Name of the option.
  string opt_ident = ".It Fl";      // Identifier for an option in man page.
  string buffer;                    // Option description extracted from man-page.
  string opt_string;                // Identified option names.
  int opt_pos;                      // Starting index of the (identified) option.
  list<opt_rel> ident_opt_list;     // List of identified option definitions (opt_rel's).

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
    if ((opt_pos = line.find(opt_ident)) != string::npos) {
      opt_pos += opt_ident.length() + 1;    // Locate the position of option name.
      opt_name = line.substr(opt_pos);

      // Check if the identified option matches the identifier.
      // `opt_list.back()` is the previously checked option, the
      // description of which is now stored in `buffer`.
      if ((opt_map_iter = opt_map.find(string(1, opt_list.back())))
                       != opt_map.end() &&
          buffer.find( (opt_map_iter->second).keyword) != string::npos) {
        ident_opt_list.push_back(opt_map_iter->second);

        // Since the option under test is a known
        // one, we remove it from `opt_list`.
        opt_list.pop_back();
      }

      // Update the string of valid options. We allow out list
      // to be populated with only short_options currently.
      if (opt_name.size() == 1)
        opt_list.append(opt_name);

      // Empty the buffer for next option's description.
      buffer.clear();
    }
    else {
      // Collect the option description until next
      // valid option definition is encountered.
      buffer.append(line);
    }
  }

  for (const auto& it : ident_opt_list)
    opt_string.append(it.value);

  return opt_string;
}
