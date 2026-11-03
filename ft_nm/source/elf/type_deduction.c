/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   type_deduction.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 15:01:18 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/03 17:10:00 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file type_deduction.c
 * @brief Deduction of the letter nm displays for a symbol, and of its
 * undefined and external attributes.
 */

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "nm_elf.h"

extern int arch_global;

/**
 * @brief Check if a symbol is undefined.
 *
 * @param[in] st_shndx ElfN_Sym::st_shndx. Section header table index of the
 * symbol.
 * @retval 1 The symbol is undefined (@c SHN_UNDEF).
 * @retval 0 It is not.
 */
int is_undef(ElfN_Section st_shndx) {
    return st_shndx == SHN_UNDEF;
}

/**
 * @brief Check if a symbol is external. Global, weak and unique symbols are
 * external, as well as undefined and common ones.
 *
 * @param[in] st_shndx ElfN_Sym::st_shndx. Section header table index of the
 * symbol.
 * @param[in] st_info ElfN_Sym::st_info. Type and binding attributes of the
 * symbol.
 * @retval 1 The symbol is external.
 * @retval 0 It is local.
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
 * @param[in] section_flag ElfN_Shdr::sh_flags of the section.
 * @param[in] name Name of the section.
 * @retval 1 The section holds debugging information.
 * @retval 0 It does not.
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
 * @brief Deduce the lowercase symbol letter from the flags and type of its
 * section.
 *
 * @param[in] section_flag ElfN_Shdr::sh_flags of the section.
 * @param[in] section_type ElfN_Shdr::sh_type of the section.
 * @param[in] name Name of the section.
 * @return Letter as displayed by nm for a local symbol: @c 't', @c 'r',
 * @c 'd', @c 'b', @c 'N' or @c 'n', @c '?' if it cannot be determined.
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
 * Special indexes and bindings are handled first (@c 'C', @c 'U', @c 'w',
 * @c 'v', @c 'i', @c 'W', @c 'V', @c 'u', @c 'a'), then the letter is
 * deduced from the section of the symbol (see section_type_from_flags()).
 *
 * @param[in] addr Address of the start of the ELF file.
 * @param[in] size Size of the file in bytes.
 * @param[in] symbol_tab Section name string table of the file.
 * @param[in] shnum Number of section headers of the file.
 * @param[in] s Symbol to deduce the letter for.
 * @return Letter as displayed by nm, @c '?' if it cannot be determined.
 * @pre #arch_global is set.
 */
int get_symbol_type(void* addr, size_t size, const elf_link_str_t* symbol_tab,
                    size_t shnum, const ElfN_Sym* s) {
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
            elf_str_at(addr, size, &symbol_tab->strings, sym_section.sh_name);

        c = section_type_from_flags(sym_section.sh_flags, sym_section.sh_type,
                                    name);
    }

    if (binding == STB_GLOBAL && islower(c)) {
        c = toupper(c);
    }
    return c;
}