/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_elf.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 15:38:27 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/19 15:29:13 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "nm_elf.h"

#include <stdio.h>

typedef struct nm_args nm_args_t;

extern nm_args_t options;

elf_ei_t read_elf_ident(void* addr) {
    char* ident = (char*)addr;
    return (elf_ei_t){
        .mag0 = ident[0],
        .mag1 = ident[1],
        .mag2 = ident[2],
        .mag3 = ident[3],
        .class = ident[4],
        .data = ident[5],
        .version = ident[6],
        .osabi = ident[7],
        .abiversion = ident[8],
    };
}

int read_elf32(void* addr) {
    (void)addr;
    int retval = 0;
    return retval;
}

static void get_symbol_sections(Elf64_Shdr* shdr_arr[],
                                elf_elem_str_t* sections,
                                elf_elem_str_t* symbols,
                                elf_elem_str_t* dynamic) {
    for (Elf64_Shdr** s = shdr_arr; *s; ++s) {
        if (*s == sections->strings) {
            continue;

        } else if ((*s)->sh_type == SHT_SYMTAB) {
            /* in .symtab, read .strtab index */
            (*symbols).elems = *s;
            (*symbols).strings = shdr_arr[(*s)->sh_link];

        } else if ((*s)->sh_type == SHT_DYNSYM) {
            /* in .dynsym, read .dynstr index */
            (*dynamic).elems = *s;
            (*dynamic).strings = shdr_arr[(*s)->sh_link];
        }
    }
}

int read_elf64(void* addr) {
    int retval = 0;

    /* sections start */
    Elf64_Ehdr* ehdr = ehdr64(addr);
    Elf64_Phdr* phdr_start = phdr64(addr + ehdr->e_phoff);
    Elf64_Shdr* shdr_start = shdr64(addr + ehdr->e_shoff);

    /* program header array */
    Elf64_Half phnum = ehdr->e_phnum;
    [[maybe_unused]] Elf64_Phdr* phdr_arr[phnum + 1];
    phdr_arr[phnum] = nullptr;
    for (int i = 0; i < phnum; ++i) {
        phdr_arr[i] = phdr_start + i;
    }

    /* section header array */
    Elf64_Half shnum = ehdr->e_shnum;
    [[maybe_unused]] Elf64_Shdr* shdr_arr[shnum + 1];
    shdr_arr[shnum] = nullptr;
    for (int i = 0; i < shnum; ++i) {
        shdr_arr[i] = shdr_start + i;
    }

    /* correspondance string table <-> symbol table */
    elf_elem_str_t sections = {};
    sections.strings = &shdr_start[ehdr->e_shstrndx];

    elf_elem_str_t symbols = {};
    elf_elem_str_t dynamic = {};
    get_symbol_sections(shdr_arr, &sections, &symbols, &dynamic);

    size_t n_sym;
    Elf64_Sym* symbol_tab;
    char* string_tab;

    /* Symtab */
    printf("*** start of symtab ***\n");

    n_sym = symbols.elems->sh_size / symbols.elems->sh_entsize;
    symbol_tab = sym64(addr + symbols.elems->sh_offset);
    string_tab = addr + symbols.strings->sh_offset;
    for (size_t i = 0; i < n_sym; ++i) {
        Elf64_Sym* s = &symbol_tab[i];
        printf("%.16lx %s\n", s->st_value, string_tab + s->st_name);
    }
    printf("*** end of symtab ***\n\n");

    /* Dynsym */
    printf("*** start of dynsym ***\n");

    n_sym = dynamic.elems->sh_size / dynamic.elems->sh_entsize;
    symbol_tab = sym64(addr + dynamic.elems->sh_offset);
    string_tab = addr + dynamic.strings->sh_offset;
    for (size_t i = 0; i < n_sym; ++i) {
        Elf64_Sym* s = &symbol_tab[i];
        printf("%.16lx %s\n", s->st_value, string_tab + s->st_name);
    }

    printf("*** end of dynsym ***\n\n");

    return retval;
}