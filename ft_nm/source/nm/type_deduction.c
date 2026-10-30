/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   type_deduction.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 15:01:18 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/30 18:07:43 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stddef.h>
#include <string.h>

#include "nm_elf.h"

extern int arch_global;

/**
 * @brief Check if a symbol is undefined.
 *
 * @param st_shndx ElfN_Sym member. Section header table index of the symbol.
 * @return int. 1 if true, 0 if false.
 */
int is_undef(ElfN_Section st_shndx) {
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
int is_external(ElfN_Section st_shndx, unsigned char st_info) {
    unsigned char binding = ELF64_ST_BIND(st_info);

    return binding == STB_GLOBAL || binding == STB_WEAK ||
           binding == STB_GNU_UNIQUE || st_shndx == SHN_UNDEF ||
           st_shndx == SHN_COMMON;
}

/**
 * @brief Check if a section holds debugging information. A section loaded in
 * memory never does.
 *
 * @param section_flag Section header member sh_flags.
 * @param name Name of the section.
 * @return int. 1 if true, 0 if false.
 */
int is_debug_section(ElfN_Xword section_flag, const char* name) {
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
 * @brief Deduce the symbol letter from the name of its section, as bfd's
 * coff_section_type() does. A name matches an entry of the table if it is equal
 * to it or continues with '.', '$' or a digit.
 *
 * @param name Name of the section.
 * @return int. Lowercase letter as displayed by nm, '?' if the name is unknown.
 */
// int section_type_from_name(const char* name) {
//     static const struct {
//         const char* prefix;
//         int type;
//     } table[] = {
//         {".bss", 'b'},     {".code", 't'},  {".data", 'd'},
//         {"*DEBUG*", 'N'},  {".debug", 'N'}, {".drectve", 'i'},
//         {".edata", 'e'},   {".fini", 't'},  {".idata", 'i'},
//         {".init", 't'},    {".pdata", 'p'}, {".rdata", 'r'},
//         {".rodata", 'r'},  {".sbss", 's'},  {".scommon", 'c'},
//         {".sdata", 'g'},   {".text", 't'},  {"vars", 'd'},
//         {"zerovars", 'b'},
//     };

//     for (size_t i = 0; i < sizeof(table) / sizeof(*table); ++i) {
//         size_t len = strlen(table[i].prefix);

//         /* ".text" and ".text.foo" match, ".textfoo" does not */
//         if (strncmp(name, table[i].prefix, len) == 0 &&
//             (name[len] == '\0' || strchr(".$0123456789", name[len]))) {
//             return table[i].type;
//         }
//     }
//     return '?';
// }

/**
 * @brief Deduce the symbol letter from the flags and type of its section. Used
 * when the section name is not recognized.
 *
 * @param section_flag Section header member sh_flags.
 * @param section_type Section header member sh_type.
 * @param name Name of the section.
 * @return int. Letter as displayed by nm, '?' if it cannot be determined.
 */
static int section_type_from_flags(ElfN_Xword section_flag,
                                   ElfN_Word section_type, const char* name) {
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
 * @brief Deduce the letter displayed by nm for a symbol. The letter is
 * uppercase for global symbols and lowercase for local ones.
 *
 * @param addr
 * @param symbol_tab
 * @param shnum
 * @param s
 * @return int
 */
int get_symbol_type(void* addr, const elf_link_str_t* symbol_tab,
                    ElfN_Half shnum, const ElfN_Sym* s) {
    /* type and binding */
    unsigned char binding, type;
    size_t shdr_size;
    if (arch_global == ARCH64) {
        binding = ELF64_ST_BIND(s->st_info);
        type = ELF64_ST_TYPE(s->st_info);
        shdr_size = sizeof(Elf64_Shdr);
    } else {
        binding = ELF32_ST_BIND(s->st_info);
        type = ELF32_ST_TYPE(s->st_info);
        shdr_size = sizeof(Elf32_Shdr);
    }

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
        ElfN_Ehdr ehdr = {};
        Ehdr_to_generic(addr, &ehdr);

        ElfN_Shdr sym_section = {};
        Shdr_to_generic(addr, ehdr.e_shoff + shdr_size * s->st_shndx,
                        &sym_section);

        const char* name =
            addr + symbol_tab->strings.sh_offset + sym_section.sh_name;

        // c = section_type_from_name(name);
        // if (c == '?') {
            c = section_type_from_flags(sym_section.sh_flags,
                                        sym_section.sh_type, name);
        // }
    }

    if (binding == STB_GLOBAL && c >= 'a' && c <= 'z') {
        c -= 'a' - 'A';
    }
    return c;
}