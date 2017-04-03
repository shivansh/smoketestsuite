/*
 * $FreeBSD$
 *
 * Smoke test for `ls` utility
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
                "/bin/ls --%s > /dev/null", long_options[option_index].name);
        ret = system(command);

        if (ret == -1) {
          fprintf(stderr, "Failed to create child process\n");
          exit(EXIT_FAILURE);
        }

        if (!WIFEXITED(ret)) {
          fprintf(stderr, "Child process failed to terminate normally\n");
          exit(EXIT_FAILURE);
        }

        if (WEXITSTATUS(ret))
          fprintf(stderr, "\nValid option '--%s' failed to execute\n",
                  long_options[option_index].name);
        else
          printf("Successful: '--%s'\n", long_options[option_index].name);

        break;

      case '?':   /* invalid long option */
        break;

      default:
        printf("getopt_long returned character code %o\n", opt);
    }
  }

  exit(EXIT_SUCCESS);
}
