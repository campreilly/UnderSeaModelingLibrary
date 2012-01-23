# Makefile for USML uBLAS tests

include ../../Symbols.mk
include ../../Common.mk

DEPENDS = \
	$(wildcard ../*.h) \

all: \
	vector_test.o \
	matrix_test.o \
	randgen_test.o \

# unit tests for vector_math.h

vector_test.o: vector_test.cc $(DEPENDS)

# unit tests for matrix_math.h

matrix_test.o: matrix_test.cc $(DEPENDS)

# unit tests for randgen_math.h

randgen_test.o: randgen_test.cc $(DEPENDS)

