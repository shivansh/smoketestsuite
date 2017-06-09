#! /bin/sh
# Script for listing all the base utilities

set -e

fetch_utils() {
  cd "$path" || exit
  find . -name Makefile | xargs grep -l 'PROG\|PROG_CXX' \
                        | sed -e 's|/Makefile$||' | cut -c 3-
}

rm -f utils_list

path="$HOME/source-codes/freebsd"
(fetch_utils) >> utils_list
