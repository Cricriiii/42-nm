/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 23:48:21 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 14:58:02 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "file_manip.h"
#include "nm_elf.h"
#include "options.h"
#include "print.h"
#include "resource_collector.h"
#include "xalloc.h"

/* The name used to invoke the program */
static char* program_name; 

/* Options parsed from the command line */
nm_args_t options = {};

int set_print_file_names;

int compare_on_name(const void* a, const void* b) {
    const char* s1 = (*(const nm_sym_out_t**)a)->name;
    const char* s2 = (*(const nm_sym_out_t**)b)->name;

    while (*s1 && *s1 == '_') ++s1;
    while (*s2 && *s2 == '_') ++s2;

    while (*s1 && toupper(*s1) == toupper(*s2)) {
        ++s1;
        ++s2;
    }

    if (options.rsort) {
        return toupper(*s2) - toupper(*s1);
    } else {
        return toupper(*s1) - toupper(*s2);
    }
}

static int display_file(void* addr, char* file) {
    int retval = 0;
    int ei_class = ((char*)addr)[EI_DATA - 1];
    [[maybe_unused]] nm_sym_out_t** symbols = nullptr;

    switch (ei_class) {
        case ELFCLASS32:
            symbols = read_elf32(addr);
            break;

        case ELFCLASS64:
            printf("\n%s:\n", file);

            symbols = read_elf64(addr);
            nm_sym_out_t **ptr = symbols;

            if (!symbols) {
                print_err(file, "no symbols");
                break;
            }
            
            while (*ptr) ++ptr;
            if (!options.no_sort) {
                qsort(symbols + 1, ptr - symbols - 1, sizeof(nm_sym_out_t*), compare_on_name);
            }
            
            for (nm_sym_out_t** s = symbols + 1; *s; ++s) {
                if (!options.debug_syms && (*s)->type == 'a') {
                    continue;
                }

                if (options.extern_only && !(*s)->external) {
                    continue;
                }

                if (options.undef_only && !(*s)->undef) {
                    continue;
                }

                size_t len_buf = strlen((*s)->name) + 20;
                char buf[len_buf - 1];
                memset(buf, 0, len_buf);
                
                if ((*s)->value || (*s)->type == 'a') {
                    sprintf(buf, "%.16lx %c %s", (*s)->value, (*s)->type, (*s)->name);
                } else {
                    sprintf(buf, "                 %c %s", (*s)->type, (*s)->name);
                }
                printf("%s\n", buf);
                del_ptr_resource(*s);
                continue;
            }
            /* Don't forget stripped files */
            break;

        default:
            retval = 1;
            print_err(file, "file format not recognized");
            break;
    }
    del_ptr_resource(symbols);
    return retval;
}

static int mainloop(char** files) {
    int retval = 0;
    for (char** file = files; *file; ++file) {
        /* Acquire file descriptor */
        int file_fd;
        if ((file_fd = open_file(*file)) == -1) {
            retval = 1;
            continue;
        }
        add_fd_resource(file_fd);

        /* Map the file into memory */
        mappedmem_t mapped = {};
        if (map_file(file_fd, &mapped) == -1) {
            print_perror(*file);
            exit(retval = 1);
        }
        del_fd_resource(file_fd);

        /* Display ELF symbols */
        retval = display_file(mapped.addr, *file);

        munmap(mapped.addr, mapped.size);
    }
    return retval;
}

int main(int argc, char** argv) {
    if (get_options(argc, argv, &options) == -1) {
        return 1;
    }
    set_print_file_names = (options.files[0] != nullptr && options.files[1] != nullptr);
    return mainloop(options.files);
}

__attribute((constructor)) void init([[maybe_unused]] int argc, char** argv) {
    program_name = *argv;
    xmalloc_set_program_name(*argv);
    print_set_program_name(*argv);
    init_resource_collector();
    atexit(destroy_resource_collector);
}

/* error open fd ulimit = 127*/
/* error open perm = 1 */