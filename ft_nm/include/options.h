/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:03:09 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/21 12:28:41 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file options.h
 * @brief Command line options of ft_nm.
 */

#pragma once

/** @brief Command line options, filled by get_options(). */
typedef struct nm_args {
    bool debug_syms;  /**< @c -a, @c --debug-syms: show debugger symbols. */
    bool extern_only; /**< @c -g, @c --extern-only: external symbols only. */
    bool help;        /**< @c -h, @c --help: print the usage and exit. */
    bool no_sort;     /**< @c -p, @c --no-sort: keep the file order. */
    bool rsort;       /**< @c -r, @c --reverse-sort: reverse the sort. */
    bool undef_only;  /**< @c -u, @c --undefined-only: undefined only. */
    char** files;     /**< Null terminated array of file paths. */
} nm_args_t;

extern nm_args_t options;

int get_options(int n_args, char** args, nm_args_t* out);
