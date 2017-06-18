#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include "tool.hpp"

using namespace std;

int
main()
{
	string line; 				// An individual line in a man-page.
	string opt_ident = ".It Fl"; 		// Identifier for an option in man page.
	string opt_name; 			// Name of the option.
	int opt_pos; 				// Starting index of the (identified) option.
	list<opt_def> opt_list; 		// List of identified option definitions (opt_def's).
	list<opt_def>::iterator opt_list_iter;
	unordered_map<string, opt_def> opt_map; // Map "option value" to "option definition".
	unordered_map<string, opt_def>::iterator opt_map_iter;

	// Option definitions.
	opt_def h_def; 		// '-h'
	h_def.type = 's';
	h_def.value = "h";
	h_def.keyword = "help";
	opt_def v_def; 		// '-v'
	v_def.type = 's';
	v_def.value = "v";
	v_def.keyword = "version";

	// `opt_map` contains all the options
	// which can be "easily" tested.
	opt_map.insert(make_pair<string, opt_def>
				("h", (opt_def)h_def));  // NOTE: Explicit typecast required here.
	opt_map.insert(make_pair<string, opt_def>
				("v", (opt_def)v_def));

	// An example man-page for ln(1).
	ifstream infile("ln.1");

	// Search for all the options accepted by the
	// utility and collect those present in `opt_map`.
	while (getline(infile, line)) {
		if ((opt_pos = line.find(opt_ident)) != string::npos) {
			opt_pos += opt_ident.length() + 1; 	// Locate the position of option name.
			opt_name = line.substr(opt_pos);
			if ((opt_map_iter = opt_map.find(opt_name)) != opt_map.end()) {
				opt_list.push_back(opt_map_iter->second);
			}
		}
	}

	for (auto it : opt_list)
		cout << it.value << ": "
		     << it.keyword << endl; 	// NOTE: Test operation, will be updated.

	return 0;
}
