/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_manip.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:21:46 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/18 23:23:46 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file file_manip.h
 * @brief Opening and read-only memory mapping of the input files.
 */

#pragma once

#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>

/**
 * @brief Read-only memory mapping of a whole file.
 *
 * Filled by map_file(), released by the caller with munmap().
 */
typedef struct mappedmem {
    void* addr;  /**< Start address of the mapping. */
    size_t size; /**< Size of the mapping in bytes. */
} mappedmem_t;

int open_file(const char* filename);
int stat_file(int file_fd, struct stat* st);
int map_file(int file_fd, mappedmem_t* mapped);
