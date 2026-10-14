/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   elf.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/14 17:32:41 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/14 21:43:37 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>

#include "ft_nm.h"
#include <stdlib.h>
#include <stdio.h>

// #define e_ident_interpret(bits) 
//     Elf##bits##_Ehdr e_ident##bits##_interpret(unsigned char* e_ident)

// e_ident_interpret(32) {
// }

void e_ident_interpret(const unsigned char e_ident[16]) {
    printf("Format: %c%c%c%c\n", e_ident[EI_MAG0], e_ident[EI_MAG1], e_ident[EI_MAG2], e_ident[EI_MAG3]);

    switch (e_ident[EI_CLASS]) {
        case ELFCLASSNONE:
            printf("Class: %s\n", "This class is invalid.");
            break;
        case ELFCLASS32:
            printf("Class: %s\n", "This defines the 32-bit architecture.");
            break;
        case ELFCLASS64:
            printf("Class: %s\n", "This defines the 64-bit architecture.");
            break;
    }
    switch (e_ident[EI_DATA]) {
        case ELFDATANONE:
            printf("Data: %s\n", "Unknown data format.");
            break;
        case ELFDATA2LSB:
            printf("Data: %s\n", "Two's complement, little-endian.");
            break;
        case ELFDATA2MSB:
            printf("Data: %s\n", "Two's complement, big-endian.");
            break;        
    }

    switch (e_ident[EI_VERSION]) {
        case EV_NONE:
            printf("Version: %s\n", "Invalid version.");
            break;
        case EV_CURRENT:
            printf("Version: %s\n", "Current version.");
            break;
    }

    switch (e_ident[EI_OSABI]) {
        case ELFOSABI_NONE:  // same as case ELFOSABI_SYSV:
            printf("OS_ABI: %s\n", "Same as ELFOSABI_SYSV");
            break;
        case ELFOSABI_HPUX:
            printf("OS_ABI: %s\n", "HP-UX ABI");
            break;
        case ELFOSABI_NETBSD:
            printf("OS_ABI: %s\n", "NetBSD ABI");
            break;
        case ELFOSABI_LINUX:
            printf("OS_ABI: %s\n", "Linux ABI");
            break;
        case ELFOSABI_SOLARIS:
            printf("OS_ABI: %s\n", "Solaris ABI");
            break;
        case ELFOSABI_IRIX:
            printf("OS_ABI: %s\n", "IRIX ABI");
            break;
        case ELFOSABI_FREEBSD:
            printf("OS_ABI: %s\n", "FreeBSD ABI");
            break;
        case ELFOSABI_TRU64:
            printf("OS_ABI: %s\n", "TRU64 UNIX ABI");
            break;
        case ELFOSABI_ARM:
            printf("OS_ABI: %s\n", "ARM architecture ABI");
            break;
        case ELFOSABI_STANDALONE:
            printf("OS_ABI: %s\n", "Stand-alone (embedded) ABI");
            break;
    }
    printf("ABI_VERSION: %d\n", e_ident[EI_ABIVERSION]);
    printf("PAD: %d\n", e_ident[EI_PAD]);
    printf("N_INDENT: %d\n", e_ident[EI_NIDENT]);
}