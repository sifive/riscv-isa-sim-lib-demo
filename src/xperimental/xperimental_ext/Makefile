CXX = g++
CXXFLAGS = -std=c++2a -fPIC -O2 -Wall -MMD -MP -D_GNU_SOURCE
#SPIKE_DIR = needs to be defined
INCLUDES = -I$(SPIKE_DIR)/riscv -I$(SPIKE_DIR)/fesvr -I$(SPIKE_DIR)/disasm -I$(SPIKE_DIR)/softfloat -I$(SPIKE_DIR)/build

SOURCES = xperia.cc xperiv.cc
OBJECTS = $(SOURCES:.cc=.o)
DEPS = $(OBJECTS:.o=.d)
TARGET = libxperi.so

# Link against required libraries
# LIBS = libriscv.a libdisasm.a libsoftfloat.a

LIBS=-lriscv -ldisasm -lsoftfloat
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -shared -o $@ $^ $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPS)

-include $(DEPS)

.PHONY: all clean
