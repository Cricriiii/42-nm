/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/16 23:48:21 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/18 23:42:43 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>

#include "file_manip.h"
#include "ft_nm.h"
#include "options.h"
#include "print.h"
#include "resource_collector.h"
#include "xalloc.h"

static char* program_name; /* The name used to invoke the program */

int mainloop(char** files) {
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
            print_err(*file);
            exit(retval = 1);
        }
        del_fd_resource(file_fd);

        munmap(mapped.addr, mapped.size);
        printf("%s\n", *file);
    }
    return retval;
}

int main(int argc, char** argv) {
    nm_args_t options = {};
    if (get_options(argc, argv, &options) == -1) {
        return 1;
    }
    return mainloop(options.files);
}

__attribute((constructor)) void init([[maybe_unused]] int argc, char** argv) {
    program_name = *argv;
    xmalloc_set_program_name(*argv);
    init_resource_collector();
    atexit(destroy_resource_collector);
}

/* error open fd ulimit = 127*/
/* error open perm = 1 */