/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resource_collector.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 20:38:50 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 11:14:17 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "resource_collector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xalloc.h"

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

static resource_collection_t* get_instance() {
    static resource_collection_t resources = {
        .meta.fd_arr_cap = ALLOC_INIT_VAL, .meta.ptr_arr_cap = ALLOC_INIT_VAL};
    return &resources;
}

static void extend_fd_arr(void) {
    resource_collection_t* rc = get_instance();

    int** open_fds = &rc->open_fds;
    size_t old_cap = rc->meta.fd_arr_cap;
    size_t new_cap = old_cap * 2;
    int* new_open_fds = xrealloc(*open_fds, new_cap * sizeof(*new_open_fds));

    memset(new_open_fds + old_cap, -1,
           (new_cap - old_cap) * sizeof(*new_open_fds));
    *open_fds = new_open_fds;
    rc->meta.fd_arr_cap = new_cap;
}

void extend_ptr_arr(void) {
    resource_collection_t* rc = get_instance();

    void*** allocated = &rc->allocated;
    size_t old_cap = rc->meta.ptr_arr_cap;
    size_t new_cap = old_cap * 2;
    void** new_allocated =
        xrealloc(*allocated, new_cap * sizeof(*new_allocated));

    memset(new_allocated + old_cap, 0,
           (new_cap - old_cap) * sizeof(*new_allocated));
    *allocated = new_allocated;
    rc->meta.ptr_arr_cap = new_cap;
}

#define extend_arr(arr) \
    _Generic((arr), int*: extend_fd_arr, void**: extend_ptr_arr)();

void init_resource_collector(void) {
    resource_collection_t* rc = get_instance();

    /* Allocate file descriptor array */
    rc->open_fds = calloc(rc->meta.fd_arr_cap, sizeof(*rc->open_fds));
    if (!rc->open_fds) {
        perror("init_resource_collector");
        free(rc->open_fds);
        exit(EXIT_FAILURE);
    }
    memset(rc->open_fds, -1, rc->meta.fd_arr_cap * sizeof(*rc->open_fds));

    /* Allocate pointer array */
    rc->allocated = calloc(rc->meta.ptr_arr_cap, sizeof(*rc->allocated));
    if (!rc->allocated) {
        perror("init_resource_collector");
        free(rc->open_fds);
        exit(EXIT_FAILURE);
    }
}

void destroy_resource_collector(void) {
    resource_collection_t* rc = get_instance();

    int* open_fds = rc->open_fds;
    for (size_t i = 0; i < rc->meta.fd_arr_cap; ++i) {
        if (open_fds[i] != -1) {
            close(open_fds[i]);
            open_fds[i] = -1;
        }
    }
    rc->meta.fd_cur_idx = 0;

    void** allocated = rc->allocated;
    for (size_t i = 0; i < rc->meta.ptr_arr_cap; ++i) {
        if (allocated[i] != nullptr) {
            free(allocated[i]);
            allocated[i] = nullptr;
        }
    }
    free(open_fds);
    free(allocated);
    rc->open_fds = nullptr;
    rc->allocated = nullptr;
}

void store_ptr_resource(void* ptr) {
    if (!ptr) {
        return;
    }

    resource_collection_t* rc = get_instance();

    void** allocated = rc->allocated;
    for (size_t i = rc->meta.ptr_cur_idx; i < rc->meta.ptr_arr_cap; ++i) {
        if (allocated[i] == nullptr) {
            allocated[i] = ptr;
            rc->meta.ptr_cur_idx = ++i;
            ++rc->meta.ptr_arr_siz;
            return;
        }
    }
    extend_arr(allocated);
    store_ptr_resource(ptr);
}

void del_ptr_resource(void* ptr) {
    if (!ptr) {
        return;
    }

    resource_collection_t* rc = get_instance();

    void** allocated = rc->allocated;
    for (size_t i = 0; i < rc->meta.ptr_arr_cap; ++i) {
        if (allocated[i] == ptr) {
            free(allocated[i]);
            allocated[i] = nullptr;
            rc->meta.ptr_cur_idx = i;
            --rc->meta.ptr_arr_siz;
            return;
        }
    }
}

void add_fd_resource(int fd) {
    if (fd < 0) {
        return;
    }
    resource_collection_t* rc = get_instance();

    int* open_fds = rc->open_fds;

    if (rc->meta.fd_arr_siz == rc->meta.fd_arr_cap) {
        extend_arr(open_fds);
        add_fd_resource(fd);
        return;
    }

    if (rc->meta.fd_cur_idx == rc->meta.fd_arr_cap) {
        rc->meta.fd_cur_idx = 0;
    }

    for (size_t i = rc->meta.fd_cur_idx; i < rc->meta.fd_arr_cap; ++i) {
        if (open_fds[i] == -1) {
            open_fds[i] = fd;
            rc->meta.fd_cur_idx = ++i;
            ++rc->meta.fd_arr_siz;
            return;
        }
    }
}

void del_fd_resource(int fd) {
    if (fd < 0) {
        return;
    }

    resource_collection_t* rc = get_instance();

    int* open_fds = rc->open_fds;
    for (size_t i = 0; i < rc->meta.fd_arr_cap; ++i) {
        if (open_fds[i] == fd) {
            close(open_fds[i]);
            open_fds[i] = -1;
            rc->meta.fd_cur_idx = i;
            --rc->meta.fd_arr_siz;
            return;
        }
    }
}