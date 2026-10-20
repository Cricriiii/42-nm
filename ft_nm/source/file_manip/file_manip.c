/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_manip.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:21:52 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 17:17:43 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "file_manip.h"

#include <fcntl.h>

#include "print.h"

int open_file(const char* filename) {
    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        print_perror();
    }
    return file_fd;
}

int stat_file(int file_fd, struct stat* st) {
    int file_stats = fstat(file_fd, st);
    if (file_stats == -1) {
        return -1;
    }
    return 0;
}

int map_file(int file_fd, mappedmem_t* mapped) {
    struct stat st = {};

    if (stat_file(file_fd, &st) == -1) {
        return -1;
    }

    mapped->size = st.st_size;

    return -((mapped->addr = mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED,
                                  file_fd, 0)) == nullptr);
}