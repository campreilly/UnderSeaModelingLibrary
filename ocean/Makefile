# Makefile for USML ocean models

include ../Symbols.mk
include ../Common.mk

DEPENDS = \
	$(wildcard ../ublas/*.h) \
	$(wildcard ../types/*.h) \

all: \
    attenuation_constant.o \
    attenuation_thorp.o \
    profile_model.o \
    profile_catenary.o \
    profile_linear.o \
    profile_munk.o \
    profile_n2.o \
    reflect_loss_constant.o \
    reflect_loss_rayleigh.o \
    boundary_flat.o \
    boundary_slope.o \

###########################################
# ocean profile

attenuation_constant.o: \
	attenuation_constant.cc \
	attenuation_constant.h \
	attenuation_model.h \
	$(DEPENDS)

attenuation_thorp.o: \
	attenuation_thorp.cc \
	attenuation_thorp.h \
	attenuation_model.h \
	$(DEPENDS)

profile_model.o: \
	profile_model.cc \
	profile_model.h \
	attenuation_model.h \
	$(DEPENDS)

profile_catenary.o: \
	profile_catenary.cc \
	profile_catenary.h \
	profile_model.h \
	attenuation_model.h \
	$(DEPENDS)

profile_linear.o: \
	profile_linear.cc \
	profile_linear.h \
	profile_model.h \
	attenuation_model.h \
	$(DEPENDS)

profile_munk.o: \
	profile_munk.cc \
	profile_munk.h \
	profile_model.h \
	attenuation_model.h \
	$(DEPENDS)

profile_n2.o: \
	profile_n2.cc \
	profile_n2.h \
	profile_model.h \
	attenuation_model.h \
	$(DEPENDS)

###########################################
# ocean boundary

reflect_loss_constant.o: \
	reflect_loss_constant.cc \
	reflect_loss_constant.h \
	reflect_loss_model.h \
	$(DEPENDS)

reflect_loss_rayleigh.o: \
	reflect_loss_rayleigh.cc \
	reflect_loss_rayleigh.h \
	reflect_loss_model.h \
	$(DEPENDS)

boundary_flat.o: \
	boundary_flat.cc \
	boundary_flat.h \
	boundary_model.h \
	reflect_loss_model.h \
	$(DEPENDS)

boundary_slope.o: \
	boundary_slope.cc \
	boundary_slope.h \
	boundary_model.h \
	reflect_loss_model.h \
	$(DEPENDS)

