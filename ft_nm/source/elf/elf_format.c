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

/**
 * @file elf_format.c
 * @brief ELF integrity checks, run before any other read of the file.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nm_elf.h"

/**
 * @brief Class of the file being read: #ARCH32 or #ARCH64.
 *
 * Set by is_elf_format(), read by every module that walks the ELF structures.
 */
int arch_global;

/**
 * @brief Check the @c e_ident field of an ELF header: magic number, class,
 * version and endianness. Set #arch_global to the detected class.
 *
 * Nothing is printed.
 *
 * @param[in] addr Address of the first byte of the mapped memory.
 * @retval ELF_OK The identification is valid.
 * @retval ELF_ERR_NOT_RECOGNIZED Not an ELF file.
 * @retval ELF_ERR_NOT_IMPLEMENTED Not little endian, the only supported byte
 *         order.
 * @pre At least @c EI_NIDENT bytes are readable at @p addr.
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
 * @brief Tell whether a table of @p count entries of @p entsize bytes,
 * starting at @p off, lies entirely inside a file of @p size bytes. Written to
 * never overflow, whatever the values read from a corrupted header.
 *
 * @param[in] size Size of the file in bytes.
 * @param[in] off Offset of the table from the start of the file.
 * @param[in] entsize Size of one entry in bytes.
 * @param[in] count Number of entries.
 * @retval true The whole table is inside the file.
 * @retval false It is not, or @p entsize is 0.
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
 * Nothing is printed.
 *
 * @param[in] mapped Memory mapping of the file.
 * @retval ELF_OK The file is large enough.
 * @retval ELF_ERR_NOT_RECOGNIZED A header or a table exceeds the file.
 * @pre #arch_global is set: _test_elf_ident() succeeded.
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
 * @param[in] mapped Memory mapping to check.
 * @retval ELF_OK The region is a supported ELF file.
 * @retval ELF_ERR_NOT_RECOGNIZED Not an ELF file, or truncated.
 * @retval ELF_ERR_NOT_IMPLEMENTED Valid ELF file, but not little endian.
 * @post On success, #arch_global holds the class of the file.
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
