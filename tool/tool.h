#define MAX_LEN 10

/* Struct for defining information specific to an option. */
typedef struct opt_descr {
	char option[MAX_LEN]; 		/* Option value */
	char identifier[MAX_LEN]; 	/* Identifier for the option in the usage message */
} opt_descr;
