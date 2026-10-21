/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_elf64.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 15:38:27 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/21 13:38:58 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>

#include "nm_elf.h"
#include "resource_collector.h"
#include "xalloc.h"

typedef struct nm_args nm_args_t;

extern nm_args_t options;

/**
 * @brief Deduce the letter displayed by nm for a symbol. The letter is
 * uppercase for global symbols and lowercase for local ones.
 *
 * @param shdr Section header table.
 * @param shnum Number of section headers.
 * @param shstrtab Section name string table.
 * @param s Symbol to be typed.
 * @return int. Letter as displayed by nm.
 */
static int get_symbol_type(const Elf64_Shdr* shdr, Elf64_Half shnum,
                           const char* shstrtab, const Elf64_Sym* s) {
    unsigned char binding = ELF64_ST_BIND(s->st_info);
    unsigned char type = ELF64_ST_TYPE(s->st_info);

    if (s->st_shndx == SHN_COMMON) {
        return 'C';
    }
    if (s->st_shndx == SHN_UNDEF) {
        if (binding == STB_WEAK) {
            return type == STT_OBJECT ? 'v' : 'w';
        }
        return 'U';
    }
    if (type == STT_GNU_IFUNC) {
        return 'i';
    }
    if (binding == STB_WEAK) {
        return type == STT_OBJECT ? 'V' : 'W';
    }
    if (binding == STB_GNU_UNIQUE) {
        return 'u';
    }
    if (binding != STB_LOCAL && binding != STB_GLOBAL) {
        return '?';
    }

    int c;

    /* bfd treats any index it can't resolve as an absolute symbol */
    if (s->st_shndx == SHN_ABS || s->st_shndx >= shnum) {
        c = 'a';
    } else {
        const Elf64_Shdr* section = &shdr[s->st_shndx];
        const char* name = shstrtab + section->sh_name;

        c = section_type_from_name(name);
        if (c == '?') {
            c = section_type_from_flags(section->sh_flags, section->sh_type,
                                        name);
        }
    }

    if (binding == STB_GLOBAL && c >= 'a' && c <= 'z') {
        c -= 'a' - 'A';
    }
    return c;
}

/**
 * @brief Read the symbol table of a 64-bit ELF file. The returned array and its
 * elements are allocated and registered in the resource collector. The first
 * element is the null symbol, and the array is null terminated.
 *
 * @param addr Address of the mapped memory region.
 * @return nm_sym_out_t**. Array of symbols, nullptr if there is no .symtab.
 */
nm_sym_out_t** read_elf64(void* addr) {
    nm_sym_out_t** ret_array = nullptr;

    Elf64_Shdr* shdr_start = shdr64(addr + ehdr64(addr)->e_shoff);

    Elf64_Half shnum = ehdr64(addr)->e_shnum;

    elf_link_str_64_t sections = {.strings =
                                      &shdr_start[ehdr64(addr)->e_shstrndx]};
    elf_link_str_64_t symbols = {};
    elf_link_str_64_t dynamic = {};
    get_symbol_sections_64(shdr_start, shnum, &sections, &symbols, &dynamic);

    if (symbols.elems) {
        size_t n_sym = symbols.elems->sh_size / symbols.elems->sh_entsize;

        ret_array = xcalloc(n_sym + 1, sizeof(nm_sym_out_t*));
        store_ptr_resource(ret_array);

        Elf64_Sym* symbol_tab = sym64(addr + symbols.elems->sh_offset);
        char* string_tab = addr + symbols.strings->sh_offset;

        size_t cur_sym_idx = 0;

        /* Convert each raw symbol into its displayable form */
        for (size_t i = 0; i < n_sym; ++i) {
            Elf64_Sym* s = &symbol_tab[i];

            nm_sym_out_t* nm_sym = xcalloc(1, sizeof(nm_sym_out_t));
            store_ptr_resource(nm_sym);
            nm_sym->name = string_tab + s->st_name;
            nm_sym->value = s->st_value;
            nm_sym->type = get_symbol_type(
                shdr_start, shnum, addr + symbols.strings->sh_offset, s);
            nm_sym->external = is_external(s->st_shndx, s->st_info);
            nm_sym->undef = is_undef(s->st_shndx);
            nm_sym->arch = ARCH64;

            ret_array[cur_sym_idx++] = nm_sym;
        }
    }

    return ret_array;
}
