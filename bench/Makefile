
####################################################################
# Configuration

# Paths to auxiliary Makefile definitions

TOOLS_BUILD_FOLDER=../../pasl/tools/build


####################################################################
# Mandatory options

USE_PTHREADS=1
USE_MATH=1


####################################################################
# Default options

USE_ALLOCATOR=
USE_HWLOC=0
USE_NUMA=0

# todo activate? USE_ANSI=1

USE_FATAL_ERRORS=1
VERBOSE_MAKE=0
VERBOSE_DEPENDENCIES=0

PROGRAMS=\
	psort.cpp \
	samplesort.cpp

####################################################################
# Makefile options

# Create a file called "settings.sh" in this folder if you want to
# configure particular options. See section below for options.

-include settings.sh

# Include here

# Options are then configured by the auxiliary file below

include $(TOOLS_BUILD_FOLDER)/Makefile_options


####################################################################
# Modes

# What are the compilation mode supported, i.e. the "modes"
# (If extending the list, need to add cases for the definition
# of COMPILE_OPTIONS_FOR further below, and also for "clean".

MODES=dbg log sta opt seq cilk

# Compilation options for each mode

COMPILE_OPTIONS_COMMON=-DDISABLE_INTERRUPTS -DSTATS_IDLE $(OPTIONS_ALL)
COMPILE_OPTIONS_FOR_dbg=$(OPTIONS_DEBUG) -DSTATS -DDEBUG -DUSE_PASL_RUNTIME # -DUSE_UCONTEXT  -DLOGGING
COMPILE_OPTIONS_FOR_log=$(OPTIONS_O2) -DSTATS -DLOGGING -DUSE_PASL_RUNTIME
COMPILE_OPTIONS_FOR_sta=$(OPTIONS_O2) -DSTATS -DUSE_PASL_RUNTIME
COMPILE_OPTIONS_FOR_seq=$(OPTIONS_O2) -DSTATS -DSEQUENTIAL_ELISION
COMPILE_OPTIONS_FOR_opt=$(OPTIONS_O2) -DUSE_PASL_RUNTIME
COMPILE_OPTIONS_FOR_cilk=$(OPTIONS_cilk) -DUSE_CILK_PLUS_RUNTIME $(OPTIONS_O2)

# Folders where to find all the header files and main sources

PCTL_PATH=../../pctl/include
PCTL_PBBS_PATH=../include
CHUNKEDSEQ_PATH=../../chunkedseq/include
BENCH_PATH=./include/
GENERATORS_PATH=$(BENCH_PATH)/generators

PBBS_PCTL_PATHS=$(PCTL_PATH) $(PCTL_PBBS_PATH) $(CHUNKEDSEQ_PATH) $(BENCH_PATH) $(GENERATORS_PATH)

PASL_PATHS=$(SEQUTIL_PATH) $(PARUTIL_PATH) $(SCHED_PATH) 

INCLUDES=. $(PASL_PATHS) $(PBBS_PCTL_PATHS)

# Folders where to find all the source files

FOLDERS=$(INCLUDES)


####################################################################
# Targets

all: opt

opt: $(PROGRAMS:.cpp=.opt)

####################################################################
# Clean


clean: clean_build clean_modes


####################################################################
# Main rules for the makefile

include $(TOOLS_BUILD_FOLDER)/Makefile_modes
