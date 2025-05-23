# RISC-V ISA Simulator as a Library Demo

This project demonstrates how to use [RISC-V ISA Simulator](https://github.com/riscv-software-src/riscv-isa-sim) as a library, and integrate it with an external simulator. 
In this demonstration, external simulator is a simple memory simulator. In our use case, all memory operations (loads, stores, and instruction fetches) are handled by the external simulator, without any of riscv-isa-sim's internal memories.

## Important Notes

### Hardcoded Parameters
Several parameters are hardcoded in `src/main.cc`:
- Start PC: `0x20000000`
- Memory size: 1GB (1024 * 1024 * 1024 bytes)
- ISA string: "rv64imafdcv"
- Privilege levels: "MSU" (Machine, Supervisor, and User)
- CPU runs sw `src/sw/main.elf`. Check `src/sw/README.md` for more details. 

### Program Termination
The program will terminate with either:
- "PASS" message: When the test program writes `0x5555` to the finisher address
- "FAIL with status {x}" message: When any other non-zero value is written to the finisher address

In both cases, when detected write to finisher address, `memory_simulator` will print the message, call `assert(0)` and terminate.

## Steps to Build the Project

### Step 1: Clone RISC-V ISA Simulator Repository
Clone the RISC-V ISA Simulator repository and checkout the required version:

```bash
git clone https://github.com/riscv-software-src/riscv-isa-sim.git
```

### Step 2: Set Environment Variable
Set the environment variable to point to your RISC-V ISA Simulator repository:

```bash
export SPIKE_SOURCE_DIR=/path/to/riscv-isa-sim
```

### Step 3: Build the Demo
There are several build targets in the Makefile. 
First you need to compile the source files. You can do it with `compile_only` target:
```bash
make compile_only
```
Then you can link the object files with the Spike libraries dynamically. You can do it with `link_demo_dynamic` target:
```bash
make link_demo_dynamic
```
Those two steps can be combined into one with `demo` target:
```bash
make demo
```

### Library Linking Details

The project uses dynamic linking to connect with the Spike libraries:

```makefile
link_demo_dynamic: $(OBJS)
    $(CXX) -std=c++17 $(OBJS) -o demo \
        -L../lib \
        -Wl,-rpath,../lib/ \
        -Wl,--no-as-needed \
        -lriscv \
        -lsoftfloat \
        -latomic
```

Key components:
- `-L../lib`: Specifies the library search path
- `-Wl,-rpath,../lib/`: Embeds the runtime library path in the executable so it can find libraries at runtime
- `-Wl,--no-as-needed`: Ensures all specified libraries are linked, even if they don't resolve any symbols
- `-lriscv`, `-lsoftfloat`, `-latomic`: Required libraries for the RISC-V simulator

If you want to compile `riscv-isa-sim` from scratch, you can use the `build_spike` target:
```bash
make build_spike
```

Note: We are forcing C++17 standard in the Makefile and therefore some Warnings are expected.

Output of the build process is `demo` executable in `src` folder.

### Optional: Building with Memory Operation Hooks
To enable memory operation observation/debugging, you can build with observability hooks enabled:

```bash
make USE_HOOKS=1
```

This will:
- Include the `hooks.h` header automatically in the build
- Enable printing of memory operations:
  - Load operations: address, data, and length
  - Fetch operations: address, instruction, and length

### (Optional) Step 4: compile sw

Our small example software is in `src/sw` folder. Elf filename is hardcoded in `main.cc`

```bash
cd src/sw
# follow instructions in sw/README.md
make
```

This step is optional because we have added a precompiled elf file in `src/sw/main.elf`

### Step 5: Run the Demo
Run the compiled demo:
```bash
./src/demo
```

## Adding External Simulator API

### riscv-isa-sim cfg_t class
class cfg_t has a new field: `external_simulator`. That field is a pointer to an object that implements `abstract_sim_if_t` interface. 

```cpp
class cfg_t {
public:
    // ...
    std::optional<abstract_sim_if_t*> external_simulator;
    // ...
};
```
Usage is in `main.cc`

```cpp
cfg.external_simulator = &ext_sim;
```

### demo_core
A custom processor implementation that inherits from riscv-isa-sim's `simif_t`.
Since recently, `bus_t` has a fallback parameter, in case no device is found on the bus, request is forwarded to the fallback device. In our case it is the external simulator. 
Example is in constructor of `demo_core`
```cpp
if (cfg->external_simulator.has_value()) {
    auto* ext_sim = cfg->external_simulator.value();
    bus_fallback = new external_sim_device_t(ext_sim);
}

if (bus_fallback != nullptr) {
    bus = std::make_unique<bus_t>(bus_fallback);
} else {
    bus = std::make_unique<bus_t>();
}
```
### riscv-isa-sim abstract_sim_if_t
This is the interface that external simulator must implement. It has two methods: `load` and `store`.
```cpp
class abstract_sim_if_t {
public:
    virtual bool load(reg_t addr, size_t len, uint8_t* bytes) = 0;
    virtual bool store(reg_t addr, size_t len, const uint8_t* bytes) = 0;
};
```
We use this class as API to integrate with `riscv-isa-sim`.

### Components used in example

### Memory Simulator
We have a base class and two implementation approaches:
- `memory_simulator`: Base class implementing core memory operations:
  - Read/write operations
  - ELF file loading
  - ROM content initialization
  - Sparse array-based memory management

Two different ways to integrate with riscv-isa-sim:
1. `memory_simulator_wrapper`: Inherits from both `memory_simulator` and `abstract_sim_if_t`
2. `memory_sim_bridge`: Bridge pattern implementation that wraps a `memory_simulator` instance

#### 1. Memory Simulator Wrapper (`memory_simulator_wrapper`)
Direct inheritance approach that combines `memory_simulator` and `abstract_sim_if_t`:

```cpp
class memory_simulator_wrapper : public memory_simulator, public abstract_sim_if_t {
    public:
        memory_simulator_wrapper(uint64_t size);
        ~memory_simulator_wrapper() override;
        bool load(reg_t addr, size_t len, uint8_t* bytes) override;
        bool store(reg_t addr, size_t len, const uint8_t* bytes) override;
};
```

It is typically used technique to make a wrapper class when integration with some other library is needed. 

#### 2. Bridge (`memory_sim_bridge`)
Wrapper class that uses composition to integrate the memory simulator with riscv-isa-sim:

```cpp
class memory_sim_bridge {
    public:
        memory_sim_bridge(memory_simulator* sim);
        bool load(reg_t addr, size_t len, uint8_t* bytes);
        bool store(reg_t addr, size_t len, const uint8_t* bytes);
    private:
        memory_simulator* sim;
};
```

This approach uses composition, where the `memory_sim_bridge` class holds a pointer to a `memory_simulator` instance and forwards memory operations to it.

Note: In both approaches we need to define `load` and `store` methods from `abstract_sim_if_t` interface.

### Main Program Flow

The main program (`src/main.cc`) demonstrates how to set up and run the RISC-V simulator with an external memory simulator. Here's a detailed breakdown:

1. **Configuration Setup**
   ```cpp
   cfg_t cfg;
   ```
   Creates a configuration object that defines:
   - ISA string with supported extensions
   - Privilege levels (MSU)
   - Starting PC address (0x20000000)
   - setting external simulator field

2. **Memory Simulator Initialization**
   ```cpp
   memory_simulator_wrapper ext_sim(1024 * 1024 * 1024, START_PC);
   ```
In case you want to use `memory_sim_bridge` instead of `memory_simulator_wrapper`, you need to define `#define USE_BRIDGE 1` before `main()`.

3. **External Simulator Integration**
   ```cpp
   cfg.external_simulator = &ext_sim;
   ```

4. **Processor Creation**
   ```cpp
   demo_core demo_riscv_core(&cfg);
   ```

5. **Runtime Execution**
   ```cpp
   demo_riscv_core.reset();
   while (1)
       demo_riscv_core.step(5000);
   ```


The program terminates when the test software writes to the finisher address:
- 0x5555: Successful completion ("PASS")
- Any other non-zero value: Error condition ("FAIL with status {x}")




