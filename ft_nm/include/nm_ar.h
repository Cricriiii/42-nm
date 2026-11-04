/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nm_ar.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/31 23:13:26 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/04 15:31:26 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file nm_ar.h
 * @brief Constants and helpers to walk an ar archive (static library).
 *
 * An archive starts with the @c ARMAG signature, followed by members. Each
 * member is a <tt>struct ar_hdr</tt> header (see @c <ar.h>) followed by its
 * data, aligned on 2 bytes.
 */

#pragma once

#include <ar.h>
#include <string.h>

/** @name Special member names
 *  Content of the @c ar_name field of the special archive members.
 *  @{ */

/** @brief Name of the symbol table member. If present, it must be first. */
#define AR_SYM_TAB "/               "

/**
 * @brief Name of the long name table member, which holds the names longer
 * than 16 bytes. It directly follows the symbol table if present, otherwise it
 * is first.
 */
#define AR_STR_TAB "//              "

/** @brief Size in bytes of the @c ar_name field of a member header. */
#define AR_NAME_LEN 16

/** @} */

/** @name Member kinds
 *  Classification of an archive member from its header.
 *  @{ */
#define AR_IS_SYM_TAB_RET 0 /**< Symbol table ("/" or "/SYM64/"). */
#define AR_IS_STR_TAB_RET 1 /**< Long name table ("//"). */
#define AR_REG_TAB_RET 2    /**< Regular member: "name/" or "/offset". */
/** @} */

/** @name Archive errors
 *  @{ */
#define AR_IS_ERR_FMT_RET -1 /**< Not an archive. */
#define AR_MALFORMED_RET -2  /**< Invalid header or member order. */
#define AR_TRUNCATED_RET -3  /**< A header or a member exceeds the file. */
/** @} */

/** @name Header checks
 *  Each macro returns non-zero if the bytes at @p addr match.
 *  @{ */

/** @brief Archive signature (@c ARMAG) at @p addr. */
#define ar_is_signature(addr) !memcmp((addr), ARMAG, SARMAG)

/** @brief Symbol table name (#AR_SYM_TAB) at @p addr. */
#define ar_is_symbol_tab(addr) !memcmp((addr), AR_SYM_TAB, AR_NAME_LEN)

/** @brief Long name table name (#AR_STR_TAB) at @p addr. */
#define ar_is_string_tab(addr) !memcmp((addr), AR_STR_TAB, AR_NAME_LEN)

/** @brief Header terminator (@c ARFMAG) at @p addr. */
#define ar_is_fmag(addr) !memcmp((addr), ARFMAG, 2)

/** @} */

/** @brief Cast @p addr to a member header (<tt>struct ar_hdr*</tt>). */
#define ar_hdr(addr) ((struct ar_hdr*)(addr))
