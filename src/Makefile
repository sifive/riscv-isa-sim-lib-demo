default: demo

.SECONDARY:

# Directory definitions
# SPIKE_DIR = $(patsubst %/,%,$(SPIKE_SOURCE_DIR))
# SPIKE_BUILD_DIR = $(SPIKE_DIR)/build
# SPIKE_INSTALL_DIR = $(PWD)/spike_install
# SPIKE_CONFIGURE_STAMP = $(SPIKE_BUILD_DIR)/.configure_stamp

SPIKE_INLCUDE_DIR=
SPIKE_LIBDIR=

# Check for required environment variable
.PHONY: check_spike_dir
check_spike_dir:
	@if [ -z "$(SPIKE_SOURCE_DIR)" ]; then \
		echo "ERROR: SPIKE_SOURCE_DIR is not set. Please set it to point to your Spike repository and run ./src/setup.sh first"; \
		exit 1; \
	 fi

# Source files
CPPLIST = \
    demo_core.cc \
    main.cc \
    memory_simulator.cc

# Object and dependency files
OBJS = $(CPPLIST:.cc=.o)
DEPS = $(CPPLIST:.cc=.d)

# Compiler flags
CXXFLAGS = \
    -MMD \
    -std=c++17 \
    -fPIC \
    -Os \
    -DVL_THREADED \
    -I. \
    $(SPIKE_INCLUDES)

#LDFLAGS = -std=c++17 -L$(SPIKE_LIBDIR) -Wl,-rpath,$(SPIKE_INSTALL_DIR)/lib

LDLIBS = -latomic -lriscv -lsoftfloat -ldisasm

# Define base flags
BASE_CFLAGS := -fPIC -O2
BASE_CXXFLAGS := -fPIC -O2 -std=c++17

# Capture original PWD at the start
ORIG_PWD := $(shell pwd)
# Add hooks.h if USE_HOOKS is defined
ifdef USE_HOOKS
    HOOK_FLAGS := -include $(ORIG_PWD)/hooks.h
    BASE_CFLAGS += $(HOOK_FLAGS)
    BASE_CXXFLAGS += $(HOOK_FLAGS)
endif


# $(SPIKE_CONFIGURE_STAMP):
# 	@if [ ! -f "$(SPIKE_BUILD_DIR)/config.status" ]; then \
# 		mkdir -p $(SPIKE_BUILD_DIR); \
# 		cd $(SPIKE_BUILD_DIR) && \
# 		CXXFLAGS="$(BASE_CXXFLAGS)" $(SPIKE_DIR)/configure \
# 			--prefix=$(SPIKE_INSTALL_DIR) \
# 			CFLAGS="$(BASE_CFLAGS)" \
# 			CXXFLAGS="$(BASE_CXXFLAGS)"; \
# 	fi
# 	touch $(SPIKE_CONFIGURE_STAMP)

# Regular (non-PHONY) target
# spike_build: $(SPIKE_CONFIGURE_STAMP)
# 	$(MAKE) -C $(SPIKE_BUILD_DIR)
# 	$(MAKE) -C $(SPIKE_BUILD_DIR) install

# Rule to build Spike
# .PHONY: force_spike_build
# force_spike_build: $(SPIKE_CONFIGURE_STAMP)
# 	$(MAKE) -C $(SPIKE_BUILD_DIR)
# 	$(MAKE) -C $(SPIKE_BUILD_DIR) install

# Compilation rule - make sure it's ONLY compiling
%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $(USERDEFINES) -o $@ -c $<

.PHONY: compile_only
compile_only: $(OBJS)

# Link with LD_LIBRARY_PATH
demo: $(OBJS)
	$(CXX) -std=c++17 $^ -o demo \
		$(SPIKE_LIBDIR) \
		-Wl,-rpath,. \
		-Wl,-rpath,./lib \
		-Wl,--no-as-needed \
		-lstdc++fs \
		-lriscv \
		-lsoftfloat \
		-latomic

-include $(DEPS)

# Cleaning targets
# .PHONY: clean clean_spike reconfigure_spike rebuild_spike build_spike
# clean_spike:
# 	rm -rf $(SPIKE_BUILD_DIR) $(SPIKE_INSTALL_DIR)
# 	rm -f $(SPIKE_CONFIGURE_STAMP)

clean: 
	rm -f $(OBJS) $(DEPS) demo

# reconfigure_spike: clean_spike $(SPIKE_CONFIGURE_STAMP)

# build_spike: $(SPIKE_LIBS)

# rebuild_spike: clean_spike $(SPIKE_LIBS)
