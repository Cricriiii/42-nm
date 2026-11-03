/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_elfN.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 15:38:27 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/02 17:32:52 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file read_elfN.c
 * @brief Extraction of the .symtab symbols of an ELF file, 32 or 64 bits,
 * into their displayable form.
 */

#include <stdio.h>
#include <string.h>

#include "nm_elf.h"
#include "resource_collector.h"
#include "xalloc.h"

extern int arch_global;

/**
 * @brief String displayed instead of a name that can't be read safely.
 */
#define ELF_CORRUPT_NAME "<corrupt>"

/**
 * @brief Tell whether the contents of a section lie entirely inside the file.
 * Written to never overflow, whatever the values of a corrupted header.
 *
 * @param[in] size Size of the file in bytes.
 * @param[in] s Section header.
 * @retval 1 The whole section is inside the file.
 * @retval 0 It is not.
 */
static int section_in_file(size_t size, const ElfN_Shdr* s) {
    return s->sh_offset <= size && s->sh_size <= size - s->sh_offset;
}

/**
 * @brief Get a string of a string table, only if it is entirely readable: the
 * table must be inside the file, the offset inside the table, and the string
 * must be null terminated before the end of the table.
 *
 * @param[in] addr Address of the start of the ELF file.
 * @param[in] size Size of the file in bytes.
 * @param[in] strtab Header of the string table.
 * @param[in] off Offset of the string in the table.
 * @return The string, pointing into the mapped file, or #ELF_CORRUPT_NAME if
 * it can't be read. Never @c nullptr.
 */
const char* elf_str_at(void* addr, size_t size, const ElfN_Shdr* strtab,
                       size_t off) {
    if (!section_in_file(size, strtab) || off >= strtab->sh_size) {
        return ELF_CORRUPT_NAME;
    }
    const char* str = (const char*)addr + strtab->sh_offset + off;
    if (!memchr(str, '\0', strtab->sh_size - off)) {
        return ELF_CORRUPT_NAME;
    }
    return str;
}

/**
 * @brief Locate the section name string table and .symtab of an ELF file, along
 * with the string table linked to the symbol table.
 *
 * A null pointer skips the matching output. A symbol table that is unusable
 * (not inside the file, or with an entry size that isn't the one of a symbol)
 * is ignored.
 *
 * @param[in] addr Address of the start of the ELF file.
 * @param[in] size Size of the file in bytes.
 * @param[in] ehdr ELF header of the file.
 * @param[in] shnum Number of sections, extended numbering resolved.
 * @param[in] shstrndx Index of the section name string table, resolved.
 * @param[out] sections Filled with the section name string table (.shstrtab),
 *             or @c nullptr.
 * @param[out] symbols Filled with .symtab and its string table (.strtab), or
 *             @c nullptr. Its @c is field stays 0 if there is no usable
 *             .symtab.
 */
static void get_symbol_sections(void* addr, size_t size, const ElfN_Ehdr* ehdr,
                                size_t shnum, size_t shstrndx,
                                elf_link_str_t* sections,
                                elf_link_str_t* symbols) {
    size_t shdr_size =
        arch_global == ARCH64 ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr);
    size_t sym_size =
        arch_global == ARCH64 ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym);

    for (size_t i = 0; i < shnum; ++i) {
        ElfN_Shdr s = {};
        Shdr_to_generic(addr, ehdr->e_shoff + shdr_size * i, &s);

        if (i == shstrndx) {
            /* get the shstrndx section from ehdr */
            if (sections) {
                (*sections).elems = (ElfN_Shdr){};
                (*sections).strings = s;
                (*sections).is = 1;
            }

        } else if (s.sh_type == SHT_SYMTAB) {
            if (!symbols || s.sh_entsize != sym_size ||
                !section_in_file(size, &s)) {
                continue;
            }
            symbols->elems = s;
            /* the linked string table, checked when a name is read */
            symbols->strings = (ElfN_Shdr){};
            if (s.sh_link < shnum) {
                Shdr_to_generic(addr, ehdr->e_shoff + shdr_size * s.sh_link,
                                &symbols->strings);
            }
            symbols->is = 1;
        }
    }
}

