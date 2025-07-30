#include "extension.h"
#include "processor.h"
#include "decode.h"
#include "insn_macros.h"
#include "decode_macros.h"


#define MATCH_PERI_A_ADD 0x0000000b
#define MASK_PERI_A_ADD  0xfe00707f 

// Argument descriptor for disassembly
struct : public arg_t {
  std::string to_string(insn_t insn) const {
    return xpr_name[insn.rs1()];
  }
} xrs1;

static reg_t peri_a_add_impl(processor_t* p, insn_t insn, reg_t pc)
{
  #include "peri_a_add_impl.h"
  return pc + 4;
}

class xperia_t : public extension_t
{
public:
  const char* name() const override { return "xperia"; }
  
  std::vector<insn_desc_t> get_instructions(const processor_t &) override {
    fprintf(stderr, "xperia get_instructions called!\n");

    std::vector<insn_desc_t> insns;
    
    insns.push_back({MATCH_PERI_A_ADD, MASK_PERI_A_ADD, 
                     peri_a_add_impl, peri_a_add_impl, peri_a_add_impl, peri_a_add_impl,
                     peri_a_add_impl, peri_a_add_impl, peri_a_add_impl, peri_a_add_impl});
    
    return insns;
  }
  
  std::vector<disasm_insn_t*> get_disasms(const processor_t *) override {
    std::vector<disasm_insn_t*> insns;
    
    insns.push_back(new disasm_insn_t("peri.a.add", MATCH_PERI_A_ADD, MASK_PERI_A_ADD, {&xrs1}));
    
    return insns;
  }
};

REGISTER_EXTENSION(xperia, []() { fprintf(stderr, "xperia factory called!\n"); return new xperia_t; })
