# Makefile for USML ocean tests

include ../../Symbols.mk
include ../../Common.mk

all: \
	attenuation_test.o \
	profile_test.o \
	reflect_loss_test.o \
	boundary_test.o \
	boundary_test.nc \

boundary_test.nc:
	ncks -O -a -d y,36.0,36.200001 -d x,15.85,16.0 \
		../../data/bathymetry/ETOPO1_Ice_g_gmt4.grd $@
	ncdump $@ > $(basename $@).log

