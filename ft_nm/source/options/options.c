/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:03:37 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 14:53:02 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "options.h"

#include <getopt.h>

#include "print.h"

/**
 * This section defines nm short and long options required by getopt_long,
 * alongside with the nm_args struct holding their values.
 */
static const char* short_options = "aghpru";

static struct option long_options[] = {
    {"debug-syms", no_argument, nullptr, 'a'},
    {"extern-only", no_argument, nullptr, 'g'},
    {"help", no_argument, nullptr, 'h'},
    {"no-sort", no_argument, nullptr, 'p'},
    {"reverse-sort", no_argument, nullptr, 'r'},
    {"undefined-only", no_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0}};

int get_options(int n_args, char** args, nm_args_t* out) {
    int opt;

    /* Read expected options */
    while ((opt = getopt_long(n_args, args, short_options, long_options,
                              nullptr)) != -1) {
        switch (opt) {
            case 'a':
                out->debug_syms = true;
                break;

            case 'g':
                out->extern_only = true;
                break;

            case 'h':
                out->help = true;
                break;

            case 'p':
                out->no_sort = true;
                break;

            case 'r':
                out->rsort = true;
                break;

            case 'u':
                out->undef_only = true;
                break;

            default:
                return usage(stderr, -1), -1;
        }
    }

    /**
     * Treat rest as filenames. Set a.out as default if no file is provided
     */

    if (*(args + optind)) {
        out->files = args + optind;
    } else {
        static char* default_files[] = {"a.out", nullptr};
        out->files = default_files;
    }

    return 0;
}
