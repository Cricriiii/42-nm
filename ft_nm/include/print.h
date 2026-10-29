/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:13:53 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/29 18:36:24 by cgajean          ###   ########.fr       */
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