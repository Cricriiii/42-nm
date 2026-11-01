/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf_format.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/29 18:26:54 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/31 23:16:18 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>

#include "nm_elf.h"
#include "print.h"

int arch_global;

/**
 * @brief Check the e_ident field of an ELF header: magic number, class,
 * version and endianness. Set arch_global to the detected class.
 *
 * @param addr Address of the first byte of the mapped memory.
 * @return int. 0 if the identification is valid, -1 otherwise (an error is
 * printed). Only little endian files are supported.
 */
static int _test_elf_ident(void* addr) {
    char* n_ident = addr;

    /* Starts by x7f ELF */
    if (n_ident[EI_MAG0] != ELFMAG0)
        goto not_recognized;
    if (n_ident[EI_MAG1] != ELFMAG1)
        goto not_recognized;
    if (n_ident[EI_MAG2] != ELFMAG2)
        goto not_recognized;
    if (n_ident[EI_MAG3] != ELFMAG3)
        goto not_recognized;

    /* Elf class 32/64 bits */
    if (n_ident[EI_CLASS] == ELFCLASS64)
        arch_global = ARCH64;
    else if (n_ident[EI_CLASS] == ELFCLASS32)
        arch_global = ARCH32;
    else
        goto not_recognized;

    /* Version */
    if (n_ident[EI_VERSION] != EV_CURRENT)
        goto not_recognized;

    /* Endianness */
    if (n_ident[EI_DATA] != ELFDATA2LSB)
        goto not_implemented;

    /* Valid path return */
    return 0;

    /* Error path return */
not_recognized:
    return print_err(ELF_NOT_RECOGNIZED), -1;

not_implemented:
    return print_err(ELF_NOT_IMPLEMENTED), -1;
}

/**
 * @brief Check that the program and section header tables fit inside the
 * mapped file. The checks are currently disabled, so it always succeeds.
 *
 * @param mapped Memory mapping of the file.
 * @return int. 0 if the file is large enough, -1 otherwise.
 */
static int _test_file_size(mappedmem_t* mapped) {
    ElfN_Ehdr ehdr = {};
    Ehdr_to_generic(mapped->addr, &ehdr);

    // /* Program header */
    // if (ehdr.e_phoff + ehdr.e_phnum * ehdr.e_phentsize > mapped->size)
    //     goto not_recognized;

    // /* Section header */
    // if (ehdr.e_shoff + ehdr.e_shnum * ehdr.e_shentsize > mapped->size)
    //     goto not_recognized;

    /* Valid path return */
    return 0;

    /* Error path return */
    // not_recognized:
    //     return print_err(ELF_NOT_RECOGNIZED), -1;
}

/**
 * @brief Check that a memory region holds a supported ELF file.
 *
 * @param mapped Memory mapping to check.
 * @return int. 0 if the region is a supported ELF file, -1 otherwise.
 */
int is_elf_format(mappedmem_t* mapped) {
    if (_test_elf_ident(mapped->addr) == -1) {
        return -1;
    }

    if (_test_file_size(mapped) == -1) {
        return -1;
    }

    return 0;
}
