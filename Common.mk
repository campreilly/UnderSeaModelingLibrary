# Common.mk - Define usual target commands for Under Sea Modeling Library (USML)
#
# 	clean 	- remove compiled code and test results from directory
#	all	- build all compile targets (to support "make clean all")
#	default	- same as all
#

.PHONY: clean all default 

default: all

clean:
	-rm -f *.o *.csv *.nc *.log
