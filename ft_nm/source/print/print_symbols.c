/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_symbols.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:05:52 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/22 16:52:16 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <locale.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "nm_elf.h"
#include "options.h"
#include "print.h"
#include "resource_collector.h"

/* Parsed command line options, defined in main.c */
typedef struct nm_args nm_args_t;
extern nm_args_t options;

/**
 * @brief Compare function provided to qsort. Sort symbol names in ascending or
 * descending order, ignoring underscores. The comparison is done by strcoll
 * using the host system locales.
 *
 * @param a First pointer to nm_sym_out_t* to be compared.
 * @param b Second pointer to nm_sym_out_t* to be compared.
 * @return int. Difference between the first two differing characters, negated
 * if the reverse sort is requested.
 */

static int compare_on_name(const void* a, const void* b) {
    const char* s1 = (*(const nm_sym_out_t**)a)->name;
    const char* s2 = (*(const nm_sym_out_t**)b)->name;

    /* Skip leading '_' */
    while (*s1 && *s1 == '_') {
        ++s1;
    }
    while (*s2 && *s2 == '_') {
        ++s2;
    }

    setlocale(LC_ALL, "");
    int result = strcoll(s1, s2);

    /* If strings are equal, compare the original names */
    if (result != 0) {
        return options.rsort ? -result : result;
    }

    result = strcmp((*(nm_sym_out_t**)a)->name,
                    (*(nm_sym_out_t**)b)->name);
    return options.rsort ? -result : result;
}

/**
 * @brief Sort, filter and print symbols according to the command line options.
 * Each printed symbol is released. The first element, the null symbol, is
 * never displayed.
 *
 * @param symbols Null terminated array of pointers to symbols, or nullptr.
 */
void print_symbols(nm_sym_out_t** symbols) {
    nm_sym_out_t** ptr = symbols;

    if (!symbols) {
        print_err("no symbols");
        return;
    }

    /* Reach the end of the array to get the number of symbols */
    while (*ptr) {
        ++ptr;
    }

    /* Sort everything but the null symbol */
    if (!options.no_sort) {
        qsort(symbols + 1, ptr - symbols - 1, sizeof(nm_sym_out_t*),
              compare_on_name);
    }

    for (nm_sym_out_t** s = symbols + 1; *s; ++s) {
        /* Skip the symbols filtered out by the options */
        if ((!options.debug_syms &&
             ((*s)->section || (*s)->type == 'a' || (*s)->type == 'N')) ||
            (options.extern_only && !(*s)->external) ||
            (options.undef_only && !(*s)->undef)) {
            continue;
        }

        int arch = (*s)->arch;
        size_t len_buf = strlen((*s)->name) + (arch >> 2) + 3;
        char buf[len_buf];
        memset(buf, 0, len_buf);

        /* Undefined symbols are padded instead of having their value printed */
        if (!(*s)->undef) {
            sprintf(buf, "%.*lx %c %s", (arch >> 2), (*s)->value, (*s)->type,
                    (*s)->name);
        } else {
            sprintf(buf, "        ");
            sprintf(buf + 8 * (arch == ARCH64), "         %c %s", (*s)->type,
                    (*s)->name);
        }
        printf("%s\n", buf);

        del_ptr_resource(*s);
    }
}