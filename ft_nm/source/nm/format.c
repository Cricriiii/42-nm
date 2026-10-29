/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   format.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/29 18:26:54 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/29 23:41:39 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include "nm_elf.h"
#include "print.h"



/**
 * @brief Test the start of the file against 0x7f ELF
 * 
 * @param addr Start of the file
 * @return int 
 */
static int _test_mag_bytes(void* addr) {
    if (is_elf_mag(addr)) {
        return print_err(ELF_NOT_RECOGNIZED), -1;
    }
}

static int _test_file_size(mappedmem_t* mapped) {   
    ElfN_Ehdr ehdr = {};
    Ehdr_to_generic(mapped->addr, &ehdr);

    if (mapped->size < EI_NIDENT) {
        return print_err(ELF_NOT_RECOGNIZED), -1;
    }

    
    
}


/**
 * @brief 
 * 
 * @param addr 
 * @return int 
 */
int elf_format_compliance(mappedmem_t* mapped) {

    if (_test_mag_bytes(mapped->addr) == -1) {
        return -1;
    }

    
    
    
    return 0;
}

