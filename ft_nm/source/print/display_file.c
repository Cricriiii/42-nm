/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display_file.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/01 19:34:24 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/01 23:34:14 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "nm_ar.h"
#include "nm_elf.h"
#include "print.h"
#include "resource_collector.h"

extern bool print_filename;

/**
 * @brief Check the ELF class of a mapped file and display its symbols.
 *
 * @param addr Address of the mapped memory region.
 * @param file Path of the file being processed.
 * @return int. 0 on success, 1 if the file format is not recognized.
 */
int display_elf_file(mappedmem_t* mapped, char* file) {
    /* Verify file format */
    if (is_elf_format(mapped) != -1) {
        
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
