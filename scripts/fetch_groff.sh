#!/bin/sh

src="$HOME/freebsd"
dir_list="$HOME/source-codes/smoketestsuite/scripts/utils_list"
groff_list="$HOME/source-codes/smoketestsuite/scripts/groff_list"

rm -f "$groff_list.1" && touch "$groff_list.1"
cd $src || exit

while IFS= read -r dir_entry
do
  for file in "$dir_entry"/*
  do
    # Check for only section 1 entries
    case "$file" in
      # *.1) cp "$file" "$HOME/source-codes/smoketestsuite/tool/groff"
        *.1) printf "%s\n" "$file" >> "$groff_list.1"
    esac
  done
done< "$dir_list"
