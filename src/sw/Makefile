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
CFLAGS += -march=rv64imafdcvh_zba_zbb_zbs_zcb_zcmop_zicond_zfa_zfh_zkt_zicbop_zicbom_zicboz_zimop_zawrs_zifencei_zicsr_zihintpause_zihintntl_ziccif_ziccamoa_ziccrse_zihpm_za64rs_zic64b_zvl1024b_zvfh_zvfbfmin_zvbb_ss1p13_svbare_smaia_ssaia_smstateen_ssstateen_smrnmi_sscofpmf_sscounterenw_svade_ssccptr_sstvecd_sstvala_ssu64xl_svinval_svnapot_svpbmt_shcounterenw_shvstvecd_shtvala_shvstvala_shvsatpa_shgatpa_xsfvqmaccqoq_xsfvfnrclipxfqf_xsfvqdotq_xsfvfexpa_xsifivecdiscarddlone_xsifivecflushdlone_xsfcease0p1_xsfvfwmaccqqq_xsfvcp
CFLAGS += -fno-integrated-as
CFLAGS += -mcpu=sifive-x390
CFLAGS += -menable-experimental-extensions

# Assembler
ASFLAGS += -c
ASFLAGS += -std=c11
ASFLAGS += -Os
ASFLAGS += -Wall
ASFLAGS += -march=rv64imafdcvh_zba_zbb_zbs_zcb_zcmop_zicond_zfa_zfh_zkt_zicbop_zicbom_zicboz_zimop_zawrs_zifencei_zicsr_zihintpause_zihintntl_ziccif_ziccamoa_ziccrse_zihpm_za64rs_zic64b_zvl1024b_zvfh_zvfbfmin_zvbb_ss1p13_svbare_smaia_ssaia_smstateen_ssstateen_smrnmi_sscofpmf_sscounterenw_svade_ssccptr_sstvecd_sstvala_ssu64xl_svinval_svnapot_svpbmt_shcounterenw_shvstvecd_shtvala_shvstvala_shvsatpa_shgatpa_xsfvqmaccqoq_xsfvfnrclipxfqf_xsfvqdotq_xsfvfexpa_xsifivecdiscarddlone_xsifivecflushdlone_xsfcease0p1_xsfvfwmaccqqq_xsfvcp
ASFLAGS += -menable-experimental-extensions

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
	rm -f $(OBJS) *.o *.elf *.bin *.lst counters.hex *.vcd out.hex

