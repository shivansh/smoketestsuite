# $FreeBSD$
#
# Makefile for building the test generation tool

PROG_CXX=	generate_tests
LOCALBASE=	/usr/local
MAN=
CXXFLAGS+=	-I${LOCALBASE}/include -std=c++11
LDFLAGS+=	-L${LOCALBASE}/lib -lboost_filesystem -lboost_system
SRCS=	logging.cpp \
	utils.cpp \
	read_annotations.cpp \
	generate_license.cpp \
	add_testcase.cpp \
	fetch_groff.cpp \
	generate_test.cpp

.PHONY: clean \
	fetch_groff \
	fetch_utils \
	run

fetch_groff:
	sh ${.CURDIR}/scripts/fetch_groff.sh

fetch_utils:
	sh ${.CURDIR}/scripts/fetch_utils.sh

run:
	@echo Generating annotations...
	sh ${.CURDIR}/scripts/generate_annot.sh
	@echo Generating test files...
	./generate_tests

.include <bsd.prog.mk>
