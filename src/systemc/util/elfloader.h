// Copyright (c) 2010-2017, The Regents of the University of California
// (Regents).  All Rights Reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the Regents nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
// SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
// OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
// HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
// MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

#include "fesvr/elf.h"
#include "fesvr/byteorder.h"
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>

#include <util/sparse_array.h>

template <typename T, int upper_width, int lower_width>
std::map<std::string, uint64_t> load_elf(const char* fn, util::sparse_array<T, upper_width, lower_width>* memif,
                                         uint64_t* entry) {
    int fd = open(fn, O_RDONLY);
    struct stat s;
    assert(fd != -1);
    if(fstat(fd, &s) < 0)
        abort();
    size_t size = s.st_size;

    char* buf = (char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(buf != MAP_FAILED);
    close(fd);

    assert(size >= sizeof(Elf64_Ehdr));
    const Elf64_Ehdr* eh64 = (const Elf64_Ehdr*)buf;
    assert(IS_ELF32(*eh64) || IS_ELF64(*eh64));
    assert(IS_ELFLE(*eh64) || IS_ELFBE(*eh64));
    assert(IS_ELF_EXEC(*eh64));
    assert(IS_ELF_RISCV(*eh64) || IS_ELF_EM_NONE(*eh64));
    assert(IS_ELF_VCURRENT(*eh64));

    std::vector<uint8_t> zeros;
    std::map<std::string, uint64_t> symbols;

#define LOAD_ELF(ehdr_t, phdr_t, shdr_t, sym_t, bswap)                                                                 \
    do {                                                                                                               \
        ehdr_t* eh = (ehdr_t*)buf;                                                                                     \
        phdr_t* ph = (phdr_t*)(buf + bswap(eh->e_phoff));                                                              \
        *entry = bswap(eh->e_entry);                                                                                   \
        assert(size >= bswap(eh->e_phoff) + bswap(eh->e_phnum) * sizeof(*ph));                                         \
        for(unsigned i = 0; i < bswap(eh->e_phnum); i++) {                                                             \
            if(bswap(ph[i].p_type) == PT_LOAD && bswap(ph[i].p_memsz)) {                                               \
                if(bswap(ph[i].p_filesz)) {                                                                            \
                    assert(size >= bswap(ph[i].p_offset) + bswap(ph[i].p_filesz));                                     \
                    memif->write(bswap(ph[i].p_paddr), (uint8_t*)buf + bswap(ph[i].p_offset), bswap(ph[i].p_filesz));  \
                }                                                                                                      \
                if(size_t pad = bswap(ph[i].p_memsz) - bswap(ph[i].p_filesz)) {                                        \
                    /*zeros.resize(pad);*/                                                                             \
                    /*memif->write(bswap(ph[i].p_paddr) + bswap(ph[i].p_filesz), zeros.data(), pad);*/                 \
                }                                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
        shdr_t* sh = (shdr_t*)(buf + bswap(eh->e_shoff));                                                              \
        assert(size >= bswap(eh->e_shoff) + bswap(eh->e_shnum) * sizeof(*sh));                                         \
        assert(bswap(eh->e_shstrndx) < bswap(eh->e_shnum));                                                            \
        assert(size >= bswap(sh[bswap(eh->e_shstrndx)].sh_offset) + bswap(sh[bswap(eh->e_shstrndx)].sh_size));         \
        char* shstrtab = buf + bswap(sh[bswap(eh->e_shstrndx)].sh_offset);                                             \
        unsigned strtabidx = 0, symtabidx = 0;                                                                         \
        for(unsigned i = 0; i < bswap(eh->e_shnum); i++) {                                                             \
            unsigned max_len = bswap(sh[bswap(eh->e_shstrndx)].sh_size) - bswap(sh[i].sh_name);                        \
            assert(bswap(sh[i].sh_name) < bswap(sh[bswap(eh->e_shstrndx)].sh_size));                                   \
            assert(strnlen(shstrtab + bswap(sh[i].sh_name), max_len) < max_len);                                       \
            if(bswap(sh[i].sh_type) & SHT_NOBITS)                                                                      \
                continue;                                                                                              \
            assert(size >= bswap(sh[i].sh_offset) + bswap(sh[i].sh_size));                                             \
            if(strcmp(shstrtab + bswap(sh[i].sh_name), ".strtab") == 0)                                                \
                strtabidx = i;                                                                                         \
            if(strcmp(shstrtab + bswap(sh[i].sh_name), ".symtab") == 0)                                                \
                symtabidx = i;                                                                                         \
        }                                                                                                              \
        if(strtabidx && symtabidx) {                                                                                   \
            char* strtab = buf + bswap(sh[strtabidx].sh_offset);                                                       \
            sym_t* sym = (sym_t*)(buf + bswap(sh[symtabidx].sh_offset));                                               \
            for(unsigned i = 0; i < bswap(sh[symtabidx].sh_size) / sizeof(sym_t); i++) {                               \
                unsigned max_len = bswap(sh[strtabidx].sh_size) - bswap(sym[i].st_name);                               \
                assert(bswap(sym[i].st_name) < bswap(sh[strtabidx].sh_size));                                          \
                assert(strnlen(strtab + bswap(sym[i].st_name), max_len) < max_len);                                    \
                symbols[strtab + bswap(sym[i].st_name)] = bswap(sym[i].st_value);                                      \
            }                                                                                                          \
        }                                                                                                              \
    } while(0)

    if(IS_ELFLE(*eh64)) {
        // memif->set_target_endianness(memif_endianness_little);
        if(IS_ELF32(*eh64)) {
            LOAD_ELF(Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Sym, from_le);
        } else {
            LOAD_ELF(Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr, Elf64_Sym, from_le);
        }
    } else {
#ifndef RISCV_ENABLE_DUAL_ENDIAN
        throw std::invalid_argument(
            "Specified ELF is big endian.  Configure with --enable-dual-endian to enable support");
#else
        memif->set_target_endianness(memif_endianness_big);
        if(IS_ELF32(*eh64))
            LOAD_ELF(Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Sym, from_be);
        else
            LOAD_ELF(Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr, Elf64_Sym, from_be);
#endif
    }

    munmap(buf, size);

    return symbols;
}
