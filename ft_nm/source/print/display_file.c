/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display_file.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/01 19:34:24 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/02 11:04:20 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "nm_ar.h"
#include "nm_elf.h"
#include "print.h"
#include "resource_collector.h"

extern bool print_filename;

/**
 * @brief Read the symbols of a mapped ELF file and display them, preceded by
 * the file name when several files are requested.
 *
 * The format check is not wired yet: the result of is_elf_format() is ignored.
 *
 * @param mapped Memory mapping of the ELF file.
 * @param file Name displayed in the header, if any.
 * @return int. Always 0 for now.
 */
int display_elf_file(mappedmem_t* mapped, char* file) {
    /* Verify file format */
    if (is_elf_format(mapped) != -1) {
        // don't forget
    }

    /* Read arch-independant ELF file */
    elf_sym_print_t** symbols = read_elfN(mapped->addr);

    /* Output values on stdout */
    if (print_filename)
        printf("\n%s:\n", file);
    print_symbols(symbols);

    /* The symbol array was allocated by read_elfN, release it now */
    del_ptr_resource(symbols);

    return 0;
}
