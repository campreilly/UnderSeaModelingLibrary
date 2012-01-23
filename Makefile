# Top-level Makefile for Under Sea Modeling Library (USML)

include Symbols.mk

LIBRARIES = \
	ublas \
	types \
	netcdf \
	ocean \
	wave_q3d \

STUDIES = \
	studies/malta_movie \
	studies/pedersen \
	studies/ray_speed

.PHONY: all default doc clean dist dist-src dist-doc dist-data

# build all libraries

all: \
	libusml.so \
	usml_test \

default: all

# document all libraries

doc:
	doxygen 

# clean all libraries

clean:
	$(foreach DIR,$(LIBRARIES),$(MAKE) -w -C $(DIR) clean;)
	$(foreach DIR,$(LIBRARIES),$(MAKE) -w -C $(DIR)/test clean;)
	$(foreach DIR,$(STUDIES),$(MAKE) -w -C $(DIR) clean;)
	-rm -rf html
	-rm -f usml_test.o libusml.so usml_test 

# build shared library
#   - assume that every *.cc file in $(LIBRARIES) coverts to *.o file
#   - call "make all" in each $(LIBRARIES) to generate *.o files
#   - link all these *.o files into $(USML_LIB)     

LIBSRC := $(foreach dir,$(LIBRARIES),$(wildcard $(dir)/*.cc))
LIBINC := $(foreach dir,$(LIBRARIES),$(wildcard $(dir)/*.h))
LIBOBJ := $(LIBSRC:.cc=.o)

libusml.so: $(LIBSRC) $(LIBINC)
	$(foreach DIR,$(LIBRARIES),$(MAKE) -w -C $(DIR) all;)
	$(CXX) -shared -o $@ $(LIBOBJ)

# build test routines
#   - assume that every *_test.cc file in $(LIBRARIES)/test coverts to *.o file
#	- ignores other *.cc files in these directories to all standalone executables
#   - call "make all" in each $(LIBRARIES) to generate *.o files
#   - compile usml_test.cc test driver
#   - collect all automated tests (*_test.o) files into $(TSTOBJ)     
#   - test/Makefiles may generate standalone apps and data 

TSTDEP := $(foreach dir,$(LIBRARIES),$(wildcard $(dir)/test/*.cc))
TSTSRC := $(foreach dir,$(LIBRARIES),$(wildcard $(dir)/test/*_test.cc))
TSTOBJ := $(TSTSRC:.cc=.o)

usml_test.o: usml_test.cc
	$(COMPILE.cc) $< $(OUTPUT_OPTION)

usml_test: $(TSTDEP) usml_test.o libusml.so
	$(foreach DIR,$(LIBRARIES),$(MAKE) -w -C $(DIR)/test all;)
	$(CXX) -o $@ usml_test.o $(TSTOBJ) libusml.so $(LDFLAGS)


# build distribution media

DIST_VERSION = 0.01
DIST_FILES = \
	usml/Common.mk \
	usml/Doxyfile \
	usml/DoxygenLayout.xml \
	usml/Mainpage.h \
	usml/Makefile \
	usml/Symbols.mk \
	usml/todo.txt \
	usml/usml_test.cc \
	usml/matlab \
	$(addprefix usml/,$(LIBRARIES)) \
	usml/studies/README.txt \
	$(addprefix usml/,$(STUDIES))

dist: dist-src dist-doc dist-data

dist-src: clean
	cd .. ; tar -czf usml-src-$(DIST_VERSION).tar.gz $(DIST_FILES)

dist-doc: doc
	cd .. ; tar -czf usml-doc-$(DIST_VERSION).tar.gz usml/html
	cp usml-install.txt ..

dist-data: 
	cd .. ; tar -czvf usml-data-$(DIST_VERSION).tar.gz usml/data

