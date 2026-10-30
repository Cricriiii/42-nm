/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_elf.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 23:49:55 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/30 13:24:37 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <elf.h>
#include <stddef.h>

#include "file_manip.h"

/* Cast an address to the matching 32-bit or 64-bit ELF structure */
#define ehdr32(addr) ((Elf32_Ehdr*)(addr))
#define ehdr64(addr) ((Elf64_Ehdr*)(addr))

#define phdr32(addr) ((Elf32_Phdr*)(addr))
#define phdr64(addr) ((Elf64_Phdr*)(addr))

#define shdr32(addr) ((Elf32_Shdr*)(addr))
#define shdr64(addr) ((Elf64_Shdr*)(addr))

#define sym32(addr) ((Elf32_Sym*)(addr))
#define sym64(addr) ((Elf64_Sym*)(addr))

#define rel32(addr) ((Elf32_Rel*)(addr))
#define rel64(addr) ((Elf64_Rel*)(addr))

#define rela32(addr) ((Elf32_Rela*)(addr))
#define rela64(addr) ((Elf64_Rela*)(addr))

#define dyn32(addr) ((Elf32_Dyn*)(addr))
#define dyn64(addr) ((Elf64_Dyn*)(addr))

#define nhdr32(addr) ((Elf32_Nhdr*)(addr))
#define nhdr64(addr) ((Elf64_Nhdr*)(addr))

/* Word size in bits, used to size the displayed symbol values */
#define ARCH32 32
#define ARCH64 64
#define ARCHERR -1

/* Fields of the e_ident array of the ELF header */
typedef struct elf_ei {
    int32_t mag0;
    int32_t mag1;
    int32_t mag2;
    int32_t mag3;
    int32_t class;
    int32_t data;
    int32_t version;
    int32_t osabi;
    int32_t abiversion;
} elf_ei_t;

elf_ei_t read_elf_ident(void* addr);

/**
 * @brief Architecture-independant types
 *
 */
typedef Elf64_Half ElfN_Half;
typedef Elf64_Word ElfN_Word;
typedef Elf64_Xword ElfN_Xword;
typedef Elf64_Addr ElfN_Addr;
typedef Elf64_Off ElfN_Off;
typedef Elf64_Sword ElfN_Sword;
typedef Elf64_Sxword ElfN_Sxword;
typedef Elf64_Section ElfN_Section;
typedef Elf64_Versym ElfN_Versym;

typedef struct {
    unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
    ElfN_Half e_type;                 /* Object file type */
    ElfN_Half e_machine;              /* Architecture */
    ElfN_Word e_version;              /* Object file version */
    ElfN_Addr e_entry;                /* Entry point virtual address */
    ElfN_Off e_phoff;                 /* Program header table file offset */
    ElfN_Off e_shoff;                 /* Section header table file offset */
    ElfN_Word e_flags;                /* Processor-specific flags */
    ElfN_Half e_ehsize;               /* ELF header size in bytes */
    ElfN_Half e_phentsize;            /* Program header table entry size */
    ElfN_Half e_phnum;                /* Program header table entry count */
    ElfN_Half e_shentsize;            /* Section header table entry size */
    ElfN_Half e_shnum;                /* Section header table entry count */
    ElfN_Half e_shstrndx;             /* Section header string table index */
} ElfN_Ehdr;

typedef struct {
    ElfN_Word sh_name;       /* Section name (string tbl index) */
    ElfN_Word sh_type;       /* Section type */
    ElfN_Xword sh_flags;     /* Section flags */
    ElfN_Addr sh_addr;       /* Section virtual addr at execution */
    ElfN_Off sh_offset;      /* Section file offset */
    ElfN_Xword sh_size;      /* Section size in bytes */
    ElfN_Word sh_link;       /* Link to another section */
    ElfN_Word sh_info;       /* Additional section information */
    ElfN_Xword sh_addralign; /* Section alignment */
    ElfN_Xword sh_entsize;   /* Entry size if section holds table */
} ElfN_Shdr;

typedef struct {
    ElfN_Word st_name;      /* Symbol name (string tbl index) */
    unsigned char st_info;  /* Symbol type and binding */
    unsigned char st_other; /* Symbol visibility */
    ElfN_Section st_shndx;  /* Section index */
    ElfN_Addr st_value;     /* Symbol value */
    ElfN_Xword st_size;     /* Symbol size */
} ElfN_Sym;

/* Symbol as displayed by nm */
typedef struct elf_sym_print {
    long value;   /* Symbol value */
    char* name;   /* Symbol name, points into the string table */
    int arch;     /* ARCH32 or ARCH64 */
    int type;     /* Letter displayed by nm */
    int external; /* Set for external symbols */
    int undef;    /* Set for undefined symbols */
    int section;  /* Set for STT_SECTION symbols */
} elf_sym_print_t;

/* A table section header and the header of its linked string table */
typedef struct elf_link_str {
    ElfN_Shdr elems;
    ElfN_Shdr strings;
    int is;
} elf_link_str_t;

/**
 * @brief elf_to_generic.c - Contains utilities meant to manipulate
 * architecture-independant structures
 */
int Ehdr_to_generic(void* addr, ElfN_Ehdr* generic);
int Shdr_to_generic(void* addr, size_t offset, ElfN_Shdr* generic);
int Sym_to_generic(void* addr, size_t offset, ElfN_Sym* generic);

/**
 * @brief read_elfN.c - Read ELF files. Architecture-independant.
 */
elf_sym_print_t** read_elfN(void* addr);

/**
 * @brief type_deduction.c - Contains the type deduction utilities
 *
 */
int is_debug_section(ElfN_Xword section_flag, const char* name);
int section_type_from_name(const char* name);
int is_undef(ElfN_Section st_shndx);
int is_external(ElfN_Section st_shndx, unsigned char st_info);

int get_symbol_type(void* addr, const elf_link_str_t* symbol_tab,
                    ElfN_Half shnum, const ElfN_Sym* s);

/**
 * @brief format.c - ELF integrity checks
 *
 */
#define ELF_NOT_RECOGNIZED "file format not recognized"
#define ELF_NO_SYMBOLS "no symbols"
#define ELF_NOT_IMPLEMENTED "program can't only read LSB files"

int elf_format_compliance(mappedmem_t* mapped);