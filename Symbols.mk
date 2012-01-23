# Symbols.mk - Define make symbols for Under Sea Modeling Library (USML)
#
# Inputs:
#	USML_ROOT - parent of USML directory
#	USML_DEBUG - turn on debugging if defined
# Outputs:
#	CPPFLAGS - C++ compiler options
#	LDFLAGS  - C++ linker options
#

# setup compiler options based on values of USML_ROOT & USML_DEBUG

# USML_ROOT=/home/sreilly/Projects

ifdef USML_DEBUG
    CPPFLAGS += -O1 -g 
	# -O1 used only because UBLAS is having problems with -O0
else
    CPPFLAGS += -O3 -g -DNDEBUG -ffast-math
endif

CPPFLAGS += -pedantic -std=c++98 -Wall -Werror -fPIC -I$(USML_ROOT) 
	# -pedantic -std=c++98 -Wall -Werror options not required
	# just a good idea for cross compiler compatibility

LIB_LIST = \
    -L$(USML_ROOT)/usml \
    -lusml \
    -lboost_unit_test_framework \
    -lnetcdf_c++ \
    -lnetcdf \
    -lm
    
LDFLAGS += $(TARGET_ARCH) $(LIB_LIST)

# check to see if USML_ROOT is set up correctly
# throw error if Symbols.mk can't be found on disk

ifneq (1,$(words $(wildcard $(USML_ROOT)/usml/Symbols.mk*)))
$(error Error: set USML_ROOT to parent of USML directory)
endif

