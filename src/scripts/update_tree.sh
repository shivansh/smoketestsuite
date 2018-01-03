#!/bin/sh
# Script for updating the src tree.

set -eu

# NOTE The following location needs to be updated
# to point to the correct local FreeBSD src tree.
src="$HOME/freebsd/tools/tools/smoketestsuite"
echo $(pwd)

cd "$(dirname $0)/.."

rsync -avzHP \
	annotations \
	README \
	Makefile \
	add_testcase.cpp add_testcase.h \
	fetch_groff.cpp fetch_groff.h \
	generate_license.cpp generate_license.h \
	generate_test.cpp generate_test.h \
	logging.cpp logging.h \
	read_annotations.cpp read_annotations.h \
	utils.cpp utils.h \
	$src

rsync -avzHP \
	scripts/README scripts/fetch_utils.sh scripts/generate_annot.sh \
	$src/scripts
