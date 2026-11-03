/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf_to_generic.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 17:51:25 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/02 23:18:50 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file elf_to_generic.c
 * @brief Conversion of 32-bit and 64-bit ELF structures into the generic
 * @c ElfN_* structures.
 *
 * Every read goes through @c memcpy(), so the source needs no alignment.
 */

#include <string.h>

#include "nm_elf.h"
#include "print.h"

/**
 * @brief Widen a 32-bit ELF header into the generic (64-bit) header.
 *
 * @param[in] addr Address of the 32-bit ELF header.
 * @return The converted header.
 */
static ElfN_Ehdr Ehdr32to64(void* addr) {
    Elf32_Ehdr raw;
    memcpy(&raw, addr, sizeof(raw));
    ElfN_Ehdr converted = {};

    converted.e_type = raw.e_type;
    converted.e_machine = raw.e_machine;
    converted.e_version = raw.e_version;
    converted.e_entry = raw.e_entry;
    converted.e_phoff = raw.e_phoff;
    converted.e_shoff = raw.e_shoff;
    converted.e_flags = raw.e_flags;
    converted.e_ehsize = raw.e_ehsize;
    converted.e_phentsize = raw.e_phentsize;
    converted.e_phnum = raw.e_phnum;
    converted.e_shentsize = raw.e_shentsize;
    converted.e_shnum = raw.e_shnum;
    converted.e_shstrndx = raw.e_shstrndx;

    return converted;
}

/**
 * @brief Read the ELF header of a file into the generic (64-bit) header,
 * whatever the class of the file.
 *
 * @param[in] addr Address of the start of the ELF file.
 * @param[out] generic Header filled by the function.
 * @retval 0 Success.
 * @retval 1 Unknown ELF class, reported on stderr. @p generic is untouched.
 */
int Ehdr_to_generic(void* addr, ElfN_Ehdr* generic) {
    int ei_class = ((char*)addr)[EI_CLASS];
    switch (ei_class) {
        case ELFCLASS32:
            *generic = Ehdr32to64(addr);
            return 0;

        case ELFCLASS64:
            memcpy(generic, addr, sizeof(*generic));
            return 0;

        default:
            print_err("file format not recognized");
            return 1;
    }
}

/**
 * @brief Widen a 32-bit ELF section header into the generic (64-bit) one.
 *
 * @param[in] addr Address of the 32-bit section header.
 * @return The converted section header.
 */
static ElfN_Shdr Shdr32to64(void* addr) {
    Elf32_Shdr raw;
    memcpy(&raw, addr, sizeof(raw));
    ElfN_Shdr converted = {};

    converted.sh_name = raw.sh_name;
    converted.sh_type = raw.sh_type;
    converted.sh_flags = raw.sh_flags;
    converted.sh_addr = raw.sh_addr;
    converted.sh_offset = raw.sh_offset;
    converted.sh_size = raw.sh_size;
    converted.sh_link = raw.sh_link;
    converted.sh_info = raw.sh_info;
    converted.sh_addralign = raw.sh_addralign;
    converted.sh_entsize = raw.sh_entsize;

    return converted;
}

/**
 * @brief Read a section header into the generic (64-bit) one, whatever the
 * class of the file.
 *
 * @param[in] addr Address of the start of the ELF file.
 * @param[in] offset Offset of the section header from @p addr.
 * @param[out] generic Section header filled by the function.
 * @retval 0 Success.
 * @retval 1 Unknown ELF class, reported on stderr. @p generic is untouched.
 * @pre The section header lies inside the file (not checked here).
 */
int Shdr_to_generic(void* addr, size_t offset, ElfN_Shdr* generic) {
    int ei_class = ((char*)addr)[EI_CLASS];
    switch (ei_class) {
        case ELFCLASS32:
            *generic = Shdr32to64(addr + offset);
            return 0;

        case ELFCLASS64:
            memcpy(generic, addr + offset, sizeof(*generic));
            return 0;

        default:
            print_err("file format not recognized");
            return 1;
    }
}

/**
 * @brief Widen a 32-bit ELF symbol into the generic (64-bit) one.
 *
 * @param[in] addr Address of the 32-bit symbol.
 * @return The converted symbol.
 */
static ElfN_Sym Sym32to64(void* addr) {
    Elf32_Sym raw;
    memcpy(&raw, addr, sizeof(raw));
    ElfN_Sym converted = {};

    converted.st_name = raw.st_name;
    converted.st_info = raw.st_info;
    converted.st_other = raw.st_other;
    converted.st_shndx = raw.st_shndx;
    converted.st_value = raw.st_value;
    converted.st_size = raw.st_size;

    return converted;
}

/**
 * @brief Read a symbol into the generic (64-bit) one, whatever the class of
 * the file.
 *
 * @param[in] addr Address of the start of the ELF file.
 * @param[in] offset Offset of the symbol from @p addr.
 * @param[out] generic Symbol filled by the function.
 * @retval 0 Success.
 * @retval 1 Unknown ELF class, reported on stderr. @p generic is untouched.
 * @pre The symbol lies inside the file (not checked here).
 */
int Sym_to_generic(void* addr, size_t offset, ElfN_Sym* generic) {
    int ei_class = ((char*)addr)[EI_CLASS];
    switch (ei_class) {
        case ELFCLASS32:
            *generic = Sym32to64(addr + offset);
            return 0;

        case ELFCLASS64:
            memcpy(generic, addr + offset, sizeof(*generic));
            return 0;

        default:
            print_err("file format not recognized");
            return 1;
    }
}