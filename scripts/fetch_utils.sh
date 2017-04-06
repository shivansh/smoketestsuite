#!/usr/bin/env sh
# Script for listing all the base utilities currently without tests

check_utils() {
  printf "=========\n"
  basename "$path"
  printf "=========\n"
  cd "$path" || exit
  for dir in */;
  do
    (
      cd "$dir" || exit
      if grep -q -s 'PROG\|PROG_CXX' "Makefile" ; then
        basename "$dir"
      fi
    )
  done
}

rm -f utils_list

path="$HOME/source-codes/freebsd/bin"
(check_utils) >> utils_list

path="$HOME/source-codes/freebsd/sbin"
(check_utils) >> utils_list
