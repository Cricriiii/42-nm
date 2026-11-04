/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 23:48:21 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/04 15:30:30 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file main.c
 * @brief Entry point of ft_nm: initialization, option parsing and the loop
 * over the requested files.
 */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include "file_manip.h"
#include "nm_ar.h"
#include "nm_elf.h"
#include "options.h"
#include "print.h"
#include "resource_collector.h"
#include "xalloc.h"

/** @brief Parsed command line options, filled by get_options(). */
nm_args_t options = {};

/**
 * @brief Set when several files are requested, so that each symbol list is
 * preceded by its file name.
 */
bool print_filename = false;

/**
 * @brief Program's core loop. Sequentially open, map and display each requested
 * file.
 *
 * Errors are reported on stderr and the loop goes on with the next file.
 *
 * @param[in] files Null terminated array of file paths.
 * @retval 0 Every file was processed.
 * @retval 1 At least one file failed.
 */
static int mainloop(char** files) {
    int retval = 0;
    for (char** file = files; *file; ++file) {
        /* Register filename into print module */
        print_set_filename(*file);

        /* Acquire file descriptor */
        int file_fd;
        if ((file_fd = open_file(*file)) == -1) {
            retval = 1;
            continue;
        }
        store_fd_resource(file_fd);

        /* Map the file into memory. On failure map_file reports the error
         * itself, skip to the next file */
        mappedmem_t mapped = {};
        if (map_file(file_fd, &mapped) == -1) {
            retval = 1;
            del_fd_resource(file_fd);
            continue;
        }

        /* Release file descriptor, only keep the mapped memory */
        del_fd_resource(file_fd);

        /* Display ELF symbols */
        int status;
        if (mapped.size >= SARMAG && ar_is_signature(mapped.addr)) {
            status = display_ar_file(&mapped, *file);
        } else {
            status = display_elf_file(&mapped, *file);
        }

        /* Output an error code if at least one error */
        if (status != 0) {
            retval = 1;
        }

        /* Release the mapped memory */
        munmap(mapped.addr, mapped.size);
    }
    return retval;
}

/**
 * @brief Parse the command line, then run the program main loop. File name
 * headers are enabled when more than one file is requested.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @retval 0 Every file was processed.
 * @retval 1 At least one file failed, or the command line is invalid.
 * @pre init() has run (it is a constructor).
 */
int main(int argc, char** argv) {
    /* Parse the command line arguments */
    if (get_options(argc, argv, &options) == -1) {
        return 1;
    }

    /* Display usage and quit */
    if (options.help) {
        usage(stdout, 0);
    }

    /* Output filenames if more than one file have been requested */
    print_filename =
        (options.files[0] != nullptr && options.files[1] != nullptr);

    /* Program's core loop */
    return mainloop(options.files);
}

/**
 * @brief Constructor, run before main(). Register the program name in the
 * allocation and print modules, initialize the resource collector and schedule
 * its destruction at exit.
 *
 * @param[in] argc Unused.
 * @param[in] argv Program arguments. Only @c argv[0] is read.
 */
__attribute((constructor)) void init([[maybe_unused]] int argc, char** argv) {
    set_symbol_locale(LC_ALL);
    xmalloc_set_program_name(*argv);
    print_set_program_name(*argv);
    init_resource_collector();
    atexit(destroy_resource_collector);
}
