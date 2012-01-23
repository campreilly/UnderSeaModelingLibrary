# Makefile for USML netCDF utilities

include ../Symbols.mk
include ../Common.mk

DEPENDS = \
	$(wildcard ../ublas/*.h) \
	$(wildcard ../types/*.h) \

all: \
	netcdf_bathy.o \
	netcdf_profile.o \
	netcdf_woa.o \

# utilities for reading netCDF bathymetry

netcdf_bathy.o: \
	netcdf_bathy.cc \
	netcdf_bathy.h \
	$(DEPENDS)

# utilities for reading netCDF ocean profiles

netcdf_profile.o: \
	netcdf_profile.cc \
	netcdf_profile.h \
	$(DEPENDS)

# utilities for reading World Ocean Atlas ocean profiles

netcdf_woa.o: \
	netcdf_woa.cc \
	netcdf_woa.h \
	netcdf_profile.h \
	$(DEPENDS)

