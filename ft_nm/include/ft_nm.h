/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_nm.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/14 17:34:06 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/16 13:42:11 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <elf.h>

typedef enum {
    BSS,
    COMMENT,
    CTORS,
    DATA,
    DATA1,
    DEBUG,
    DTORS,
    DYNAMIC,
    DYNSTR,
    DYNSYM,
    FINI,
    GNU_VERSION,
    GNU_VERSION_D,
    GNU_VERSION_R,
    GOT,
    HASH,
    INIT,
    INTERP,
    LINE,
    NOTE,
    NOTE_ABI_TAG,
    NOTE_GNU_BUILD_ID,
    NOTE_GNU_STACK,
    NOTE_OPENBSD_IDENT,
    PLT,
    RELNAME,
    RELANAME,
    RODATA,
    RODATA1,
    SHSTRTAB,
    SYMTAB,
    TEXT
} elf_section_e;

void executable_header_interpret(const unsigned char e_ident[16]);

void ehdr_interpret(const Elf64_Ehdr* e);
void phdr_interpret(const Elf64_Phdr* p);
void shdr_interpret(const Elf64_Shdr* s, const void* e,
                    Elf64_Off shstrtab_start);
void sym_interpret(const Elf64_Sym* s);