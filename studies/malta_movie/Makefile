# Makefile for malta_movie study
#
#    clean 	- remove compiled code and test results from directory
#    all	- build all compile targets (to support "make clean all")
#    default	- same as all
#
include ../../Symbols.mk

.PHONY: clean all default 

default: all

all: libusml.so data malta_movie malta_rays

libusml.so:
	ln -s ../../libusml.so

data:
	ln -s ../../data

malta_movie: malta_movie.o libusml.so
	$(CXX) -o $@ $< $(LDFLAGS)

malta_rays: malta_rays.o libusml.so
	$(CXX) -o $@ $< $(LDFLAGS)

clean:
	-rm -f *.o *.csv *.nc *.log
	-rm libusml.so data malta_movie malta_rays

