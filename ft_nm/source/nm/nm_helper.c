/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_helper.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 15:01:18 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/21 13:24:06 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stddef.h>
#include <string.h>

#include "nm_elf.h"

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
 * @brief Check if a symbol is undefined.
 *
 * @param st_shndx ElfN_Sym member. Section header table index of the symbol.
 * @return int. 1 if true, 0 if false.
 */
int is_undef(uint64_t st_shndx) {
    return st_shndx == SHN_UNDEF;
}

/**
 * @brief Check if a symbol is external. Global, weak and unique symbols are
 * external, as well as undefined and common ones.
 *
 * @param st_shndx ElfN_Sym member. Section header table index of the symbol.
 * @param st_info ElfN_Sym member. Type and binding attributes of the symbol.
 * @return int. 1 if true, 0 if false.
 */
int is_external(uint64_t st_shndx, unsigned char st_info) {
    unsigned char binding = ELF64_ST_BIND(st_info);

    return binding == STB_GLOBAL || binding == STB_WEAK ||
           binding == STB_GNU_UNIQUE || st_shndx == SHN_UNDEF ||
           st_shndx == SHN_COMMON;
}

/**
 * @brief Deduce the symbol letter from the name of its section, as bfd's
 * coff_section_type() does. A name matches an entry of the table if it is equal
 * to it or continues with '.', '$' or a digit.
 *
 * @param name Name of the section.
 * @return int. Lowercase letter as displayed by nm, '?' if the name is unknown.
 */
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

/**
 * @brief Deduce the symbol letter from the flags and type of its section. Used
 * when the section name is not recognized.
 *
 * @param section_flag Section header member sh_flags.
 * @param section_type Section header member sh_type.
 * @param name Name of the section.
 * @return int. Letter as displayed by nm, '?' if it cannot be determined.
 */
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

/**
 * @brief Check if a section holds debugging information. A section loaded in
 * memory never does.
 *
 * @param section_flag Section header member sh_flags.
 * @param name Name of the section.
 * @return int. 1 if true, 0 if false.
 */
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

/**
 * @brief Locate the symbol tables (.symtab and .dynsym) and their string tables
 * among the section headers of a 32-bit file.
 *
 * @param shdr Section header table.
 * @param shnum Number of section headers.
 * @param sections Holds the section name string table, which is skipped.
 * @param symbols Filled with .symtab and its string table.
 * @param dynamic Filled with .dynsym and its string table.
 */
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

/**
 * @brief Same as get_symbol_sections_32, for a 64-bit file.
 *
 * @param shdr Section header table.
 * @param shnum Number of section headers.
 * @param sections Holds the section name string table, which is skipped.
 * @param symbols Filled with .symtab and its string table.
 * @param dynamic Filled with .dynsym and its string table.
 */
void get_symbol_sections_64(
    Elf64_Shdr* shdr, Elf64_Half shnum, elf_link_str_64_t* sections,
    elf_link_str_64_t* symbols, elf_link_str_64_t* dynamic) {
    for (Elf64_Half i = 0; i < shnum; ++i) {
        Elf64_Shdr* s = &shdr[i];

        if (s == sections->strings || s->sh_link == shnum) {
            continue;

        } else if (s->sh_type == SHT_SYMTAB) {
            /* in .symtab, read .strtab index */
            if (symbols) {
                (*symbols).elems = s;
                (*symbols).strings = &shdr[s->sh_link];
            }

        } else if (s->sh_type == SHT_DYNSYM) {
            /* in .dynsym, read .dynstr index */
            if (dynamic) {
                (*dynamic).elems = s;
                (*dynamic).strings = &shdr[s->sh_link];
            }
        }
    }
}
