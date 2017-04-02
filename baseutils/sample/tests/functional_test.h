/*
 * $FreeBSD$
 *
 * Header file for sample functional test
 */

int opt;
int option_index = 0;     /* longindex */
int flag = 0;             /* flipped when a valid argument is found */

char short_options[] = "";

const struct option long_options[] = {
  { "version", no_argument, 0, 0 },
  { "help",    no_argument, 0, 0 }
};
