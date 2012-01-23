# Makefile for ray_speed study
#
#    clean 	- remove compiled code and test results from directory
#    all	- build all compile targets (to support "make clean all")
#    default	- same as all
#
include ../../Symbols.mk

.PHONY: clean all default 

default: all

all: libusml.so data ray_speed

libusml.so:
	ln -s ../../libusml.so

data:
	ln -s ../../data

ray_speed: ray_speed.o libusml.so
	$(CXX) -o $@ $< $(LDFLAGS)

clean:
	-rm -f *.o *.csv *.nc *.log
	-rm libusml.so data ray_speed

