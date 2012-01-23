# Makefile for USML basic data types

include ../Symbols.mk
include ../Common.mk

DEPENDS = $(wildcard ../ublas/*.h)

all: \
    wvector1.o \
    wvector.o \
    wposition1.o \
    wposition.o \

# world coordinate vectors

wvector1.o: \
	wvector1.cc \
	wvector.h \
	wvector1.h \
	wposition1.h \
	$(DEPENDS)

wvector.o: \
	wvector.cc \
	wvector.h \
	wvector1.h \
	wposition1.h \
	$(DEPENDS)

# world coordinate positions

wposition1.o: \
	wposition1.cc \
	wvector.h \
	wvector1.h \
	wposition1.h \
	$(DEPENDS)

wposition.o: \
	wposition.cc \
	wposition.h \
	wvector.h \
	wvector1.h \
	wposition1.h \
	$(DEPENDS)

