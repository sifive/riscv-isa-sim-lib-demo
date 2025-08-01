#define DECODE_MACRO_USAGE_LOGGED 0
#include <sys/syscall.h>
#include "extension.h"
#include "processor.h"
#include "decode.h"
#include "insn_macros.h"
#include "decode_macros.h"

#ifndef xlen
#define xlen 64
#endif

#define MATCH_PERI_V_ADD 0x0000002b
#define MASK_PERI_V_ADD  0xfc00707f

// Argument descriptor for disassembly
struct : public arg_t {
  std::string to_string(insn_t insn) const {
    return xpr_name[insn.rs1()];
  }
} xrs1;

// Add more argument descriptors for vector instruction
struct : public arg_t {
  std::string to_string(insn_t insn) const {
    return vr_name[insn.rd()];
  }
} xvd;

struct : public arg_t {
  std::string to_string(insn_t insn) const {
    return vr_name[insn.rs1()];
  }
} xvs1;

struct : public arg_t {
  std::string to_string(insn_t insn) const {
    return vr_name[insn.rs2()];
  }
} xvs2;

static reg_t peri_v_add_impl(processor_t* p, insn_t insn, reg_t pc)
{
  WRITE_RD(sext_xlen(RS1 + RS2));
  return pc + 4;
}

class xperiv_t : public extension_t
{
public:
  const char* name() const override { return "xperiv"; }
  
  std::vector<insn_desc_t> get_instructions(const processor_t &) override {
    fprintf(stderr, "xperiv get_instructions called!\n");

    std::vector<insn_desc_t> insns;
    
    insns.push_back({MATCH_PERI_V_ADD, MASK_PERI_V_ADD, 
                     peri_v_add_impl, peri_v_add_impl, peri_v_add_impl, peri_v_add_impl,
                     peri_v_add_impl, peri_v_add_impl, peri_v_add_impl, peri_v_add_impl});
    
    return insns;
  }
  
  std::vector<disasm_insn_t*> get_disasms(const processor_t *) override {
    std::vector<disasm_insn_t*> insns;
    
    insns.push_back(new disasm_insn_t("peri.v.add", MATCH_PERI_V_ADD, MASK_PERI_V_ADD, {&xvd, &xvs1, &xvs2}));
    
    return insns;
  }
};

REGISTER_EXTENSION(periv, []() { fprintf(stderr, "xperiv factory called!\n"); return new xperiv_t; })
