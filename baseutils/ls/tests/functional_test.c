/*
 * $FreeBSD$
 *
 * Functional test for `ls` utility
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "functional_test.h"

int
main(int argc, char *argv[])
{
  while ((opt = getopt_long(argc, argv, short_options,
                  long_options, &option_index)) != -1) {
    switch(opt) {
      case 0:     /* valid long option */
        /* generate the valid command for execution */
        snprintf(command, sizeof(command),
                "/bin/ls --%s", long_options[option_index].name);
        ret = system(command);
        if (ret == -1)
          exit(EXIT_FAILURE);

        /* check if the command is valid */
        if (WEXITSTATUS(ret) != EXIT_SUCCESS) {
          fprintf(stderr, "Valid option `--%s` failed to execute\n",
                  long_options[option_index].name);
          exit(EXIT_FAILURE);
        }

      case '?':   /* invalid long option */
        break;

      default:
        break;
    }
  }
  return 0;
}
