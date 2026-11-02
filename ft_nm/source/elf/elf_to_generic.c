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

#include <string.h>

#include "nm_elf.h"
#include "print.h"

/**
 * @brief Widen a 32-bit ELF header into the generic (64-bit) header.
 *
 * @param addr Address of the 32-bit ELF header.
 * @return ElfN_Ehdr. The converted header.
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
 * @param addr Address of the start of the ELF file.
 * @param generic Header filled by the function.
 * @return int. 0 on success, 1 if the ELF class is unknown.
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
 * @param addr Address of the 32-bit section header.
 * @return ElfN_Shdr. The converted section header.
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
 * @param addr Address of the start of the ELF file.
 * @param offset Offset of the section header from addr.
 * @param generic Section header filled by the function.
 * @return int. 0 on success, 1 if the ELF class is unknown.
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
 * @param addr Address of the 32-bit symbol.
 * @return ElfN_Sym. The converted symbol.
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
 * @param addr Address of the start of the ELF file.
 * @param offset Offset of the symbol from addr.
 * @param generic Symbol filled by the function.
 * @return int. 0 on success, 1 if the ELF class is unknown.
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