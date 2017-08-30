# $FreeBSD$
#
# Makefile for building functional tests

CC 		= gcc
CFLAGS 		= -I.
OBJS 		= functional_test.o log.o

functional_test : $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC)

$(OBJS) 	: $(.PREFIX).c $(.PREFIX).h
	$(CC) $(CFLAGS) -c $(.PREFIX).c

.PHONY: clean test

clean:
	rm -f functional_test error.log $(OBJS)

test:
	./functional_test --version --help
