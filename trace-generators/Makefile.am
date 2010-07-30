################################################################################
#	\file Makefile.am
#	\author Gregory Diamos
#	\date Thursday July 29, 2010
# 	\brief Automake input file describing the programs and sources needed to
#			build the Ocelot
################################################################################

################################################################################
## Global Parameters
################################################################################

################################################################################
## Autoconf configuration
ACLOCAL_AMFLAGS = -I m4
################################################################################

################################################################################
## Programs and Libraries
BUILT_SOURCES =
bin_PROGRAMS = 
check_PROGRAMS = 
EXTRA_PROGRAMS = MemoryTraceAnalyzer InstructionTraceAnalyzer \
	BranchTraceAnalyzer ParallelismTraceAnalyzer SharedComputationAnalyzer
lib_LTLIBRARIES = libocelotTrace.la
################################################################################

################################################################################
## libocelotTrace
libocelotTrace_la_CXXFLAGS = -Wall -ansi -pedantic -Werror -std=c++0x
libocelotTrace_la_SOURCES = \
	traces/implementation/BranchTraceGenerator.cpp \
	traces/implementation/ParallelismTraceGenerator.cpp \
	traces/implementation/SharedComputationGenerator.cpp \
	traces/implementation/MemoryTraceGenerator.cpp \
	traces/implementation/CacheSimulator.cpp \
	traces/implementation/InstructionTraceGenerator.cpp \
	traces/implementation/WarpSynchronousGenerator.cpp \
	traces/implementation/PerformanceBoundGenerator.cpp \
	traces/implementation/ConvergenceGenerator.cpp
	
################################################################################

################################################################################
## ParallelismTraceAnalyzer
ParallelismTraceAnalyzer_CXXFLAGS = -Wall -ansi -pedantic -Werror -std=c++0x
ParallelismTraceAnalyzer_SOURCES = \
	traces/implementation/ParallelismTraceAnalyzer.cpp
ParallelismTraceAnalyzer_LDADD = libocelotTrace.la
ParallelismTraceAnalyzer_LDFLAGS = -static
################################################################################
	
################################################################################
## InstructionTraceAnalyzer
InstructionTraceAnalyzer_CXXFLAGS = -Wall -ansi -pedantic -Werror -std=c++0x
InstructionTraceAnalyzer_SOURCES = \
	traces/implementation/InstructionTraceAnalyzer.cpp
InstructionTraceAnalyzer_LDADD = libocelotTrace.la
InstructionTraceAnalyzer_LDFLAGS = -static
################################################################################
	
################################################################################
## BranchTraceAnalyzer
BranchTraceAnalyzer_CXXFLAGS = -Wall -ansi -pedantic -Werror -std=c++0x
BranchTraceAnalyzer_SOURCES = traces/implementation/BranchTraceAnalyzer.cpp
BranchTraceAnalyzer_LDADD = libocelotTrace.la
BranchTraceAnalyzer_LDFLAGS = -static
################################################################################
	
################################################################################
## SharedComputationAnalyzer
SharedComputationAnalyzer_CXXFLAGS = -Wall -ansi -pedantic -Werror -std=c++0x
SharedComputationAnalyzer_SOURCES = \
	traces/implementation/SharedComputationAnalyzer.cpp
SharedComputationAnalyzer_LDADD = libocelotTrace.la
SharedComputationAnalyzer_LDFLAGS = -static
################################################################################
	
################################################################################
## MemoryTraceAnalyzer
MemoryTraceAnalyzer_CXXFLAGS = -Wall -ansi -pedantic -Werror -std=c++0x
MemoryTraceAnalyzer_SOURCES = \
	traces/implementation/MemoryTraceAnalyzer.cpp
MemoryTraceAnalyzer_LDADD = libocelotTrace.la
MemoryTraceAnalyzer_LDFLAGS = -static
################################################################################

################################################################################
## Headers
nobase_include_HEADERS = traces/interface/MemoryTraceGenerator.h \
	traces/interface/BranchTraceAnalyzer.h \
	traces/interface/SharedComputationGenerator.h \
	traces/interface/ParallelismTraceGenerator.h \
	traces/interface/MemoryTraceAnalyzer.h \
	traces/interface/SharedComputationAnalyzer.h \
	traces/interface/BranchTraceGenerator.h \
	traces/interface/ParallelismTraceAnalyzer.h \
	traces/interface/KernelEntry.h \
	traces/interface/CacheSimulator.h \
	traces/interface/InstructionTraceGenerator.h \
	traces/interface/MachineAttributesGenerator.h \
	traces/interface/InstructionTraceAnalyzer.h \
	traces/interface/KernelDimensionsGenerator.h \
	traces/interface/WatchTraceGenerator.h \
	traces/interface/PerformanceBoundGenerator.h \
	traces/interface/ConvergenceGenerator.h
################################################################################

################################################################################
## Extra clean rules
CLEANFILES = 
################################################################################
