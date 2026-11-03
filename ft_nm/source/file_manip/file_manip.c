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

/**
 * @file file_manip.c
 * @brief Opening, status and read-only memory mapping of the input files.
 */

#include "file_manip.h"

#include <fcntl.h>

#include "print.h"

/**
 * @brief Open a file in read-only mode. Print the system error message on
 * failure.
 *
 * @param[in] filename Path of the file.
 * @return The file descriptor, or -1 on failure (already reported on stderr).
 * @pre The file name is registered with print_set_filename().
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
 * Nothing is printed, errno is set by fstat().
 *
 * @param[in] file_fd File descriptor.
 * @param[out] st Structure filled with the file status.
 * @retval 0 Success.
 * @retval -1 Failure.
 */
int stat_file(int file_fd, struct stat* st) {
    int file_stats = fstat(file_fd, st);
    if (file_stats == -1) {
        return -1;
    }
    return 0;
}

/**
 * @brief Map an open file into memory, in read-only mode.
 *
 * Directories and empty files can't be mapped: the former is reported by a
 * warning, the latter is silently refused, as nm does. Any other failure is
 * reported with the system error message.
 *
 * @param[in] file_fd File descriptor. It can be closed once mapped.
 * @param[out] mapped Structure filled with the address and size of the
 *             mapping.
 * @retval 0 Success.
 * @retval -1 Failure, already reported to the user.
 * @post On success, the caller must release the mapping with @c munmap().
 */
int map_file(int file_fd, mappedmem_t* mapped) {
    struct stat st = {};

    if (stat_file(file_fd, &st) == -1) {
        print_perror();
        return -1;
    }
    if (S_ISDIR(st.st_mode)) {
        print_warning_directory();
        return -1;
    }
    if (st.st_size == 0) {
        return -1;
    }

    mapped->size = st.st_size;
    mapped->addr =
        mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
    if (mapped->addr == MAP_FAILED) {
        print_perror();
        return -1;
    }
    return 0;
}
