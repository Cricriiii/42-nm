/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_ar.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/31 23:12:48 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/01 23:52:17 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "nm_ar.h"
#include "nm_elf.h"
#include "print.h"

extern bool print_filename;

/**
 * @brief
 *
 * @param mapped
 * @return int
 */
static int _ar_is_hdr(struct ar_hdr* hdr) {
    if (ar_is_symbol_tab(hdr->ar_name)) {
        return AR_IS_SYM_TAB;
    } else if (ar_is_string_tab(hdr->ar_name)) {
        return AR_IS_STR_TAB;
    } else {
        return AR_REG_TAB;
    }
}

/**
 * @brief
 *
 * @param mapped
 * @param ofst
 * @param long_name_table
 */
static int _display_ar_elf_segment(mappedmem_t* mapped, off_t ofst,
                                   char* long_name_table) {
    char* name = nullptr;
    void* cur_addr = mapped->addr + ofst;

    if (ar_hdr(cur_addr)->ar_name[0] == '/' &&
        isdigit(ar_hdr(cur_addr)->ar_name[1])) {
        name = long_name_table + atol(ar_hdr(cur_addr)->ar_name + 1);
    } else {
        name = ar_hdr(cur_addr)->ar_name;
    }

    mappedmem_t tmp = {.addr = cur_addr + sizeof(struct ar_hdr),
                       .size = mapped->size - ofst - sizeof(struct ar_hdr)};
    if (is_elf_format(&tmp) == 0) {
        size_t len = 0;
        if (name) {
            len = strlen(name);
            char printed_name[len + 1];
            memset(printed_name, 0, len);
            char* end = strchr(name, '/');
            if (end) {
                snprintf(printed_name, len, "%.*s", (int)(end - name), name);
            }
            display_elf_file(&tmp, printed_name);
        }
    } else {
        return AR_IS_ERR_FMT;
    }
    return 0;
}

/**
 * @brief
 *
 * @param mapped
 * @return int
 */
int display_ar_file(mappedmem_t* mapped, char* file) {
    void* cur_addr = mapped->addr + SARMAG;
    int ar_sym_count = 0;
    int ar_str_count = 0;
    char* long_name_table = nullptr;

    if (print_filename) {
        printf("\n%s:\n", file);
    }

    while (cur_addr - mapped->addr < (long)(mapped->size)) {
        int type = _ar_is_hdr(ar_hdr(cur_addr));
        switch (type) {
            case AR_IS_SYM_TAB:
                if (ar_sym_count++ || ar_str_count)
                    return AR_MALFORMED;
                break;

            case AR_IS_STR_TAB:
                if (ar_str_count++)
                    return AR_MALFORMED;
                long_name_table = (void*)(&ar_hdr(cur_addr)->ar_fmag) + 2;
                break;

            default:
                if (_display_ar_elf_segment(mapped, cur_addr - mapped->addr,
                                            long_name_table) == AR_IS_ERR_FMT) {
                    return AR_IS_ERR_FMT;
                }
                break;
        }

        size_t step = sizeof(struct ar_hdr) + atol(ar_hdr(cur_addr)->ar_size);
        cur_addr += step;
    }

    return 0;
}