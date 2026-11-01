/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:13:53 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/01 23:51:31 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdio.h>

typedef struct mappedmem mappedmem_t;

typedef struct elf_sym_print elf_sym_print_t;

void set_symbol_locale(int locale);

void print_set_program_name(const char* pname);
void print_set_filename(const char* fname);
[[noreturn]] void usage(FILE* stream, int status);
void print_perror(void);
void print_err(const char* errmsg);
void print_symbols(elf_sym_print_t** symbols);

#define ELF_NOT_RECOGNIZED "file format not recognized"
#define ELF_NO_SYMBOLS "no symbols"
#define ELF_NOT_IMPLEMENTED "program can't only read LSB files"

#define NM_MALFORMED_AR "malformed archive"

/*bfd plugin: LLVM gold plugin has failed to create LTO module:
The end of the file was unexpectedly encountered*/

#define AR_UNEXPECTED_END "The end of the file was unexpectedly encountered"

int display_elf_file(mappedmem_t* mapped, char* file);
int display_ar_file(mappedmem_t* mapped, char* file);