
####################################################################
# Configuration

# Paths to auxiliary Makefile definitions

TOOLS_BUILD_FOLDER=../../pasl/tools/build

PBENCH_PATH=../../pbench

include $(PBENCH_PATH)/Makefile_common


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
	granularity_bench.cpp \
	merkletree_bench.cpp \
	psort.cpp \
	blockradixsort_bench.cpp \
	samplesort_bench.cpp \
	deterministichash_bench.cpp \
	suffixarray_bench.cpp \
	quickhull_bench.cpp \
	nearestneighbours_bench.cpp \
        raycast_bench.cpp \
        reduce_bench.cpp \
        scan_bench.cpp \
        loop_bench.cpp \
        delaunay_bench.cpp \
        delaunayrefine_bench.cpp \
        bfs_bench.cpp \
        pbfs_bench.cpp \
        sequence_data.cpp \

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

MODES=manc unkm25 unkm50 unkm100 unkm400 unkm500 unkm unks25 unks50 unks100 unks400 unks500 unks

# Compilation options for each mode

COMPILE_OPTIONS_COMMON=-DDISABLE_INTERRUPTS -DSTATS_IDLE  -lssl -lcrypto $(OPTIONS_ALL)
COMPILE_OPTIONS_FOR_cilk=$(OPTIONS_cilk) -DUSE_CILK_PLUS_RUNTIME $(OPTIONS_O2)

COMPILE_OPTIONS_FOR_manc=$(COMPILE_OPTIONS_FOR_cilk) -DTIMING -DMANUAL_CONTROL

COMPILE_OPTIONS_FOR_unkm25=$(COMPILE_OPTIONS_FOR_cilk) -DTIMING -DEASYOPTIMISTIC -DKAPPA25
COMPILE_OPTIONS_FOR_unkm50=$(COMPILE_OPTIONS_FOR_cilk) -DTIMING -DEASYOPTIMISTIC -DKAPPA50
COMPILE_OPTIONS_FOR_unkm100=$(COMPILE_OPTIONS_FOR_cilk) -DTIMING -DEASYOPTIMISTIC -DKAPPA100
COMPILE_OPTIONS_FOR_unkm300=$(COMPILE_OPTIONS_FOR_cilk) -DTIMING -DEASYOPTIMISTIC -DKAPPA300
COMPILE_OPTIONS_FOR_unkm400=$(COMPILE_OPTIONS_FOR_cilk) -DTIMING -DEASYOPTIMISTIC -DKAPPA400
COMPILE_OPTIONS_FOR_unkm500=$(COMPILE_OPTIONS_FOR_cilk) -DTIMING -DEASYOPTIMISTIC -DKAPPA500
COMPILE_OPTIONS_FOR_unkm=$(COMPILE_OPTIONS_FOR_unkm300)

COMPILE_OPTIONS_FOR_unks25=$(COMPILE_OPTIONS_FOR_cilk) -DEASYOPTIMISTIC -DSMART_ESTIMATOR -DKAPPA25
COMPILE_OPTIONS_FOR_unks50=$(COMPILE_OPTIONS_FOR_cilk) -DEASYOPTIMISTIC -DSMART_ESTIMATOR -DKAPPA50
COMPILE_OPTIONS_FOR_unks100=$(COMPILE_OPTIONS_FOR_cilk) -DEASYOPTIMISTIC -DSMART_ESTIMATOR -DKAPPA100
COMPILE_OPTIONS_FOR_unks300=$(COMPILE_OPTIONS_FOR_cilk) -DEASYOPTIMISTIC -DSMART_ESTIMATOR -DKAPPA300
COMPILE_OPTIONS_FOR_unks400=$(COMPILE_OPTIONS_FOR_cilk) -DEASYOPTIMISTIC -DSMART_ESTIMATOR -DKAPPA400
COMPILE_OPTIONS_FOR_unks500=$(COMPILE_OPTIONS_FOR_cilk) -DEASYOPTIMISTIC -DSMART_ESTIMATOR -DKAPPA500
COMPILE_OPTIONS_FOR_unks=$(COMPILE_OPTIONS_FOR_unks300)

# Folders where to find all the header files and main sources

PCTL_PATH=../../pctl/include
PCTL_PBBS_PATH=../include
CHUNKEDSEQ_PATH=../../chunkedseq/include
BENCH_PATH=./include/
GENERATORS_PATH=$(BENCH_PATH)/generators
PBBS_PATH=../../pbbs-include
CMDLINE_PATH=../../cmdline/include

PBBS_PCTL_PATHS=$(PCTL_PATH) $(PCTL_PBBS_PATH) $(CHUNKEDSEQ_PATH) $(BENCH_PATH) $(GENERATORS_PATH)

PASL_PATHS=$(SEQUTIL_PATH) $(PARUTIL_PATH) $(SCHED_PATH)

INCLUDES=. $(PASL_PATHS) $(PBBS_PCTL_PATHS) $(PBBS_PATH) $(CMDLINE_PATH)

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
