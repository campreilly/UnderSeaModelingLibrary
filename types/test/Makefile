# Makefile for USML data type tests

include ../../Symbols.mk
include ../../Common.mk

all: \
	position_test.o \
	sequence_test.o \
	datagrid_test.o \

# unit tests for position definitions
 
position_test.o: \
	position_test.cc \
	../wposition.h \
	../wvector.h \
	../wvector1.h \
	../wposition1.h \

# unit tests for sequences

sequence_test.o: \
	sequence_test.cc \
	$(wildcard ../seq_*.h) \

# unit tests for data_grid.h

datagrid_test.o: \
	datagrid_test.cc \
	../data_grid.h \
	$(wildcard ../seq_*.h) \

