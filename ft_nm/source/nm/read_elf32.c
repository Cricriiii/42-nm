/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_elf32.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 15:38:27 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 17:27:32 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>

#include "nm_elf.h"
#include "resource_collector.h"
#include "xalloc.h"

typedef struct nm_args nm_args_t;

extern nm_args_t options;

static int get_symbol_type(const Elf32_Shdr* shdr, Elf32_Half shnum,
                           const char* shstrtab, const Elf32_Sym* s) {
    unsigned char binding = ELF32_ST_BIND(s->st_info);
    unsigned char type = ELF32_ST_TYPE(s->st_info);

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
        const Elf32_Shdr* section = &shdr[s->st_shndx];
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

nm_sym_out_t** read_elf32(void* addr) {
    nm_sym_out_t** ret_array = nullptr;

    /* sections start */
    Elf32_Shdr* shdr_start = shdr32(addr + ehdr32(addr)->e_shoff);

    /* section header array */
    Elf32_Half shnum = ehdr32(addr)->e_shnum;

    elf_link_str_32_t sections = {.strings =
                                      &shdr_start[ehdr32(addr)->e_shstrndx]};
    elf_link_str_32_t symbols = {};
    elf_link_str_32_t dynamic = {};
    get_symbol_sections_32(shdr_start, shnum, &sections, &symbols, &dynamic);

    if (symbols.elems) {
        size_t n_sym = symbols.elems->sh_size / symbols.elems->sh_entsize;

        ret_array = xcalloc(n_sym + 1, sizeof(nm_sym_out_t*));
        store_ptr_resource(ret_array);

        Elf32_Sym* symbol_tab = sym32(addr + symbols.elems->sh_offset);
        char* string_tab = addr + symbols.strings->sh_offset;

        size_t cur_sym_idx = 0;

        for (size_t i = 0; i < n_sym; ++i) {
            Elf32_Sym* s = &symbol_tab[i];

            nm_sym_out_t* nm_sym = xcalloc(1, sizeof(nm_sym_out_t));
            store_ptr_resource(nm_sym);
            nm_sym->name = string_tab + s->st_name;
            nm_sym->value = s->st_value;
            nm_sym->type = get_symbol_type(shdr_start, shnum,
                                           addr + symbols.elems->sh_offset, s);
            nm_sym->external = is_external(s->st_shndx, s->st_info);
            nm_sym->undef = is_undef(s->st_shndx);
            nm_sym->arch = ARCH32;

            ret_array[cur_sym_idx++] = nm_sym;
        }
    }

    return ret_array;
}
