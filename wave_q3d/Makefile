# Makefile for USML propagation model: Geodetic Acoustic Rays in the Time Domain

include ../Symbols.mk
include ../Common.mk

DEPENDS = \
	$(wildcard ../ublas/*.h) \
	$(wildcard ../types/*.h) \
	$(wildcard ../ocean/*.h) \

all: \
    ode_integ.o \
    proploss.o \
    reflection_model.o \
    spreading_ray.o \
    spreading_hybrid_gaussian.o \
    wave_front.o \
    wave_queue_netcdf.o \
    wave_queue.o \

ode_integ.o: \
	ode_integ.cc \
	ode_integ.h \
	$(DEPENDS)

proploss.o: \
	proploss.cc \
	proploss.h \
	eigenray.h \
	$(DEPENDS)

reflection_model.o: \
	reflection_model.cc \
	reflection_model.h \
	ode_integ.h \
	wave_queue.h \
	$(DEPENDS)

spreading_ray.o: \
	spreading_ray.cc \
	spreading_ray.h \
	wave_queue.h \
	$(DEPENDS)

spreading_hybrid_gaussian.o: \
	spreading_hybrid_gaussian.cc \
	spreading_hybrid_gaussian.h \
	wave_queue.h \
	$(DEPENDS)

wave_front.o: \
	wave_front.cc \
	wave_front.h \
	$(DEPENDS)

wave_queue_netcdf.o: \
	wave_queue_netcdf.cc \
	wave_queue.h \
	wave_front.h \
	$(DEPENDS)

wave_queue.o: \
	wave_queue.cc \
	wave_queue.h \
	eigenray.h \
	ode_integ.h \
	proploss.h \
	reflection_model.h \
	spreading_model.h \
	spreading_ray.h \
	spreading_hybrid_gaussian.h \
	wave_front.h \
	$(DEPENDS)

