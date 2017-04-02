# $FreeBSD$

# Makefile for building functional tests

CC = gcc
objects = functional_test.o

functional_test: $(objects)
	$(CC) -o functional_test $(objects)

functional_test.o: functional_test.h
	$(CC) -c functional_test.c

.PHONY: clean
clean:
	rm -f functional_test $(objects)
