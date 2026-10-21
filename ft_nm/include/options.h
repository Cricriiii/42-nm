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

#pragma once

typedef struct nm_args {
    bool debug_syms;  /* -a option */
    bool extern_only; /* -g option */
    bool help;        /* -h option */
    bool no_sort;     /* -p option */
    bool rsort;       /* -r option */
    bool undef_only;  /* -u option */
    char** files;     /* Null terminated array of file paths */
} nm_args_t;

int get_options(int n_args, char** args, nm_args_t* out);