/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf_format.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/29 18:26:54 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/02 16:22:08 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nm_elf.h"

int arch_global;

/**
 * @brief Check the e_ident field of an ELF header: magic number, class,
 * version and endianness. Set arch_global to the detected class.
 *
 * @param addr Address of the first byte of the mapped memory.
 * @return int. ELF_OK if the identification is valid, ELF_ERR_NOT_RECOGNIZED
 * if it is not an ELF file, ELF_ERR_NOT_IMPLEMENTED if it is not little endian
 * (the only supported byte order). Nothing is printed.
 */
static int _test_elf_ident(void* addr) {
    char* n_ident = addr;

    /* Starts by x7f ELF */
    if (n_ident[EI_MAG0] != ELFMAG0)
        return ELF_ERR_NOT_RECOGNIZED;
    if (n_ident[EI_MAG1] != ELFMAG1)
        return ELF_ERR_NOT_RECOGNIZED;
    if (n_ident[EI_MAG2] != ELFMAG2)
        return ELF_ERR_NOT_RECOGNIZED;
    if (n_ident[EI_MAG3] != ELFMAG3)
        return ELF_ERR_NOT_RECOGNIZED;

    /* Elf class 32/64 bits */
    if (n_ident[EI_CLASS] == ELFCLASS64)
        arch_global = ARCH64;
    else if (n_ident[EI_CLASS] == ELFCLASS32)
        arch_global = ARCH32;
    else
        return ELF_ERR_NOT_RECOGNIZED;

    /* Version */
    if (n_ident[EI_VERSION] != EV_CURRENT)
        return ELF_ERR_NOT_RECOGNIZED;

    /* Endianness */
    if (n_ident[EI_DATA] != ELFDATA2LSB)
        return ELF_ERR_NOT_IMPLEMENTED;

    return ELF_OK;
}

/**
 * @brief Tell whether a table of count entries of entsize bytes, starting at
 * off, lies entirely inside a file of size bytes. Written to never overflow,
 * whatever the values read from a corrupted header.
 *
 * @param size Size of the file in bytes.
 * @param off Offset of the table from the start of the file.
 * @param entsize Size of one entry in bytes.
 * @param count Number of entries.
 * @return bool. true if the whole table is inside the file.
 */
static bool _table_fits(size_t size, uint64_t off, uint64_t entsize,
                        uint64_t count) {
    if (off > size || entsize == 0) {
        return false;
    }
    return (size - off) / entsize >= count;
}

/**
 * @brief Check that the ELF header, the program header table and the section
 * header table fit inside the mapped file. The position of the tables is not
 * assumed: only that they lie inside the file.
 *
 * Needs arch_global to be set, so it must run after _test_elf_ident.
 *
 * @param mapped Memory mapping of the file.
 * @return int. ELF_OK if the file is large enough, ELF_ERR_NOT_RECOGNIZED
 * otherwise. Nothing is printed.
 */
static int _test_file_size(mappedmem_t* mapped) {
    const bool is64 = (arch_global == ARCH64);
    const size_t ehdr_size = is64 ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr);
    const size_t phdr_size = is64 ? sizeof(Elf64_Phdr) : sizeof(Elf32_Phdr);
    const size_t shdr_size = is64 ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr);

    /* The header itself must be readable before reading any of its fields */
    if (mapped->size < ehdr_size) {
        return ELF_ERR_NOT_RECOGNIZED;
    }

    ElfN_Ehdr ehdr = {};
    Ehdr_to_generic(mapped->addr, &ehdr);

    /* Program header table, optional */
    if (ehdr.e_phnum != 0) {
        if (ehdr.e_phentsize != phdr_size ||
            !_table_fits(mapped->size, ehdr.e_phoff, phdr_size, ehdr.e_phnum)) {
            return ELF_ERR_NOT_RECOGNIZED;
        }
    }

    /* Section header table, optional: e_shoff == 0 means no sections */
    if (ehdr.e_shoff != 0) {
        uint64_t shnum = ehdr.e_shnum;

        if (ehdr.e_shentsize != shdr_size) {
            return ELF_ERR_NOT_RECOGNIZED;
        }

        /* Extended numbering: the real count is the size of section 0 */
        if (shnum == 0) {
            if (!_table_fits(mapped->size, ehdr.e_shoff, shdr_size, 1)) {
                return ELF_ERR_NOT_RECOGNIZED;
            }
            ElfN_Shdr first = {};
            Shdr_to_generic(mapped->addr, ehdr.e_shoff, &first);
            shnum = first.sh_size;
            if (shnum == 0) {
                return ELF_ERR_NOT_RECOGNIZED;
            }
        }

        if (!_table_fits(mapped->size, ehdr.e_shoff, shdr_size, shnum)) {
            return ELF_ERR_NOT_RECOGNIZED;
        }
    }

    return ELF_OK;
}

/**
 * @brief Check that a memory region holds a supported ELF file. This module
 * only reports the kind of failure, printing it is up to the caller.
 *
 * @param mapped Memory mapping to check.
 * @return int. ELF_OK if the region is a supported ELF file, otherwise
 * ELF_ERR_NOT_RECOGNIZED or ELF_ERR_NOT_IMPLEMENTED.
 */
int is_elf_format(mappedmem_t* mapped) {
    if (mapped->size < EI_NIDENT) {
        return ELF_ERR_NOT_RECOGNIZED;
    }
    int err = _test_elf_ident(mapped->addr);
    if (err != ELF_OK) {
        return err;
    }

    return _test_file_size(mapped);
}
