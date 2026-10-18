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

#pragma once

#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct mappedmem {
    void* addr;
    size_t size;
} mappedmem_t;

int open_file(const char* filename);
int stat_file(int file_fd, struct stat* st);
int map_file(int file_fd, mappedmem_t* mapped);