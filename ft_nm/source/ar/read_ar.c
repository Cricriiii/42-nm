/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_ar.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/31 23:12:48 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/01 00:53:50 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "nm_ar.h"
#include "nm_elf.h"
#include "print.h"
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief 
 * 
 * @param mapped 
 * @return int 
 */
static int _ar_is_hdr(struct ar_hdr* hdr) {

    if (ar_is_symbol_tab(hdr->ar_name)) {
        return AR_IS_SYM_TAB;
    }

    if (ar_is_string_tab(hdr->ar_name)) {
        return AR_IS_STR_TAB;
    }

    return AR_IS_NAMED_DATA;
    
    // return AR_IS_ERR_FMT;
}

/**
 * @brief 
 * 
 * @param mapped 
 * @return int 
 */
int ar_is_format(mappedmem_t* mapped) {
    /* Starts by !<arch> */
    if (!ar_is_signature(mapped->addr)) {
        goto not_recognized;
    }

    int ar_sym_count = 0;
    int ar_str_count = 0;
    void* cur_addr = mapped->addr + SARMAG;
    while (true) {
    
        if (!ar_is_fmag(((struct ar_hdr*)cur_addr)->ar_fmag)) {
            return AR_IS_ERR_FMT;
        }
        
        int type = _ar_is_hdr(((struct ar_hdr*)cur_addr));        
        switch (type) {
            case AR_IS_SYM_TAB:
                if (ar_sym_count++ || ar_str_count) {
                    return AR_IS_ERR_FMT;
                }
                break;

            case AR_IS_STR_TAB:
                ++ar_str_count;
                break;

            case AR_IS_NAMED_DATA:
                break;

            case AR_IS_ERR_FMT:
                return AR_IS_ERR_FMT;
        }

        size_t step = sizeof(struct ar_hdr) + atol(((struct ar_hdr*)cur_addr)->ar_size);
        cur_addr+= step;
        if (cur_addr - mapped->addr >= (long)(mapped->size))
            break;
    }

    /* Valid path return */
    return 0;

    /* Error path return */
not_recognized:
    return print_err(ELF_NOT_RECOGNIZED), -1;
}