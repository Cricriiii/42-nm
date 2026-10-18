/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resource_collector.h                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 20:38:17 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/18 22:42:34 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stddef.h>

#define ALLOC_INIT_VAL 2

typedef struct resource_collection {
    int* open_fds;
    void** allocated;
    struct meta {
        size_t fd_cur_idx;
        size_t fd_arr_siz;
        size_t fd_arr_cap;

        size_t ptr_cur_idx;
        size_t ptr_arr_siz;
        size_t ptr_arr_cap;
    } meta;
} resource_collection_t;

void init_resource_collector(void);
void destroy_resource_collector(void);

void add_ptr_resource(void* ptr);
void del_ptr_resource(void* ptr);

void add_fd_resource(int fd);
void del_fd_resource(int fd);
