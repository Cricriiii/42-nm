/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   format.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/29 18:26:54 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/29 18:45:42 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include "nm_elf.h"
#include "print.h"

int elf_format_compliance(void* addr) {

    /* Test the start of the file against 0x7f ELF*/
    if (is_elf_mag(addr)) {
        return print_err("file format not recognized"), 1;
    }
    
    return 0;
    return -1;
}

