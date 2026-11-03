/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_elf.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 23:49:55 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/01 00:53:55 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file nm_elf.h
 * @brief ELF reading: architecture-independent types, integrity checks,
 * symbol extraction and type deduction.
 *
 * 32-bit and 64-bit files are handled through a single set of generic
 * structures (@c ElfN_*), laid out like their 64-bit counterparts. The
 * @c *_to_generic() functions widen 32-bit structures into them.
 */

#pragma once

#include <elf.h>
#include <stddef.h>

#include "file_manip.h"

/** @name Casts to ELF structures
 *  Cast an address to the matching 32-bit or 64-bit structure of @c <elf.h>.
 *  @{ */
#define ehdr32(addr) ((Elf32_Ehdr*)(addr)) /**< ELF header, 32-bit. */
#define ehdr64(addr) ((Elf64_Ehdr*)(addr)) /**< ELF header, 64-bit. */

#define phdr32(addr) ((Elf32_Phdr*)(addr)) /**< Program header, 32-bit. */
#define phdr64(addr) ((Elf64_Phdr*)(addr)) /**< Program header, 64-bit. */

#define shdr32(addr) ((Elf32_Shdr*)(addr)) /**< Section header, 32-bit. */
#define shdr64(addr) ((Elf64_Shdr*)(addr)) /**< Section header, 64-bit. */

#define sym32(addr) ((Elf32_Sym*)(addr)) /**< Symbol, 32-bit. */
#define sym64(addr) ((Elf64_Sym*)(addr)) /**< Symbol, 64-bit. */

#define rel32(addr) ((Elf32_Rel*)(addr)) /**< Relocation, 32-bit. */
#define rel64(addr) ((Elf64_Rel*)(addr)) /**< Relocation, 64-bit. */

#define rela32(addr) ((Elf32_Rela*)(addr)) /**< Relocation with addend, 32. */
#define rela64(addr) ((Elf64_Rela*)(addr)) /**< Relocation with addend, 64. */

#define dyn32(addr) ((Elf32_Dyn*)(addr)) /**< Dynamic entry, 32-bit. */
#define dyn64(addr) ((Elf64_Dyn*)(addr)) /**< Dynamic entry, 64-bit. */

#define nhdr32(addr) ((Elf32_Nhdr*)(addr)) /**< Note header, 32-bit. */
#define nhdr64(addr) ((Elf64_Nhdr*)(addr)) /**< Note header, 64-bit. */
/** @} */

/** @name Architectures
 *  Word size in bits, used to size the displayed symbol values.
 *  @{ */
#define ARCH32 32  /**< 32-bit file (ELFCLASS32). */
#define ARCH64 64  /**< 64-bit file (ELFCLASS64). */
#define ARCHERR -1 /**< Unknown class. */
/** @} */

/**
 * @brief Number of hex digits of a displayed symbol value.
 *
 * @param arch #ARCH32 or #ARCH64.
 * @return 8 for #ARCH32, 16 for #ARCH64.
 */
#define arch_hex_width(arch) ((arch) >> 2)

/** @name Architecture-independent scalar types
 *  Aliases of the 64-bit types, wide enough for both classes.
 *  @{ */
typedef Elf64_Half ElfN_Half;       /**< Unsigned 16-bit. */
typedef Elf64_Word ElfN_Word;       /**< Unsigned 32-bit. */
typedef Elf64_Xword ElfN_Xword;     /**< Unsigned 64-bit. */
typedef Elf64_Addr ElfN_Addr;       /**< Virtual address. */
typedef Elf64_Off ElfN_Off;         /**< File offset. */
typedef Elf64_Sword ElfN_Sword;     /**< Signed 32-bit. */
typedef Elf64_Sxword ElfN_Sxword;   /**< Signed 64-bit. */
typedef Elf64_Section ElfN_Section; /**< Section index. */
typedef Elf64_Versym ElfN_Versym;   /**< Version symbol information. */
/** @} */

/** @brief Architecture-independent ELF header. @see Ehdr_to_generic() */
typedef struct {
    unsigned char e_ident[EI_NIDENT]; /**< Magic number and other info. */
    ElfN_Half e_type;                 /**< Object file type. */
    ElfN_Half e_machine;              /**< Architecture. */
    ElfN_Word e_version;              /**< Object file version. */
    ElfN_Addr e_entry;                /**< Entry point virtual address. */
    ElfN_Off e_phoff;                 /**< Program header table file offset. */
    ElfN_Off e_shoff;                 /**< Section header table file offset. */
    ElfN_Word e_flags;                /**< Processor-specific flags. */
    ElfN_Half e_ehsize;               /**< ELF header size in bytes. */
    ElfN_Half e_phentsize;            /**< Program header table entry size. */
    ElfN_Half e_phnum;                /**< Program header table entry count. */
    ElfN_Half e_shentsize;            /**< Section header table entry size. */
    ElfN_Half e_shnum;                /**< Section header table entry count. */
    ElfN_Half e_shstrndx;             /**< Section header string table index. */
} ElfN_Ehdr;

