/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:03:09 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/18 23:19:04 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

typedef struct nm_args {
    int debug_syms;  /* -a option*/
    int extern_only; /* -g option*/
    int help;        /* -h option*/
    int no_sort;     /* -p option*/
    int rsort;       /* -r option*/
    int undef_only;  /* -u option*/
    char** files;
} nm_args_t;

int get_options(int n_args, char** args, nm_args_t* out);