/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_symbols.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 17:05:52 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 18:49:41 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "nm_elf.h"
#include "options.h"
#include "print.h"
#include "resource_collector.h"

typedef struct nm_args nm_args_t;

extern nm_args_t options;
extern char* filename;

static int compare_on_name(const void* a, const void* b) {
    const char* s1 = (*(const nm_sym_out_t**)a)->name;
    const char* s2 = (*(const nm_sym_out_t**)b)->name;

    while (*s1 && *s1 == '_')
        ++s1;
    while (*s2 && *s2 == '_')
        ++s2;

    while (*s1 && toupper(*s1) == toupper(*s2)) {
        ++s1;
        ++s2;
    }

    if (options.rsort) {
        return toupper(*s2) - toupper(*s1);
    } else {
        return toupper(*s1) - toupper(*s2);
    }
}

void print_symbols(nm_sym_out_t** symbols) {
    nm_sym_out_t** ptr = symbols;

    if (!symbols) {
        print_err("no symbols");
        return;
    }

    while (*ptr) {
        ++ptr;
    }

    if (!options.no_sort) {
        qsort(symbols + 1, ptr - symbols - 1, sizeof(nm_sym_out_t*),
              compare_on_name);
    }

    for (nm_sym_out_t** s = symbols + 1; *s; ++s) {
        if ((!options.debug_syms && (*s)->type == 'a') ||
            (options.extern_only && !(*s)->external) ||
            (options.undef_only && !(*s)->undef)) {
            continue;
        }

        size_t len_buf = strlen((*s)->name) + ((*s)->arch >> 2) + 3;
        char buf[len_buf - 1];
        memset(buf, 0, len_buf);

        if ((*s)->value || (*s)->type == 'a') {
            sprintf(buf, "%.*lx %c %s", (*s)->arch >> 2, (*s)->value,
                    (*s)->type, (*s)->name);
        } else {
            sprintf(buf, "                 %c %s", (*s)->type, (*s)->name);
        }
        printf("%s\n", buf);

        del_ptr_resource(*s);
    }
}