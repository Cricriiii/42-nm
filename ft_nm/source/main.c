/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/10 00:22:15 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/10 15:52:29 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>

#include "cvector.h"

struct global_args_t {
    bool debug_syms;  /* -a option*/
    bool extern_only; /* -g option*/
    bool help;        /* -h option*/
    bool no_sort;     /* -p option*/
    bool rsort;       /* -r option*/
    bool undef_only;  /* -u option*/
    cvector(char*) filename;

} global_args = {};

static const char* opt_string = "aghpru";

const struct option opts[] = {{"debug-syms", no_argument, nullptr, 'a'},
                              {"extern-only", no_argument, nullptr, 'g'},
                              {"help", no_argument, NULL, 'h'},
                              {"no-sort", no_argument, nullptr, 'p'},
                              {"reverse-sort", no_argument, nullptr, 'r'},
                              {"undefined-only", no_argument, nullptr, 'u'},
                              {nullptr, 0, nullptr, 0}};

int main(int argc, char** argv, char** envp) {
    (void)argc;
    (void)argv;
    (void)envp;

    int long_index;
    int opt;
    while ((opt = getopt_long(argc, argv, opt_string, opts, &long_index)) !=
           -1) {
        switch (opt) {
            case 'a':
                global_args.debug_syms = true;
                break;

            case 'g':
                global_args.extern_only = true;
                break;

            case 'h':
                global_args.help = true;
                break;

            case 'p':
                global_args.no_sort = true;
                break;

            case 'r':
                global_args.rsort = true;
                break;

            case 'u':
                global_args.undef_only = true;
                break;

            default:
                cvector_push_back(global_args.filename, optarg);
        }
    }

    for (int i = optind; i < argc; ++i) {
        cvector_push_back(global_args.filename, argv[i]);
    }

    if (cvector_empty(global_args.filename)) {
        cvector_push_back(global_args.filename, "a.out");
    }

    printf(
        "debug_syms = %d\n"
        "extern_only = %d\n"
        "help = %d\n"
        "no_sort = %d\n"
        "rsort = %d\n"
        "undef_only = %d\n",
        global_args.debug_syms, global_args.extern_only, global_args.help,
        global_args.no_sort, global_args.rsort, global_args.undef_only);

    printf("Files:\n");
    for (size_t i = 0; i < cvector_size(global_args.filename); i++) {
        printf("  %s\n", global_args.filename[i]);
    }        

    return (0);
}
