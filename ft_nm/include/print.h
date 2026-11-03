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

/**
 * @file print.h
 * @brief Output of ft_nm: symbol lists, usage and error messages.
 */

#pragma once

#include <stdio.h>

typedef struct mappedmem mappedmem_t;

typedef struct elf_sym_print elf_sym_print_t;

/** @name Symbol output (print_symbols.c)
 *  @{ */
void set_symbol_locale(int locale);
void print_symbols(elf_sym_print_t** symbols);
/** @} */

/** @name Messages (print.c)
 *  @{ */
void print_set_program_name(const char* pname);
void print_set_filename(const char* fname);
[[noreturn]] void usage(FILE* stream, int status);
void print_perror(void);
void print_warning_directory(void);
void print_err(const char* errmsg);
/** @} */

/** @name Error messages
 *  Passed to print_err(), worded like nm.
 *  @{ */
#define ELF_NOT_RECOGNIZED "file format not recognized" /**< Not ELF. */
#define ELF_NO_SYMBOLS "no symbols"                     /**< No .symtab. */
#define ELF_NOT_IMPLEMENTED \
    "program can't only read LSB files"     /**< Big-endian file. */
#define NM_MALFORMED_AR "malformed archive" /**< Invalid ar archive. */

/**
 * @brief End of file reached while reading an archive member.
 *
 * nm prints it as: <tt>bfd plugin: LLVM gold plugin has failed to create LTO
 * module: The end of the file was unexpectedly encountered</tt>.
 */
#define AR_UNEXPECTED_END "The end of the file was unexpectedly encountered"
/** @} */

/** @name File display (display_file.c, read_ar.c)
 *  @{ */
int display_elf_file(mappedmem_t* mapped, char* file);
int display_ar_file(mappedmem_t* mapped, char* file);
/** @} */
