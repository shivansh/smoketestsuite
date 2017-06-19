// Basic option defition.
typedef struct opt_def {
	char type;						// Option type: (s)short/(l)long.
	std::string value;
	std::string keyword; 	// The keyword which should be looked up in the
												// message (if) produced when using this option.
} opt_def;

namespace tool {
	std::string line;										// An individual line in a man-page.
	std::string opt_ident = ".It Fl"; 	// Identifier for an option in man page.
	std::string opt_name;								// Name of the option.
	int opt_pos;												// Starting index of the (identified) option.
	std::list<opt_def> opt_list;				// List of identified option definitions (opt_def's).
	std::list<opt_def>::iterator opt_list_iter;
	std::unordered_map<std::string, opt_def> opt_map; // Map "option value" to "option definition".
	std::unordered_map<std::string, opt_def>::iterator opt_map_iter;

	class insert_opts {
		insert_opts();
	};
}
