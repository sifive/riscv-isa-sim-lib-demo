default: demo

.SECONDARY:

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
    -I$(SPIKE_DIR)

LDFLAGS = -std=c++17 -L$(SPIKE_INSTALL_DIR)/lib -Wl,-rpath,$(SPIKE_INSTALL_DIR)/lib

LDLIBS = \
    -latomic \
    -L$(SPIKE_INSTALL_DIR)/lib -lriscv -lsoftfloat -ldisasm

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

# Directory definitions
SPIKE_DIR = $(patsubst %/,%,$(SPIKE_SOURCE_DIR))
SPIKE_BUILD_DIR = $(SPIKE_DIR)/build
SPIKE_INSTALL_DIR = $(PWD)/spike_install
SPIKE_CONFIGURE_STAMP = $(SPIKE_BUILD_DIR)/.configure_stamp

$(SPIKE_CONFIGURE_STAMP):
	@if [ ! -f "$(SPIKE_BUILD_DIR)/config.status" ]; then \
		mkdir -p $(SPIKE_BUILD_DIR); \
		cd $(SPIKE_BUILD_DIR) && \
		CXXFLAGS="$(BASE_CXXFLAGS)" $(SPIKE_DIR)/configure \
			--prefix=$(SPIKE_INSTALL_DIR) \
			CFLAGS="$(BASE_CFLAGS)" \
			CXXFLAGS="$(BASE_CXXFLAGS)"; \
	fi
	touch $(SPIKE_CONFIGURE_STAMP)

# Regular (non-PHONY) target
spike_build: $(SPIKE_CONFIGURE_STAMP)
	$(MAKE) -C $(SPIKE_BUILD_DIR)
	$(MAKE) -C $(SPIKE_BUILD_DIR) install

# Rule to build Spike
.PHONY: force_spike_build
force_spike_build: $(SPIKE_CONFIGURE_STAMP)
	$(MAKE) -C $(SPIKE_BUILD_DIR)
	$(MAKE) -C $(SPIKE_BUILD_DIR) install

# Compilation rule - make sure it's ONLY compiling
%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $(USERDEFINES) -o $@ $<

# Compile individual files without linking
main.o: main.cc
	$(CXX) -c $(CXXFLAGS) $(USERDEFINES) -o $@ $<

demo_core.o: demo_core.cc
	$(CXX) -c $(CXXFLAGS) $(USERDEFINES) -o $@ $<

memory_simulator.o: memory_simulator.cc
	$(CXX) -c $(CXXFLAGS) $(USERDEFINES) -o $@ $<

# Basic compilation flags (no Spike dependencies)
BASIC_CXXFLAGS = -std=c++17 -fPIC -Os -I.

compile_only:
	$(CXX) -c $(BASIC_CXXFLAGS) -I$(SPIKE_DIR) main.cc
	$(CXX) -c $(BASIC_CXXFLAGS) -I$(SPIKE_DIR) demo_core.cc
	$(CXX) -c $(BASIC_CXXFLAGS) -I$(SPIKE_DIR) memory_simulator.cc

# Link with LD_LIBRARY_PATH
link_demo_dynamic: $(OBJS)
	$(CXX) -std=c++17 $(OBJS) -o demo \
		-Wl,-rpath,$(LD_LIBRARY_PATH) \
		-Wl,--no-as-needed \
		$(LD_LIBRARY_PATH)/libriscv.so \
		$(LD_LIBRARY_PATH)/libsoftfloat.so \
		-latomic

# Full build process with dynamic linking
demo: compile_only link_demo_dynamic
-include $(DEPS)

# Cleaning targets
.PHONY: clean clean_spike reconfigure_spike rebuild_spike build_spike
clean_spike:
	rm -rf $(SPIKE_BUILD_DIR) $(SPIKE_INSTALL_DIR)
	rm -f $(SPIKE_CONFIGURE_STAMP)

clean: clean_spike
	rm -f $(OBJS) $(DEPS) demo

reconfigure_spike: clean_spike $(SPIKE_CONFIGURE_STAMP)

build_spike: $(SPIKE_LIBS)

rebuild_spike: clean_spike $(SPIKE_LIBS)
