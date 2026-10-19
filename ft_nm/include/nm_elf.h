/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_elf.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 23:49:55 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/19 16:20:14 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <elf.h>

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

typedef struct elf_elem_str {
    Elf64_Shdr* elems;
    Elf64_Shdr* strings;
} elf_elem_str_t;

typedef struct sym64_named {
    char* name;
    Elf64_Sym* sym;
} sym64_named_t;

int read_elf32(void* addr);
int read_elf64(void* addr);
