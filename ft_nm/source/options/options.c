/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:03:37 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/21 12:38:28 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "options.h"

#include <getopt.h>

#include "print.h"

/**
 * @brief Supported options, as expected by getopt_long: the short form string
 * and the table of long forms. Both must stay in sync.
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

/**
 * @brief Parse the command line according to the supported options. The
 * remaining arguments are the files to process, "a.out" if there are none.
 * Print the usage on stderr and exit with status 1 if an unknown option is met.
 *
 * @param n_args Number of command line arguments.
 * @param args Array of null terminated strings: the arguments. Reordered by
 * getopt_long, the file list points into it.
 * @param out Option structure filled by the parser.
 * @return int. 0 on success.
 */
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
                usage(stderr, 1);
        }
    }

    /* Remaining arguments are file paths. If none, default to a.out. */
    if (*(args + optind)) {
        out->files = args + optind;
    } else {
        static char* default_files[] = {"a.out", nullptr};
        out->files = default_files;
    }

    return 0;
}
