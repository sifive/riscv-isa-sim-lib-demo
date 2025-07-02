# Useful targets:
# compile_only - compiles the source files into object files.
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
    demo_core.cc \
    main.cc \
    memory_simulator.cc

# Object and dependency files
OBJS := $(CPPLIST:.cc=.o)
DEPS := $(CPPLIST:.cc=.d)

# Compiler flags
CFLAGS := -Os -fPIC
DEMO_CXXFLAGS := \
    -MMD \
    -std=c++17 \
    -I$(ROOT_DIR) \
    $(CFLAGS) \
	-I$(SPIKE_SOURCE_DIR) # check why is this now needed


DEMO_LDFLAGS := -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib -Wl,--no-as-needed $(LDFLAGS)

DEMO_LDLIBS := -latomic -lriscv -lsoftfloat -ldisasm -lstdc++fs $(LDLIBS) 

-include $(DEPS)

%.o: %.cc
	$(CXX) -c $(DEMO_CXXFLAGS) -o $@ -c $<

demo: $(OBJS)
	$(CXX) $(DEMO_LDFLAGS) $^ -o $@ $(DEMO_LDLIBS)

# Compilation rule - make sure it's ONLY compiling
.PHONY: compile_only
compile_only: $(OBJS)

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPS) demo


# Spike

# TODO if we want a default value here, then there is no point in checking for it with
# the check_spike_dir target.
#SPIKE_SOURCE_DIR ?= $(ROOT_DIR)/../riscv-isa-sim

# Check for required environment variable
.PHONY: check_spike_dir
check_spike_dir:
	@if [ -z "$(SPIKE_SOURCE_DIR)" ]; then \
		echo "ERROR: SPIKE_SOURCE_DIR is not set. Please set it to point to your Spike repository and run ./src/setup.sh first"; \
		exit 1; \
	 fi

# Directory definitions
SPIKE_DIR = $(patsubst %/,%,$(SPIKE_SOURCE_DIR))
SPIKE_BUILD_DIR = $(SPIKE_DIR)/build
SPIKE_INSTALL_DIR = $(ROOT_DIR)/spike_install
SPIKE_CONFIGURE_STAMP = $(SPIKE_BUILD_DIR)/.configure_stamp

# Define base flags
SPIKE_CLFAGS := -fPIC -O2
SPIKE_CXXFLAGS := -fPIC -O2 -std=c++17

# Add hooks.h if USE_HOOKS is defined
ifdef USE_HOOKS
    HOOK_FLAGS := -I$(ROOT_DIR)/hooks.h
    SPIKE_CFLAGS += $(HOOK_FLAGS)
    SPIKE_CXXFLAGS += $(HOOK_FLAGS)
endif

$(SPIKE_CONFIGURE_STAMP):
	@if [ ! -f "$(SPIKE_BUILD_DIR)/config.status" ]; then \
		mkdir -p $(SPIKE_BUILD_DIR) && \
		cd $(SPIKE_BUILD_DIR) && \
		$(SPIKE_DIR)/configure \
			--prefix=$(SPIKE_INSTALL_DIR) \
			CFLAGS="$(SPIKE_CFLAGS)" \
			CXXFLAGS="$(SPIKE_CXXFLAGS)"; \
	fi
	touch $(SPIKE_CONFIGURE_STAMP)

# Regular (non-PHONY) target
.PHONY: spike_build
spike_build: $(SPIKE_CONFIGURE_STAMP) | check_spike_dir
	$(MAKE) -C $(SPIKE_BUILD_DIR)
	$(MAKE) -C $(SPIKE_BUILD_DIR) install

.PHONY: reconfigure_spike
reconfigure_spike: | check_spike_dir
	$(MAKE) clean_spike
	$(MAKE) $(SPIKE_CONFIGURE_STAMP)

# Cleaning targets
.PHONY: clean_spike
clean_spike: | check_spike_dir
	$(RM) -r $(SPIKE_BUILD_DIR) $(SPIKE_INSTALL_DIR)
	$(RM) $(SPIKE_CONFIGURE_STAMP)
