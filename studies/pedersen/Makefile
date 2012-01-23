# Makefile for pedersen_test study
#
#    clean 	- remove compiled code and test results from directory
#    all	- build all compile targets (to support "make clean all")
#    default	- same as all
#
include ../../Symbols.mk

.PHONY: clean all default 

default: all

all: libusml.so data ffp_n2shallow.csv ffp_n2deep.csv pedersen_test

libusml.so:
	ln -s ../../libusml.so

data:
	ln -s ../../data

ffp_n2shallow.csv:
	ln -s ../../fastfield/test/ffp_n2shallow.csv

ffp_n2deep.csv:
	ln -s ../../fastfield/test/ffp_n2deep.csv

pedersen_test: pedersen_test.o libusml.so
	$(CXX) -o $@ $< $(LDFLAGS)

clean:
	-rm -f *.o *.csv *.nc *.log *.eps
	-rm -f cass/EIGENRAY.DAT cass/EIGENRAY.EIG cass/PRESSURE.DAT
	-rm libusml.so data pedersen_test

