
# variables which should to be set:
# SPIKE_INCLUDE_DIR - path to spike headers
ifeq ($(SPIKE_INCLUDE_DIR),)
$(warning SPIKE_INCLUDE_DIR is not set)
endif

# Useful targets:
# default - builds the demo.
# clean - removes all generated files.
#
# Useful variables:
# CXX - C++ compiler to use.
# CFLAGS - additional flags for the C compiler.
# LDFLAGS - additional flags for the linker.

# This is the default target.
demo:

# Prevent deletion of intermediate files.
.SECONDARY:

# Get the directory of this makefile.
ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
VPATH := $(ROOT_DIR)

# Source files
CPPLIST := \
	sc_main.cpp \
	turbo/turbo_core.cpp \
	uncore/turbo_uncore.cpp

# Object and dependency files
OBJS := $(CPPLIST:.cpp=.o)
DEPS := $(CPPLIST:.cpp=.d)

# Compiler flags
CFLAGS := -Os -fPIC
DEMO_CXXFLAGS := \
	-MMD \
	-std=c++17 \
	-I$(ROOT_DIR) \
	$(CFLAGS) \
	-I$(SYSTEMC_INCLUDE) \
	-I$(SPIKE_INCLUDE_DIR) # TODO: Fix this

DEMO_LDFLAGS := -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib -Wl,--no-as-needed $(LDFLAGS)

DEMO_LDLIBS := -latomic -lriscv -lsoftfloat -ldisasm -lstdc++fs $(SYSTEMC_LIBDIR)/libsystemc.a $(LDLIBS)

-include $(DEPS)

%.o: %.cpp
	$(CXX) -c $(DEMO_CXXFLAGS) -o $@ -c $<

demo: $(OBJS) $(SYSTEMC_LIBDIR)/libsystemc.a
	$(CXX) $(DEMO_LDFLAGS) $^ -o $@ $(DEMO_LDLIBS)


.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPS) demog