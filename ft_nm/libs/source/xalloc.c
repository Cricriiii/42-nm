/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   xalloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 14:34:54 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 22:41:42 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "xalloc.h"

#include <stdckdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Global static variables.
 *
 * @param program_name Name displayed in the out of memory message.
 *
 */
static char* program_name;

/**
 * @brief Let user modules register the global program name.
 *
 * @param pname Program name.
 */
void xmalloc_set_program_name(const char* pname) {
    program_name = (char*)pname;
}

/**
 * @brief Malloc function wrapper. Minimum size adjusted to 1. Print error
 * message and exit if fails.
 *
 * @param size Size to be allocated.
 * @return void* Pointer to the new memory allocation.
 */
void* xmalloc(size_t size) {
    if (!size) {
        size = 1;
    }

    void* new_memory = malloc(size);
    if (!new_memory) {
        xmalloc_failed(size);
    }

    return new_memory;
}

/**
 * @brief Calloc function wrapper. Minimum size adjusted to 1. Print error
 * message and exit if fails, or in case of size_t overflow.
 *
 * @param nmemb Number of elements to allocate.
 * @param size Size of each element.
 * @return void* Pointer to the new initialized memory allocation.
 */
void* xcalloc(size_t nmemb, size_t size) {
    if (!nmemb || !size) {
        nmemb = size = 1;
    }

    size_t total_size;
    if (ckd_mul(&total_size, nmemb, size)) {
        perror("calloc: size overflow");
        xmalloc_failed(nmemb * size);
    }

    void* new_memory = calloc(nmemb, size);
    if (!new_memory) {
        xmalloc_failed(size);
    }

    return new_memory;
}

/**
 * @brief Realloc function wrapper. Use malloc if source memory is null, realloc
 * otherwise. Print error message and exit if allocation fails.
 *
 * @param old_memory Source memory.
 * @param size New memory size.
 * @return void* Pointer to the new non-initialized memory allocation.
 */
void* xrealloc(void* old_memory, size_t size) {
    if (!size) {
        size = 1;
    }

    void* new_memory;
    if (!old_memory) {
        new_memory = xmalloc(size);
    } else {
        new_memory = realloc(old_memory, size);
    }

    if (!new_memory) {
        xmalloc_failed(size);
    }

    return new_memory;
}

/**
 * @brief Print out an error message and exit in case of memory allocation
 * failure.
 *
 * @param size Size of the failed memory allocation.
 */
void xmalloc_failed(size_t size) {
    fprintf(stderr, "\n%s%sout of memory allocating %lu bytes\n", program_name,
            *program_name ? ": " : "", (unsigned long)size);
    exit(EXIT_FAILURE);
}
