/*
 * $FreeBSD$
 *
 * Header file for functional test of `ls` utility
 */

int opt;
int option_index = 0;     /* longindex */
int ret;                  /* return value of command */
FILE *fp;
char command[50];         /* TODO choose a safe limit for command length */

char short_options[] = "";

static struct option long_options[] = {
  { "version", no_argument, 0, 0 },
  { "help",    no_argument, 0, 0 },
  { 0,         0,           0, 0 }

};
