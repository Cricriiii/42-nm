/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_helper.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 15:01:18 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 16:33:13 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stddef.h>
#include <string.h>

#include "nm_elf.h"

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

int is_undef(uint64_t st_shndx) {
    return st_shndx == SHN_UNDEF;
}

int is_external(uint64_t st_shndx, unsigned char st_info) {
    unsigned char binding = ELF64_ST_BIND(st_info);

    return binding == STB_GLOBAL || binding == STB_WEAK ||
           binding == STB_GNU_UNIQUE || st_shndx == SHN_UNDEF ||
           st_shndx == SHN_COMMON;
}

/* section name -> letter, as bfd's coff_section_type() */
int section_type_from_name(const char* name) {
    static const struct {
        const char* prefix;
        int type;
    } table[] = {
        {".bss", 'b'},     {".code", 't'},  {".data", 'd'},
        {"*DEBUG*", 'N'},  {".debug", 'N'}, {".drectve", 'i'},
        {".edata", 'e'},   {".fini", 't'},  {".idata", 'i'},
        {".init", 't'},    {".pdata", 'p'}, {".rdata", 'r'},
        {".rodata", 'r'},  {".sbss", 's'},  {".scommon", 'c'},
        {".sdata", 'g'},   {".text", 't'},  {"vars", 'd'},
        {"zerovars", 'b'},
    };

    for (size_t i = 0; i < sizeof(table) / sizeof(*table); ++i) {
        size_t len = strlen(table[i].prefix);

        /* ".text" and ".text.foo" match, ".textfoo" does not */
        if (strncmp(name, table[i].prefix, len) == 0 &&
            (name[len] == '\0' || strchr(".$0123456789", name[len]))) {
            return table[i].type;
        }
    }
    return '?';
}

int section_type_from_flags(uint64_t section_flag, uint64_t section_type,
                            const char* name) {
    bool has_contents = section_type != SHT_NOBITS;
    bool readonly = !(section_flag & SHF_WRITE);

    if (section_flag & SHF_EXECINSTR) {
        return 't';
    }
    if ((section_flag & SHF_ALLOC) && has_contents) {
        return readonly ? 'r' : 'd';
    }
    if (!has_contents) {
        return 'b';
    }
    if (is_debug_section(section_flag, name)) {
        return 'N';
    }
    if (readonly) {
        return 'n';
    }
    return '?';
}

int is_debug_section(uint64_t section_flag, const char* name) {
    if (section_flag & SHF_ALLOC) {
        return false;
    }
    return strncmp(name, ".debug", 6) == 0 ||
           strncmp(name, ".zdebug", 7) == 0 ||
           strncmp(name, ".gnu.debuglto_.debug_", 21) == 0 ||
           strncmp(name, ".gnu.linkonce.wi.", 17) == 0 ||
           strncmp(name, ".line", 5) == 0 || strncmp(name, ".stab", 5) == 0 ||
           strcmp(name, ".gdb_index") == 0;
}

void get_symbol_sections_32(Elf32_Shdr* shdr, Elf32_Half shnum,
                            elf_link_str_32_t* sections,
                            elf_link_str_32_t* symbols,
                            elf_link_str_32_t* dynamic) {
    for (Elf32_Half i = 0; i < shnum; ++i) {
        Elf32_Shdr* s = &shdr[i];

        if (s == sections->strings || s->sh_link == shnum) {
            continue;

        } else if (s->sh_type == SHT_SYMTAB) {
            /* in .symtab, read .strtab index */
            (*symbols).elems = s;
            (*symbols).strings = &shdr[s->sh_link];

        } else if (s->sh_type == SHT_DYNSYM) {
            /* in .dynsym, read .dynstr index */
            (*dynamic).elems = s;
            (*dynamic).strings = &shdr[s->sh_link];
        }
    }
}

void get_symbol_sections_64(Elf64_Shdr* shdr, Elf64_Half shnum,
                            elf_link_str_64_t* sections,
                            elf_link_str_64_t* symbols,
                            elf_link_str_64_t* dynamic) {
    for (Elf64_Half i = 0; i < shnum; ++i) {
        Elf64_Shdr* s = &shdr[i];

        if (s == sections->strings || s->sh_link == shnum) {
            continue;

        } else if (s->sh_type == SHT_SYMTAB) {
            /* in .symtab, read .strtab index */
            (*symbols).elems = s;
            (*symbols).strings = &shdr[s->sh_link];

        } else if (s->sh_type == SHT_DYNSYM) {
            /* in .dynsym, read .dynstr index */
            (*dynamic).elems = s;
            (*dynamic).strings = &shdr[s->sh_link];
        }
    }
}
