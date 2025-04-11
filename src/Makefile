default: spike_demo

.SECONDARY:

# Directory definitions
SPIKE_DIR = $(PWD)/riscv-isa-sim
SPIKE_BUILD_DIR = $(PWD)/riscv-isa-sim/build
SPIKE_INSTALL_DIR = $(PWD)/spike_install
SPIKE_CONFIGURE_STAMP = $(SPIKE_BUILD_DIR)/.configure_stamp

# Spike LIBS
SPIKE_LIBS = $(SPIKE_INSTALL_DIR)/lib/libriscv.a \
             $(SPIKE_INSTALL_DIR)/lib/libsoftfloat.a \
             $(SPIKE_INSTALL_DIR)/lib/libdisasm.a

# Source files
CPPLIST = \
    s2_demo_proc.cc \
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

LDFLAGS = -std=c++17

LDLIBS = \
    -latomic \
    $(SPIKE_LIBS)

$(SPIKE_CONFIGURE_STAMP):
	mkdir -p $(SPIKE_BUILD_DIR)
	cd $(SPIKE_BUILD_DIR) && $(SPIKE_DIR)/configure --prefix=$(SPIKE_INSTALL_DIR) \
		CFLAGS="-fPIC -O2" \
		CXXFLAGS="-fPIC -O2 -std=c++17"
	touch $(SPIKE_CONFIGURE_STAMP)

# Rule to build Spike
.PHONY: force_spike_build
force_spike_build: $(SPIKE_CONFIGURE_STAMP)
	$(MAKE) -C $(SPIKE_BUILD_DIR)
	$(MAKE) -C $(SPIKE_BUILD_DIR) install

$(SPIKE_LIBS): force_spike_build

# Compilation rule
%.o: %.cc
	$(CXX) $(CXXFLAGS) $(USERDEFINES) -c -o $@ $< -MP -MMD

# Link rules
CORE_OBJS = $(OBJS)

spike_demo: $(CORE_OBJS) $(SPIKE_LIBS)
	$(CXX) $(LDFLAGS) $(CORE_OBJS) -o $@ $(LDLIBS)

-include $(DEPS)

# Cleaning targets
.PHONY: clean clean_spike reconfigure_spike rebuild_spike build_spike
clean_spike:
	rm -rf $(SPIKE_BUILD_DIR) $(SPIKE_INSTALL_DIR)
	rm -f $(SPIKE_CONFIGURE_STAMP)

clean: clean_spike
	rm -f $(OBJS) $(DEPS) spike_demo

reconfigure_spike: clean_spike $(SPIKE_CONFIGURE_STAMP)

build_spike: $(SPIKE_LIBS)

rebuild_spike: clean_spike $(SPIKE_LIBS)
