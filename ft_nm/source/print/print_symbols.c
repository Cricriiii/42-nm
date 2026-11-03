/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_symbols.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:05:52 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/02 18:06:56 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file print_symbols.c
 * @brief Sorting, filtering and printing of a symbol list, as nm does.
 */

#include <ctype.h>
#include <locale.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "nm_elf.h"
#include "options.h"
#include "print.h"
#include "resource_collector.h"

/**
 * @brief Set the locale category used by strcoll() when sorting symbols, from
 * the environment.
 *
 * @param[in] locale Locale category (@c LC_COLLATE, @c LC_ALL...).
 */
void set_symbol_locale(int locale) {
    setlocale(locale, "");
}

/**
 * @brief Compare function provided to qsort(). Order symbols by name, in
 * ascending or descending order. The comparison is done by strcoll(), so it
 * follows the collation rules of the current locale.
 *
 * @param[in] a Pointer to the first @c elf_sym_print_t* to compare.
 * @param[in] b Pointer to the second @c elf_sym_print_t* to compare.
 * @return Negative, zero or positive as strcoll() does, with the sign
 * inverted if the reverse sort is requested (nm_args::rsort).
 */
static int compare_on_name(const void* a, const void* b) {
    const char* s1 = (*(const elf_sym_print_t**)a)->name;
    const char* s2 = (*(const elf_sym_print_t**)b)->name;

    int result = strcoll(s1, s2);
    return options.rsort ? -result : result;
}

/**
 * @brief Sort, filter and print symbols on stdout according to the command
 * line options (#options). The first element, the null symbol, is never
 * displayed.
 *
 * @param[in,out] symbols Null terminated array of pointers to symbols, as
 *                returned by read_elfN(). Sorted in place. If @c nullptr,
 *                #ELF_NO_SYMBOLS is reported on stderr.
 */
void print_symbols(elf_sym_print_t** symbols) {
    elf_sym_print_t** ptr = symbols;

    if (!symbols) {
        print_err(ELF_NO_SYMBOLS);
        return;
    }

    /* Reach the end of the array to get the number of symbols */
    while (*ptr) {
        ++ptr;
    }

    /* Sort everything but the null symbol */
    if (!options.no_sort) {
        qsort(symbols + 1, ptr - symbols - 1, sizeof(elf_sym_print_t*),
              compare_on_name);
    }

    for (elf_sym_print_t** s = symbols + 1; *s; ++s) {
        /* Skip the symbols filtered out by the options */
        if ((!options.debug_syms && ((*s)->section || (*s)->file)) ||
            (options.extern_only && !(*s)->external) ||
            (options.undef_only && !(*s)->undef)) {
            continue;
        }

        int width = arch_hex_width((*s)->arch);

        /* Undefined symbols are padded instead of having their value printed */
        if ((*s)->undef) {
            printf("%*s %c %s\n", width, "", (*s)->type, (*s)->name);
        } else {
            printf("%.*lx %c %s\n", width, (*s)->value, (*s)->type, (*s)->name);
        }
    }
}