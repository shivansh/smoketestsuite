/*
 * $FreeBSD$
 *
 * Header file for functional test of `ls` utility
 */

int opt;
int option_index = 0;     /* longindex */
int ret;                  /* return value of command */
FILE *fp;
char command[20];         /* TODO choose a safe limit for command length */
char path[1035];

char short_options[] = "";

const struct option long_options[] = {
  { "version", no_argument, 0, 0 },
  { "help",    no_argument, 0, 0 }
};
