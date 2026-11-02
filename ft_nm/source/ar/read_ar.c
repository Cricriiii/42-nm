/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_ar.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/31 23:12:48 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/02 11:06:03 by cgajean          ###   ########.fr       */
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
 * @brief Classify an archive member from the name field of its header.
 *
 * @param hdr Header of the member to classify.
 * @return AR_IS_SYM_TAB for the symbol table ("/"), AR_IS_STR_TAB for the
 *         long name table ("//"), AR_REG_TAB for any other member.
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
 * @brief Display the symbols of a regular archive member.
 *
 * Resolve the member name (inline, or "/<offset>" into the long name table),
 * strip the trailing '/' terminator, then print the symbols of the embedded
 * ELF file under that name.
 *
 * @param mapped Memory mapping of the whole archive.
 * @param ofst Offset of the member header from the start of the mapping.
 * @param long_name_table Content of the "//" member, or nullptr if the
 *        archive has none.
 * @return 0 on success, AR_IS_ERR_FMT if the member is not a valid ELF file.
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
                snprintf(printed_name, len + 1, "%.*s", (int)(end - name), name);
            }
            bool saved_print_state = print_filename;
            print_filename = true;
            display_elf_file(&tmp, printed_name);
            print_filename = saved_print_state;
        }
    } else {
        return AR_IS_ERR_FMT;
    }
    return 0;
}

/**
 * @brief Walk an archive member by member and display each ELF member.
 *
 * The symbol table ("/") must come first and appear at most once, the long
 * name table ("//") must appear at most once. Any violation makes the archive
 * malformed.
 *
 * @param mapped Memory mapping of the whole archive.
 * @param file Archive path, printed as a header when print_filename is set.
 * @return 0 on success, AR_MALFORMED if the special members are misplaced or
 *         duplicated, AR_IS_ERR_FMT if a member is not a valid ELF file.
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