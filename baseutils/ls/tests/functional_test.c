/*
 * Copyright 2017 Shivansh Rai
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
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
        ret = system(command);    /* execute the command */

        if (ret == -1) {
          failed++;
          fprintf(stderr, "Failed to create child process\n");
          exit(EXIT_FAILURE);
        }
        else if (!WIFEXITED(ret)) {
          failed++;
          fprintf(stderr, "Child process failed to terminate normally\n");
          exit(EXIT_FAILURE);
        }
        else if (WEXITSTATUS(ret)) {
          failed++;
          fprintf(stderr, "\nValid option '--%s' failed to execute\n",
                          long_options[option_index].name);
        }
        else
          printf("Successful: '--%s'\n", long_options[option_index].name);

        break;

      case '?':   /* invalid long option */
        break;

      default:
        printf("getopt_long returned character code %o\n", opt);
    }
  }
  printf("Failed: %d\n", failed);

  exit(EXIT_SUCCESS);
}
