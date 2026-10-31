/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:13:53 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/31 23:26:19 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdio.h>

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