/** @brief Architecture-independent section header. @see Shdr_to_generic() */
typedef struct {
    ElfN_Word sh_name;       /**< Section name (string table index). */
    ElfN_Word sh_type;       /**< Section type. */
    ElfN_Xword sh_flags;     /**< Section flags. */
    ElfN_Addr sh_addr;       /**< Section virtual address at execution. */
    ElfN_Off sh_offset;      /**< Section file offset. */
    ElfN_Xword sh_size;      /**< Section size in bytes. */
    ElfN_Word sh_link;       /**< Link to another section. */
    ElfN_Word sh_info;       /**< Additional section information. */
    ElfN_Xword sh_addralign; /**< Section alignment. */
    ElfN_Xword sh_entsize;   /**< Entry size if the section holds a table. */
} ElfN_Shdr;

/** @brief Architecture-independent symbol. @see Sym_to_generic() */
typedef struct {
    ElfN_Word st_name;      /**< Symbol name (string table index). */
    unsigned char st_info;  /**< Symbol type and binding. */
    unsigned char st_other; /**< Symbol visibility. */
    ElfN_Section st_shndx;  /**< Section index. */
    ElfN_Addr st_value;     /**< Symbol value. */
    ElfN_Xword st_size;     /**< Symbol size. */
} ElfN_Sym;

/**
 * @brief Symbol as displayed by nm.
 *
 * Built by read_elfN() from an ElfN_Sym, filtered and printed by
 * print_symbols().
 */
typedef struct elf_sym_print {
    long value;   /**< Value displayed by nm. */
    char* name;   /**< Symbol name, points into the mapped file. */
    int arch;     /**< #ARCH32 or #ARCH64. */
    int type;     /**< Letter displayed by nm. */
    int external; /**< Non-zero for external symbols. */
    int undef;    /**< Non-zero for undefined symbols. */
    int section;  /**< Non-zero for @c STT_SECTION symbols. */
    int file;     /**< Non-zero for @c STT_FILE symbols. */
} elf_sym_print_t;

/**
 * @brief A table section and the string table its names point into.
 *
 * Filled by get_symbol_sections(), for either .symtab and .strtab, or the
 * section name string table alone.
 */
typedef struct elf_link_str {
    ElfN_Shdr elems;   /**< Header of the table section (empty if none). */
    ElfN_Shdr strings; /**< Header of the linked string table. */
    int is;            /**< Non-zero if the section was found. */
} elf_link_str_t;

/** @name Conversion to the generic structures (elf_to_generic.c)
 *  @{ */
int Ehdr_to_generic(void* addr, ElfN_Ehdr* generic);
int Shdr_to_generic(void* addr, size_t offset, ElfN_Shdr* generic);
int Sym_to_generic(void* addr, size_t offset, ElfN_Sym* generic);
/** @} */

/** @name Symbol extraction (read_elfN.c)
 *  @{ */
elf_sym_print_t** read_elfN(void* addr, size_t size);
const char* elf_str_at(void* addr, size_t size, const ElfN_Shdr* strtab,
                       size_t off);
/** @} */

/** @name Symbol type deduction (type_deduction.c)
 *  @{ */
int is_debug_section(ElfN_Xword section_flag, const char* name);
int is_undef(ElfN_Section st_shndx);
int is_external(ElfN_Section st_shndx, unsigned char st_info);
int get_symbol_type(void* addr, size_t size, const elf_link_str_t* symbol_tab,
                    size_t shnum, const ElfN_Sym* s);
/** @} */

/** @name Integrity checks (elf_format.c)
 *  Results of is_elf_format(). Printing is left to the caller.
 *  @{ */
#define ELF_OK 0                   /**< Supported ELF file. */
#define ELF_ERR_NOT_RECOGNIZED -1  /**< Not an ELF file, or truncated. */
#define ELF_ERR_NOT_IMPLEMENTED -2 /**< Valid ELF, but not little-endian. */

int is_elf_format(mappedmem_t* mapped);
/** @} */
