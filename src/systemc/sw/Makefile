# RISCV_PATH

CC = $(RISCV_PATH)/bin/riscv64-unknown-elf-clang
OC = $(RISCV_PATH)/bin/riscv64-unknown-elf-objcopy
OD = $(RISCV_PATH)/bin/riscv64-unknown-elf-objdump
AS = $(RISCV_PATH)/bin/riscv64-unknown-elf-as

# C compilation directives
CFLAGS += -c
CFLAGS += -std=c11
CFLAGS += -Wall
CFLAGS += -Wno-unused-label
CFLAGS += -Os
CFLAGS += -march=rv64imafdcv_zicsr
CFLAGS += -fno-integrated-as

# Assembler
ASFLAGS += -c
ASFLAGS += -std=c11
ASFLAGS += -Os
ASFLAGS += -Wall
ASFLAGS += -march=rv64imafdcv_zicsr

# Linker directives.
LFLAGS += -Wall
LFLAGS += -Os
LFLAGS += -Wl,--no-relax
LFLAGS += -nostdlib
LFLAGS += -Tscript.ld

INCLUDE = -I.

C_SRC = ./main.c
S_SRC = ./start.S

OBJS += $(S_SRC:.S=.o)
OBJS += $(C_SRC:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

%.o: %.S
	$(CC) -x assembler-with-cpp $(ASFLAGS) $(INCLUDE) $< -o $@

%.elf: $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

%.lst: %.elf
	$(OD) --source --all-headers --demangle --line-numbers --wide $< > $@

.PHONY: all
all: main.lst main.elf

.PHONY: clean
clean:
	rm -f $(OBJS) *.o *.elf *.bin *.lst out.hex

