default: check_prereqs spike_demo

.SECONDARY:

# Check for required environment variable
ifndef SPIKE_SOURCE_DIR
$(error SPIKE_SOURCE_DIR is not set. Please set it to point to your Spike repository and run ./src/spike_s2_demo.sh first)
endif

# Directory definitions
SPIKE_DIR = $(SPIKE_SOURCE_DIR)
SPIKE_BUILD_DIR = $(SPIKE_DIR)/build
SPIKE_INSTALL_DIR = $(PWD)/spike_install
SPIKE_CONFIGURE_STAMP = $(SPIKE_BUILD_DIR)/.configure_stamp

# Copy files stamp (managed by spike_s2_demo.sh)
COPY_FILES_STAMP = $(SPIKE_DIR)/.copy_files_stamp

# Check prerequisites
.PHONY: check_prereqs
check_prereqs:
	@if [ ! -f "$(COPY_FILES_STAMP)" ]; then \
		echo "Error: Spike files are not patched yet"; \
		echo "Please run './src/spike_s2_demo.sh' first to patch the files"; \
		echo "Make sure SPIKE_SOURCE_DIR is set correctly"; \
		exit 1; \
	fi


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

$(SPIKE_CONFIGURE_STAMP): check_prereqs
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
