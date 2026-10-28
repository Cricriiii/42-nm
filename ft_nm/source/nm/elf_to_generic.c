/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf_to_generic.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 17:51:25 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/28 18:48:28 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "nm_elf.h"
#include "print.h"

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

int Shdr_to_generic(void* addr, ElfN_Shdr* generic) {
    int ei_class = ((char*)addr)[EI_CLASS];
    switch (ei_class) {
        case ELFCLASS32:
            *generic = Shdr32to64(addr);
            return 0;

        case ELFCLASS64:
            *generic = *(ElfN_Shdr*)addr;
            return 0;

        default:
            print_err("file format not recognized");
            return 1;
    }
}

