/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf_to_generic.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 17:51:25 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/29 15:31:02 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "nm_elf.h"
#include "print.h"

/**
 * @brief
 *
 * @param addr
 * @return ElfN_Ehdr
 */
static ElfN_Ehdr Ehdr32to64(void* addr) {
    Elf32_Ehdr* raw = ehdr32(addr);
    ElfN_Ehdr converted = {};

    converted.e_type = raw->e_type;
    converted.e_machine = raw->e_machine;
    converted.e_version = raw->e_version;
    converted.e_entry = raw->e_entry;
    converted.e_phoff = raw->e_phoff;
    converted.e_shoff = raw->e_shoff;
    converted.e_flags = raw->e_flags;
    converted.e_ehsize = raw->e_ehsize;
    converted.e_phentsize = raw->e_phentsize;
    converted.e_phnum = raw->e_phnum;
    converted.e_shentsize = raw->e_shentsize;
    converted.e_shnum = raw->e_shnum;
    converted.e_shstrndx = raw->e_shstrndx;

    return converted;
}

/**
 * @brief
 *
 * @param addr
 * @param generic
 * @return int
 */
int Ehdr_to_generic(void* addr, ElfN_Ehdr* generic) {
    int ei_class = ((char*)addr)[EI_CLASS];
    switch (ei_class) {
        case ELFCLASS32:
            *generic = Ehdr32to64(addr);
            return 0;

        case ELFCLASS64:
            *generic = *(ElfN_Ehdr*)addr;
            return 0;

        default:
            print_err("file format not recognized");
            return 1;
    }
}

/**
 * @brief
 *
 * @param addr
 * @return ElfN_Shdr
 */
static ElfN_Shdr Shdr32to64(void* addr) {
    Elf32_Shdr* raw = shdr32(addr);
    ElfN_Shdr converted = {};

    converted.sh_name = raw->sh_name;
    converted.sh_type = raw->sh_type;
    converted.sh_flags = raw->sh_flags;
    converted.sh_addr = raw->sh_addr;
    converted.sh_offset = raw->sh_offset;
    converted.sh_size = raw->sh_size;
    converted.sh_link = raw->sh_link;
    converted.sh_info = raw->sh_info;
    converted.sh_addralign = raw->sh_addralign;
    converted.sh_entsize = raw->sh_entsize;

    return converted;
}

/**
 * @brief
 *
 * @param addr
 * @param offset
 * @param generic
 * @return int
 */
int Shdr_to_generic(void* addr, size_t offset, ElfN_Shdr* generic) {
    int ei_class = ((char*)addr)[EI_CLASS];
    switch (ei_class) {
        case ELFCLASS32:
            *generic = Shdr32to64(addr + offset);
            return 0;

        case ELFCLASS64:
            *generic = *(ElfN_Shdr*)(addr + offset);
            return 0;

        default:
            print_err("file format not recognized");
            return 1;
    }
}

/**
 * @brief
 *
 * @param addr
 * @return ElfN_Sym
 */
static ElfN_Sym Sym32to64(void* addr) {
    Elf32_Sym* raw = sym32(addr);
    ElfN_Sym converted = {};

    converted.st_name = raw->st_name;
    converted.st_info = raw->st_info;
    converted.st_other = raw->st_other;
    converted.st_shndx = raw->st_shndx;
    converted.st_value = raw->st_value;
    converted.st_size = raw->st_size;

    return converted;
}

/**
 * @brief
 *
 * @param addr
 * @param offset
 * @param generic
 * @return int
 */
int Sym_to_generic(void* addr, size_t offset, ElfN_Sym* generic) {
    int ei_class = ((char*)addr)[EI_CLASS];
    switch (ei_class) {
        case ELFCLASS32:
            *generic = Sym32to64(addr + offset);
            return 0;

        case ELFCLASS64:
            *generic = *(ElfN_Sym*)(addr + offset);
            return 0;

        default:
            print_err("file format not recognized");
            return 1;
    }
}