/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resource_collector.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 20:38:50 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/30 18:23:01 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file resource_collector.c
 * @brief Implementation of the resource collector: a singleton holding two
 * growable arrays of slots, one for file descriptors, one for allocations.
 */

#include "resource_collector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xalloc.h"

/** @brief Initial capacity, in slots, of both containers. */
#define ALLOC_INIT_VAL 2

/**
 * @brief Structure used by the resource collector. Each container is paired
 * with a current index, where the next free slot search starts, a size (number
 * of stored elements) and a capacity (number of slots).
 */
typedef struct resource_collection {
    int* open_fds;    /**< Stored file descriptors, -1 marks a free slot. */
    void** allocated; /**< Stored allocations, nullptr marks a free slot. */

    /** @brief Bookkeeping of both containers. */
    struct meta {
        size_t fd_cur_idx; /**< Slot where the next free fd search starts. */
        size_t fd_arr_siz; /**< Number of stored file descriptors. */
        size_t fd_arr_cap; /**< Number of slots of @c open_fds. */

        size_t ptr_cur_idx; /**< Slot where the next free ptr search starts. */
        size_t ptr_arr_siz; /**< Number of stored allocations. */
        size_t ptr_arr_cap; /**< Number of slots of @c allocated. */
    } meta;                 /**< Bookkeeping of both containers. */
} resource_collection_t;

/**
 * @brief Get the instance of the singleton resource collector object.
 *
 * @return Pointer to the resource collector object.
 */
static resource_collection_t* get_instance() {
    static resource_collection_t resources = {
        .meta.fd_arr_cap = ALLOC_INIT_VAL, .meta.ptr_arr_cap = ALLOC_INIT_VAL};
    return &resources;
}

/**
 * @brief Double the capacity of the file descriptor container. The new slots
 * are marked as free.
 *
 * @note Exits the program if the allocation fails (see xrealloc()).
 */
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

/**
 * @brief Double the capacity of the pointer container. The new slots are
 * marked as free.
 *
 * @note Exits the program if the allocation fails (see xrealloc()).
 */
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

/**
 * @brief Double the capacity of a container, choosing the extension function
 * from its type with @c _Generic.
 *
 * @param arr Container: @c int* calls extend_fd_arr(), @c void** calls
 * extend_ptr_arr().
 */
#define extend_arr(arr) \
    _Generic((arr), int*: extend_fd_arr, void**: extend_ptr_arr)();

/**
 * @brief Initialize the resource collector by allocating both the file
 * descriptor and pointer containers.
 *
 * @attention Must be called prior to any other use of the collector.
 * @note Prints the system error message and exits if an allocation fails.
 */
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

/**
 * @brief Release all the stored resources: close every file descriptor, free
 * every allocation, then both containers.
 *
 * @post The collector must be reinitialized with init_resource_collector()
 * before further use.
 */
void destroy_resource_collector(void) {
    resource_collection_t* rc = get_instance();

    /* Release all file descriptor resources */
    int* open_fds = rc->open_fds;
    for (size_t i = 0; i < rc->meta.fd_arr_cap; ++i) {
        if (open_fds[i] != -1) {
            close(open_fds[i]);
            open_fds[i] = -1;
        }
    }
    rc->meta.fd_cur_idx = 0;

    /* Release all pointer resources */
    void** allocated = rc->allocated;
    for (size_t i = 0; i < rc->meta.ptr_arr_cap; ++i) {
        if (allocated[i] != nullptr) {
            free(allocated[i]);
            allocated[i] = nullptr;
        }
    }

    /* Free both file descriptor and pointer containers */
    free(open_fds);
    free(allocated);
    rc->open_fds = nullptr;
    rc->allocated = nullptr;
}

/**
 * @brief Store a memory allocation address. The container grows when full.
 *
 * The collector takes ownership: the memory is freed by del_ptr_resource() or
 * destroy_resource_collector(), never directly by the caller.
 *
 * @param[in] ptr Address of the memory to be stored. @c nullptr is ignored.
 * @pre init_resource_collector() has been called.
 */
void store_ptr_resource(void* ptr) {
    if (!ptr) {
        return;
    }

    resource_collection_t* rc = get_instance();

    void** allocated = rc->allocated;

    /* Grow before the scan, so that a free slot is guaranteed */
    if (rc->meta.ptr_arr_siz == rc->meta.ptr_arr_cap) {
        extend_arr(allocated);
    }

    /* Circular scan starting at the current index, visiting every slot once.
     * The container may have moved, so read it from the collector. */
    for (size_t n = 0; n < rc->meta.ptr_arr_cap; ++n) {
        size_t i = (rc->meta.ptr_cur_idx + n) % rc->meta.ptr_arr_cap;
        if (rc->allocated[i] == nullptr) {
            rc->allocated[i] = ptr;
            rc->meta.ptr_cur_idx = (i + 1) % rc->meta.ptr_arr_cap;
            ++rc->meta.ptr_arr_siz;
            return;
        }
    }
}

/**
 * @brief Free a stored memory allocation and release its slot. The slot is
 * the next one to be reused. Unknown addresses are ignored.
 *
 * @param[in] ptr Address of the memory to be freed. @c nullptr is ignored.
 * @see store_ptr_resource()
 */
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

/**
 * @brief Store an opened file descriptor. The only check performed is that it
 * is greater than or equal to 0. The container grows when full.
 *
 * The collector takes ownership: the descriptor is closed by del_fd_resource()
 * or destroy_resource_collector(), never directly by the caller.
 *
 * @param[in] fd File descriptor to be stored. Negative values are ignored.
 * @pre init_resource_collector() has been called.
 */
void store_fd_resource(int fd) {
    if (fd < 0) {
        return;
    }
    resource_collection_t* rc = get_instance();

    int* open_fds = rc->open_fds;

    /* Grow before the scan, so that a free slot is guaranteed */
    if (rc->meta.fd_arr_siz == rc->meta.fd_arr_cap) {
        extend_arr(open_fds);
    }

    /* Circular scan starting at the current index, visiting every slot once.
     * The container may have moved, so read it from the collector. */
    for (size_t n = 0; n < rc->meta.fd_arr_cap; ++n) {
        size_t i = (rc->meta.fd_cur_idx + n) % rc->meta.fd_arr_cap;
        if (rc->open_fds[i] == -1) {
            rc->open_fds[i] = fd;
            rc->meta.fd_cur_idx = (i + 1) % rc->meta.fd_arr_cap;
            ++rc->meta.fd_arr_siz;
            return;
        }
    }
}

/**
 * @brief Close a stored file descriptor and release its slot. The slot is the
 * next one to be reused. Unknown descriptors are ignored.
 *
 * @param[in] fd File descriptor to be closed. Negative values are ignored.
 * @see store_fd_resource()
 */
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