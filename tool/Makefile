# $FreeBSD$
#
# Makefile for building automation tool

CC 	= c++
CFLAGS	= -I/usr/local/include -std=c++11
LIBS 	= -L/usr/local/lib -lboost_system -lboost_thread
OBJS 	= utils.o read_annotations.o add_testcase.o generate_test.o

generate_tests 	: $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC) $(LIBS)

$(OBJS) 	: $(.PREFIX).cpp $(.PREFIX).h
	$(CC) $(CFLAGS) -c $(.PREFIX).cpp

.PHONY: clean clean_tests run validate_changes

clean:
	rm -f generate_test $(OBJS)

clean_tests:
	rm -f generated_tests/*

run:
	@echo Generating annotations...
	sh ../scripts/generate_annot.sh
	@echo Generating test files...
	./generate_tests

validate_changes:
	sh ../scripts/validate.sh
