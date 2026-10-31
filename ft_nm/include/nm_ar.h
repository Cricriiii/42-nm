/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_ar.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/31 23:13:26 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/01 00:16:15 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ar.h>
#include <string.h>
#include "file_manip.h"

int ar_is_format(mappedmem_t* mapped);

#define AR_IS_ERR_FMT -1

/* Archive symbol table. If present, must be the first. */
#define AR_SYM_TAB "/               "
#define AR_IS_SYM_TAB 1

/* Archive string table holding filenames for filename greater than 16 bytes. */
/* Directly follow the archive symbol table if present, otherwise is first. */
#define AR_STR_TAB "//              "     
#define AR_IS_STR_TAB 0

#define AR_IS_NAMED_DATA 2

#define AR_NAME_LEN 16

#define ar_is_signature(addr) !memcmp((addr), ARMAG, SARMAG)
#define ar_is_symbol_tab(addr) !memcmp((addr), AR_SYM_TAB, AR_NAME_LEN)
#define ar_is_string_tab(addr) !memcmp((addr), AR_STR_TAB, AR_NAME_LEN)
#define ar_is_fmag(addr) !memcmp((addr), ARFMAG, 2)