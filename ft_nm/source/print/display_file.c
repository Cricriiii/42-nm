/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display_file.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/01 19:34:24 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/02 12:19:41 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file display_file.c
 * @brief Display of the symbols of a single ELF file.
 */

#include "nm_ar.h"
#include "nm_elf.h"
#include "print.h"
#include "resource_collector.h"

extern bool print_filename;

/**
 * @brief Read the symbols of a mapped ELF file and display them, preceded by
 * the file name when several files are requested.
 *
 * The format is checked first, and a format error reported by is_elf_format()
 * is printed on stderr.
 *
 * @param[in] mapped Memory mapping of the ELF file.
 * @param[in] file Name displayed in the header, if any.
 * @retval 0 Success.
 * @retval 1 The file format is rejected.
 */
int display_elf_file(mappedmem_t* mapped, char* file) {
    /* Verify file format */
    switch (is_elf_format(mapped)) {
        case ELF_OK:
            break;

        case ELF_ERR_NOT_IMPLEMENTED:
            print_err(ELF_NOT_IMPLEMENTED);
            return 1;

        case ELF_ERR_NOT_RECOGNIZED:
        default:
            print_err(ELF_NOT_RECOGNIZED);
            return 1;
    }

    /* Read arch-independant ELF file */
    elf_sym_print_t** symbols = read_elfN(mapped->addr, mapped->size);

    /* Output values on stdout */
    if (print_filename) {
        printf("\n%s:\n", file);
    }
    print_symbols(symbols);

    /* The symbol array was allocated by read_elfN, release it now */
    del_ptr_resource(symbols);

    return 0;
}
