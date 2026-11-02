/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_ar.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/31 23:13:26 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/03 00:16:01 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ar.h>
#include <string.h>

#include "file_manip.h"


/* Archive symbol table. If present, must be the first. */
#define AR_SYM_TAB "/               "
#define AR_IS_SYM_TAB_RET 0

/* Archive string table holding filenames for filename greater than 16 bytes. */
/* Directly follow the archive symbol table if present, otherwise is first. */
#define AR_STR_TAB "//              "
#define AR_IS_STR_TAB_RET 1

/* Should be a name or /length */
#define AR_REG_TAB_RET 2

/* Archive errors */
#define AR_IS_ERR_FMT_RET -1
#define AR_MALFORMED_RET -2
#define AR_TRUNCATED_RET -3

/* Name member length */
#define AR_NAME_LEN 16

#define ar_is_signature(addr) !memcmp((addr), ARMAG, SARMAG)
#define ar_is_symbol_tab(addr) !memcmp((addr), AR_SYM_TAB, AR_NAME_LEN)
#define ar_is_string_tab(addr) !memcmp((addr), AR_STR_TAB, AR_NAME_LEN)
#define ar_is_fmag(addr) !memcmp((addr), ARFMAG, 2)

#define ar_hdr(addr) ((struct ar_hdr*)(addr))
