#!/bin/sh
#
# Copyright 2017 Shivansh Rai
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $FreeBSD$

# Script for listing location of groff scripts of utilities in section 1

set -e

src="../../../"
dir_list="tools/tools/smoketestsuite/scripts/utils_list"
groff_src="tools/tools/smoketestsuite/groff"

cd "$src"
rm -rf "$groff_src" && mkdir "$groff_src"

while IFS= read -r dir_entry
do
	# Copy the groff scripts only for the
	# utilities which do not contain tests.
	if [ ! -d "$dir_entry/tests" ]; then
		for file in "$dir_entry"/*
		do
			case "$file" in
				*.1) cp "$file" "$groff_src" ;;
				*.8) cp "$file" "$groff_src" ;;
			esac
		done
	fi
done< "$dir_list"

# Remove the groff scripts for which the tool
# (still) hangs waiting for user input.
rm -f "$groff_src/pax.1"

# Remove non-executable utilities
rm -f "$groff_src/elfcopy.1"
