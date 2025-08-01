# check if CXX not defined
ifeq ($(CXX),)
CXX = g++
endif

CXXFLAGS = -std=c++2a -fPIC -O2 -Wall -MMD -MP -D_GNU_SOURCE

ifeq ($(SPIKE_DIR),)
$(error SPIKE_DIR is not set)
endif

# cehck if Spike install area is in PATH
# check if SPIKE_DIR/lib is in LD_LIBRARY_PATH
# check if LD_LIBRARY_PATH is set
# ifeq ($(LD_LIBRARY_PATH),)
# $(error LD_LIBRARY_PATH is not set)
# endif

SPIKE_INCLUDE_DIR = $(SPIKE_DIR)/include
INCLUDES = \
	-I$(SPIKE_INCLUDE_DIR)/riscv \
	-I$(SPIKE_INCLUDE_DIR)/fesvr \
	-I$(SPIKE_INCLUDE_DIR)/disasm \
	-I$(SPIKE_INCLUDE_DIR)/softfloat

# Source files
SOURCES = xperia.cc xperiv.cc

OBJECTS = $(SOURCES:.cc=.o)
DEPS = $(OBJECTS:.o=.d)

# Link against required libraries
LIBS = -lriscv -ldisasm -lsoftfloat

TARGET = libxperi.so

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -L$(SPIKE_DIR)/lib -shared -o $@ $^ $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPS)

-include $(DEPS)

.PHONY: all clean
