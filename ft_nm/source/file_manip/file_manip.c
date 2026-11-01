/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_manip.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:21:52 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 23:33:51 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "file_manip.h"

#include <fcntl.h>

#include "print.h"

/**
 * @brief Open a file in read-only mode. Print the system error message on
 * failure.
 *
 * @param filename Path of the file.
 * @return int. The file descriptor, -1 on failure.
 */
int open_file(const char* filename) {
    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        print_perror();
    }
    return file_fd;
}

/**
 * @brief Retrieve the status of an open file.
 *
 * @param file_fd File descriptor.
 * @param st Structure filled with the file status.
 * @return int. 0 on success, -1 on failure.
 */
int stat_file(int file_fd, struct stat* st) {
    int file_stats = fstat(file_fd, st);
    if (file_stats == -1) {
        return -1;
    }
    return 0;
}

/**
 * @brief Map an open file into memory, in read-only mode. The caller must
 * release the mapping with munmap().
 *
 * @param file_fd File descriptor.
 * @param mapped Structure filled with the address and size of the mapping.
 * @return int. 0 on success, -1 on failure.
 */
int map_file(int file_fd, mappedmem_t* mapped) {
    struct stat st = {};

    if (stat_file(file_fd, &st) == -1) {
        return -1;
    }

    mapped->size = st.st_size;

    return -((mapped->addr = mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED,
                                  file_fd, 0)) == MAP_FAILED);
}