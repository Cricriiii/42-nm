/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_elf.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 23:49:55 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/28 18:57:54 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <elf.h>

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

/* A table section header and the header of its linked string table */
typedef struct elf_link_str_32 {
    Elf32_Shdr* elems;
    Elf32_Shdr* strings;
} elf_link_str_32_t;

typedef struct elf_link_str_64 {
    Elf64_Shdr* elems;
    Elf64_Shdr* strings;
} elf_link_str_64_t;

typedef struct elf_link_str {
    ElfN_Shdr* elems;
    ElfN_Shdr* strings;
} elf_link_str_t;

/* Arch-independant structure */
typedef Elf64_Half ElfN_Half;
typedef Elf64_Word ElfN_Word;
typedef Elf64_Word ElfN_Word;
typedef Elf64_Xword ElfN_Xword;
typedef Elf64_Addr ElfN_Addr;
typedef Elf64_Off ElfN_Off;
typedef Elf64_Xword ElfN_Xword;
typedef Elf64_Word ElfN_Word;
typedef Elf64_Word ElfN_Word;
typedef Elf64_Xword ElfN_Xword;
typedef Elf64_Xword ElfN_Xword;

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

/* Symbol as displayed by nm */
typedef struct nm_sym_out {
    long value;   /* Symbol value */
    char* name;   /* Symbol name, points into the string table */
    int arch;     /* ARCH32 or ARCH64 */
    int type;     /* Letter displayed by nm */
    int external; /* Set for external symbols */
    int undef;    /* Set for undefined symbols */
    int section;  /* Set for STT_SECTION symbols */
} nm_sym_out_t;

nm_sym_out_t** read_elf32(void* addr);
nm_sym_out_t** read_elf64(void* addr);
nm_sym_out_t** read_elfN(void* addr);

int is_debug_section(uint64_t section_flag, const char* name);
int section_type_from_name(const char* name);
int is_undef(uint64_t st_shndx);
int is_external(uint64_t st_shndx, unsigned char st_info);

int section_type_from_flags(uint64_t section_flag, uint64_t section_type,
                            const char* name);

__attribute((nonnull(1, 3))) void get_symbol_sections_32(
    Elf32_Shdr* shdr, Elf32_Half shnum, elf_link_str_32_t* sections,
    elf_link_str_32_t* symbols, elf_link_str_32_t* dynamic);

__attribute((nonnull(1, 3))) void get_symbol_sections_64(
    Elf64_Shdr* shdr, Elf64_Half shnum, elf_link_str_64_t* sections,
    elf_link_str_64_t* symbols, elf_link_str_64_t* dynamic);


#define is_elf_mag(addr) memcmp((addr), "\x7f" "ELF", 4)
    
int Ehdr_to_generic(void* addr, ElfN_Ehdr* generic);
int Shdr_to_generic(void* addr, ElfN_Shdr* generic);
