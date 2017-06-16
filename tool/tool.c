#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "tool.h"
#define SET_SIZE 2

int
main()
{
	char line[128]; 		/* A line to be parsed in man page. TODO: Safe size limit. */
	char opt_ident[] = ".It Fl"; 	/* Identifier for an option in man page. */
	char *opt_ptr;
	FILE *fp;

	/* Define custom options which can be easily tested. */
	opt_descr h_descr;
	sprintf(h_descr.option,     "%c", 'h');
	sprintf(h_descr.identifier, "%s", "help");

	opt_descr v_descr;
	sprintf(v_descr.option,     "%c", 'v');
	sprintf(v_descr.identifier, "%s", "version");

	/* TODO: Update the DS for O(1) membership tests */
	opt_descr opt_set[SET_SIZE] = {h_descr, v_descr};

	/* An example man-page for ln(1). */
	fp = fopen("ln.1", "r");
	while (fgets(line, sizeof(line), fp) != NULL) {
		if ((opt_ptr = strstr(line, opt_ident)) != NULL) {
			opt_ptr += sizeof(opt_ident); 	/* Newline character included in the offset */
			for (int i = 0; i < SET_SIZE; i++) {
				if (!strcmp(opt_set[i].option, opt_ptr)) {
					/* Check for identifier in usage message when using this option */
				}
			}
		}
	}

	fclose(fp);
	return 0;
}
