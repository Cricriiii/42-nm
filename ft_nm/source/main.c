/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 23:48:21 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 23:08:43 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "file_manip.h"
#include "nm_elf.h"
#include "options.h"
#include "print.h"
#include "resource_collector.h"
#include "xalloc.h"

/**
 * @brief Global program variables.
 *
 * @param options Parsed command line arguments.
 * @param print_filename Set when several files are requested, so that each
 * symbol list is preceded by its file name.
 */
nm_args_t options = {};
bool print_filename = false;

/**
 * @brief Check the ELF class of a mapped file and display its symbols.
 *
 * @param addr Address of the mapped memory region.
 * @param file Path of the file being processed.
 * @return int. 0 on success, 1 if the file format is not recognized.
 */
static int display_file(void* addr, char* file) {
    nm_sym_out_t** symbols;

    /* The class byte of e_ident selects the 32 or 64 bit reader */
    int ei_class = ((char*)addr)[EI_DATA - 1];
    switch (ei_class) {
        case ELFCLASS32:
            symbols = read_elf32(addr);
            break;

        case ELFCLASS64:
            symbols = read_elf64(addr);
            break;

        default:
            print_err("file format not recognized");
            return 1;
    }

    /* Output values on stdout */
    if (print_filename)
        printf("\n%s:\n", file);
    print_symbols(symbols);

    /* The symbol array was allocated by read_elfN, release it now */
    del_ptr_resource(symbols);

    return 0;
}

/**
 * @brief Program's core loop. Sequentially open, map and display each requested
 * file.
 *
 * @param files Array of file paths.
 * @return int. Returns the status code of the last processed file.
 */
static int mainloop(char** files) {
    int retval = 0;
    for (char** file = files; *file; ++file) {
        /* Register filename into print module */
        print_set_filename(*file);

        /* Acquire file descriptor */
        int file_fd;
        if ((file_fd = open_file(*file)) == -1) {
            switch (errno) {
                case EMFILE:
                case ENFILE:
                    retval = 127;
                    break;
                default:
                    retval = 1;
                    break;
            }
            continue;
        }
        store_fd_resource(file_fd);

        /* Map the file into memory */
        mappedmem_t mapped = {};
        if (map_file(file_fd, &mapped) == -1) {
            print_perror();
            exit(retval = 1);
        }

        /* Release file descriptor, only keep the mapped memory */
        del_fd_resource(file_fd);

        /* Display ELF symbols */
        retval = display_file(mapped.addr, *file);

        /* Release the mapped memory */
        munmap(mapped.addr, mapped.size);
    }
    return retval;
}

/**
 * @brief Constructor, run before main. Register the program name in the
 * allocation and print modules, initialize the resource collector and schedule
 * its destruction at exit.
 *
 * @param argc Unused.
 * @param argv Program arguments. Only argv[0] is read.
 */
__attribute((constructor)) void init([[maybe_unused]] int argc, char** argv) {
    xmalloc_set_program_name(*argv);
    print_set_program_name(*argv);
    init_resource_collector();
    atexit(destroy_resource_collector);
}

/**
 * @brief Parse the command line, then run the program main loop. File name
 * headers are enabled when more than one file is requested.
 *
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 * @return int. Error code of the last processed file.
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

    /* Output filenames if more than one files have been requested */
    print_filename =
        (options.files[0] != nullptr && options.files[1] != nullptr);

    /* Program's core loop */
    return mainloop(options.files);
}