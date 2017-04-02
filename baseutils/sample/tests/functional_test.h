/*
 * $FreeBSD$
 *
 * Header file for sample functional test
 */

int opt;
int option_index = 0;     /* longindex */
int flag = 0;             /* flipped when a valid argument is found */

/* The following short and long options are supposed to
 * be specific to the "sample" base utility under test
 * and provide an exemplary workflow which will be used
 */
char short_options[] = "";

const struct option long_options[] = {
  { "version", no_argument, 0, 0 },
  { "help",    no_argument, 0, 0 }
};
