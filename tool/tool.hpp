// Basic option defition.
typedef struct opt_def {
	char type; 		// Option type: (s)short/(l)long.
	std::string value;
	std::string keyword; 	// The keyword which should be looked up in the
				// message (if) produced when using this option.
} opt_def;
