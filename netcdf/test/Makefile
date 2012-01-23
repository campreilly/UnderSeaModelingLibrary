# Makefile for USML netCDF tests

include ../../Symbols.mk
include ../../Common.mk

all: \
	read_bathy_test.o \
	etopo_cmp.nc \
	etopo_cmp2w.nc \
	etopo_cmp2e.nc \
	read_profile_test.o \
	woa_cmp.nc \
	woa_cmp2.nc \

# unit tests for bathymetry extraction from netCDF
 
etopo_cmp.nc:
	ncks -O -a -d y,18.0,23.000001 -d x,-160.0,-154.0 ../../data/bathymetry/ETOPO1_Ice_g_gmt4.grd $@
	ncdump $@ > $(basename $@).log

etopo_cmp2w.nc:
	ncks -O -a -d y,-1.0,2.000001 -d x,179.0,180.0 ../../data/bathymetry/ETOPO1_Ice_g_gmt4.grd $@
	ncdump $@ > $(basename $@).log

etopo_cmp2e.nc:
	ncks -O -a -d y,-1.0,2.000001 -d x,-179.9834,-178.0 ../../data/bathymetry/ETOPO1_Ice_g_gmt4.grd $@
	ncdump $@ > $(basename $@).log

# unit tests for ocean profile extraction from netCDF
 
woa_cmp.nc:
	ncks -O -a -v depth,time,lat,lon,t_an -d time,165.0,165.0 -d lat,18.0,23.0 \
	   -d lon,200.0,206.0 ../../data/woa09/temperature_monthly_1deg.nc $@
	ncdump $@ > $(basename $@).log

woa_cmp2.nc:
	ncks -O -a -v depth,time,lat,lon,t_an -d time,0.0,0.0 -d lat,0.5,0.5 \
	   -d lon,0.5,0.5 ../../data/woa09/temperature_monthly_1deg.nc $@
	ncdump $@ > $(basename $@).log
