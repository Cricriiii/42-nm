/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_elfN.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 15:38:27 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/30 12:01:07 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>

#include "nm_elf.h"
#include "resource_collector.h"
#include "xalloc.h"

typedef struct nm_args nm_args_t;

extern int arch_global;

extern nm_args_t options;

/**
 * @brief Interpret the 16-byte identification field (e_ident) of the ELF
 * header.
 *
 * @param addr Address of the first byte of the mapped memory.
 * @return elf_ei_t. Structure holding the identification fields.
 */
elf_ei_t read_elf_ident(void* addr) {
    char* ident = (char*)addr;
    return (elf_ei_t){
        .mag0 = ident[0],
        .mag1 = ident[1],
        .mag2 = ident[2],
        .mag3 = ident[3],
        .class = ident[4],
        .data = ident[5],
        .version = ident[6],
        .osabi = ident[7],
        .abiversion = ident[8],
    };
}

/**
 * @brief Get the symbol sections object
 *
 * @param addr
 * @param sections
 * @param symbols
 * @param dynamic
 */
static void get_symbol_sections(void* addr, elf_link_str_t* sections,
                                elf_link_str_t* symbols,
                                elf_link_str_t* dynamic) {
    ElfN_Ehdr ehdr = {};
    Ehdr_to_generic(addr, &ehdr);

    size_t shdr_size =
        arch_global == ARCH64 ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr);

    for (ElfN_Half i = 0; i < ehdr.e_shnum; ++i) {
        ElfN_Shdr s = {};
        Shdr_to_generic(addr, ehdr.e_shoff + shdr_size * i, &s);

        if (i == ehdr.e_shstrndx) {
            /* get e_shstrndx from ehdr */
            if (sections) {
                (*sections).elems = (ElfN_Shdr){};
                (*sections).strings = s;
                (*sections).is = 1;
            }

        } else if (s.sh_type == SHT_SYMTAB) {
            /* in .symtab, read .strtab index */
            if (symbols) {
                (*symbols).elems = s;
                Shdr_to_generic(addr, ehdr.e_shoff + shdr_size * s.sh_link,
                                &(*symbols).strings);
                (*symbols).is = 1;
            }

        } else if (s.sh_type == SHT_DYNSYM) {
            /* in .dynsym, read .dynstr index */
            if (dynamic) {
                (*dynamic).elems = s;
                Shdr_to_generic(addr, ehdr.e_shoff + shdr_size * s.sh_link,
                                &(*dynamic).strings);
                (*dynamic).is = 1;
            }
        }
    }
}

/**
 * @brief Read the symbol table of a 64-bit ELF file. The returned array and its
 * elements are allocated and registered in the resource collector. The first
 * element is the null symbol, and the array is null terminated.
 *
 * @param addr Address of the mapped memory region.
 * @return elf_sym_print_t**. Array of symbols, nullptr if there is no .symtab.
 */
elf_sym_print_t** read_elfN(void* addr) {
    elf_sym_print_t** ret_array = nullptr;

    ElfN_Ehdr ehdr = {};
    if (Ehdr_to_generic(addr, &ehdr) == -1) {
        ;  // treat error
    }

    elf_link_str_t sections = {};  // encore a faire
    elf_link_str_t symbols = {};
    elf_link_str_t dynamic = {};
    get_symbol_sections(addr, &sections, &symbols, &dynamic);

    if (symbols.is) {
        size_t n_sym = symbols.elems.sh_size / symbols.elems.sh_entsize;

        ret_array = xcalloc(n_sym + 1, sizeof(elf_sym_print_t*));
        store_ptr_resource(ret_array);

        size_t cur_sym_idx = 0;

        size_t shdr_size, sym_size;

        if (arch_global == ARCH64) {
            shdr_size = sizeof(Elf64_Shdr);
            sym_size = sizeof(Elf64_Sym);
        } else {
            shdr_size = sizeof(Elf32_Shdr);
            sym_size = sizeof(Elf32_Sym);
        }

        /* Convert each raw symbol into its displayable form */
        for (size_t i = 0; i < n_sym; ++i) {
            /* obtain symbol data */
            ElfN_Sym s = {};
            Sym_to_generic(addr, symbols.elems.sh_offset + sym_size * i, &s);

            /* create output symbol data */
            elf_sym_print_t* nm_sym = xcalloc(1, sizeof(elf_sym_print_t));
            store_ptr_resource(nm_sym);

            /* symbol architecture */
            nm_sym->arch = arch_global;

            /* symbol section */
            nm_sym->section = ((arch_global == ARCH64
                                    ? ELF64_ST_TYPE(s.st_info)
                                    : ELF32_ST_TYPE(s.st_info)) == STT_SECTION);

            if (nm_sym->section) {
                ElfN_Shdr sym_section = {};
                Shdr_to_generic(addr, ehdr.e_shoff + shdr_size * s.st_shndx,
                                &sym_section);
                nm_sym->name =
                    addr + sections.strings.sh_offset + sym_section.sh_name;
            } else {
                nm_sym->name = addr + symbols.strings.sh_offset + s.st_name;
                ;
            }

            /* symbol value */
            nm_sym->value = s.st_value;

            /* symbol type */
            nm_sym->type = get_symbol_type(addr, &sections, ehdr.e_shnum, &s);

            /* symbol external */
            nm_sym->external = is_external(s.st_shndx, s.st_info);

            /* symbol undef*/
            nm_sym->undef = is_undef(s.st_shndx);

            /* store symbol pointer in array */
            ret_array[cur_sym_idx++] = nm_sym;
        }
    }

    return ret_array;
}