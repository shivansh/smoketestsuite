#!/bin/sh
#
# Copyright 2017-2018 Shivansh Rai
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

# Script for generating annotations based on generated tests.

update_annotations() {
	cd "$testdir" 2>/dev/null
	report=$(kyua report)
	cd -
	i=2
	annotations=""
	diff=""

	while true
	do
		testcase=$(printf "%s" "$report" | awk 'NR=='"$i"' {print $1}')
		exitstatus=$(printf "%s" "$report" | awk 'NR=='"$i"' {print $3}')
		check=$(printf "%s" "$testcase" | cut -s -f1 -d":")

		if [ ! "$check" ] && [ "$annotations" ]; then
			file="$tooldir/annotations/$test.ant"  # Annotations file
			# Append only the new annotations.
			printf "$annotations" > "$file.temp"
			[ ! -e "$file" ] && touch "$file"
			diff=$(comm -13 "$file" "$file.temp")
			if [ "$diff" ]; then
				if [ $prompt = 0 ]; then
					prompt=1
					printf "\nModified annotation files -\n"
				fi
				printf "$diff\n" >> "$file"
				printf "  * %s\n" "$test.ant"
			fi
			rm -f "$file.temp"
			break
		fi

		if [ "$exitstatus" = "failed:" ]; then
			testcase=${testcase#"$test:"}
			annotations="$annotations$testcase\n"
		elif [ ! "$exitstatus" ]; then
			break
		fi
		i=$((i+1))
	done
}

suffix="_test"
extension=".sh"
prompt=0
tooldir=$(dirname $0)/..

# Check if the directory "generated_tests/" is populated.
if [ ! -d "$tooldir/generated_tests" ] || \
	[ -z "$(ls -A $tooldir/generated_tests)" ]; then
	exit
fi

for f in "$tooldir/generated_tests"/*
do
	file=$(basename "$f")
	test=${file%$extension}
	utility=${test%$suffix}

	# Guess the correct installation directory.
	if [ -d "/usr/tests/bin/$utility" ]; then
		testdir="/usr/tests/bin/$utility"
	elif [ -d "/usr/tests/usr.bin/$utility" ]; then
		testdir="/usr/tests/usr.bin/$utility"
	elif [ -d "/usr/tests/usr.sbin/$utility" ]; then
		testdir="/usr/tests/usr.sbin/$utility"
	else
		continue
	fi
	update_annotations
done

if [ $prompt = 1 ]; then
	printf "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n"
fi