/**
 * @brief Read the symbol table (.symtab) of an ELF file, 32 or 64 bits. The
 * returned array and its elements are allocated and registered in the resource
 * collector. The first element is the null symbol, and the array is null
 * terminated.
 *
 * Nothing is read outside of the file: the fields read from the headers are
 * checked first, and a name that can't be read is displayed as
 * #ELF_CORRUPT_NAME.
 *
 * @param[in] addr Address of the start of the ELF file.
 * @param[in] size Size of the file in bytes.
 * @return Null terminated array of symbols, or @c nullptr if there is no usable
 * .symtab. Owned by the resource collector: release it with
 * del_ptr_resource(), never with @c free().
 * @pre is_elf_format() succeeded on this file (#arch_global is set).
 */
elf_sym_print_t** read_elfN(void* addr, size_t size) {
    elf_sym_print_t** ret_array = nullptr;

    ElfN_Ehdr ehdr = {};
    Ehdr_to_generic(addr, &ehdr);

    size_t shdr_size, sym_size;
    if (arch_global == ARCH64) {
        shdr_size = sizeof(Elf64_Shdr);
        sym_size = sizeof(Elf64_Sym);
    } else {
        shdr_size = sizeof(Elf32_Shdr);
        sym_size = sizeof(Elf32_Sym);
    }

    /* No section header table: nothing to read */
    if (ehdr.e_shoff == 0) {
        return nullptr;
    }

    /* Extended numbering: the real values are stored in section 0, whose
     * position was checked by is_elf_format */
    size_t shnum = ehdr.e_shnum;
    size_t shstrndx = ehdr.e_shstrndx;
    if (shnum == 0 || shstrndx == SHN_XINDEX) {
        ElfN_Shdr first = {};
        Shdr_to_generic(addr, ehdr.e_shoff, &first);
        if (shnum == 0) {
            shnum = first.sh_size;
        }
        if (shstrndx == SHN_XINDEX) {
            shstrndx = first.sh_link;
        }
    }

    elf_link_str_t sections = {};
    elf_link_str_t symbols = {};
    get_symbol_sections(addr, size, &ehdr, shnum, shstrndx, &sections,
                        &symbols);

    if (symbols.is) {
        size_t n_sym = symbols.elems.sh_size / sym_size;

        ret_array = xcalloc(n_sym + 1, sizeof(elf_sym_print_t*));
        store_ptr_resource(ret_array);

        size_t cur_sym_idx = 0;

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

            /* symbol file */
            nm_sym->file = ((arch_global == ARCH64
                                 ? ELF64_ST_TYPE(s.st_info)
                                 : ELF32_ST_TYPE(s.st_info)) == STT_FILE);

            if (nm_sym->section) {
                /* a section symbol is named after its section */
                const char* name = ELF_CORRUPT_NAME;
                if (s.st_shndx < shnum) {
                    ElfN_Shdr sym_section = {};
                    Shdr_to_generic(addr, ehdr.e_shoff + shdr_size * s.st_shndx,
                                    &sym_section);
                    name = elf_str_at(addr, size, &sections.strings,
                                      sym_section.sh_name);
                }
                nm_sym->name = (char*)name;
            } else {
                nm_sym->name =
                    (char*)elf_str_at(addr, size, &symbols.strings, s.st_name);
            }

            /* symbol value */
            nm_sym->value = s.st_value;
            if (s.st_shndx == SHN_COMMON) {
                /* common symbol: st_value is an alignment, nm shows the size */
                nm_sym->value = s.st_size;
            }
            if (ehdr.e_type == ET_REL && s.st_shndx != SHN_UNDEF &&
                s.st_shndx < SHN_LORESERVE && s.st_shndx < shnum) {
                /* relocatable: nm (BFD) prints st_value + section address */
                ElfN_Shdr val_section = {};
                Shdr_to_generic(addr, ehdr.e_shoff + shdr_size * s.st_shndx,
                                &val_section);
                nm_sym->value += val_section.sh_addr;
            }

            /* symbol type */
            nm_sym->type = get_symbol_type(addr, size, &sections, shnum, &s);

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
