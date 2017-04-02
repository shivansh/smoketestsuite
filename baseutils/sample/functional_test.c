/*
 * $FreeBSD$
 *
 * Sample test file: takes (a specific) program name along with arguments
 * as input and checks whether the passed arguments are valid.
 */

#include <stdlib.h>
#include <getopt.h>
#include "functional_test.h"

int
main(int argc, char *argv[])
{
  while ((opt = getopt_long(argc, argv, short_options,
                  long_options, &option_index)) != -1) {
    switch(opt) {
      case 0:     /* valid long option */
        flag = 1;
        break;

      case '?':   /* invalid long option */
        break;

      default:
        break;
    }
  }
  return 0;
}
