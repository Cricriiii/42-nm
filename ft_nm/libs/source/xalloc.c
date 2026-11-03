/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   xalloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 14:34:54 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/03 10:35:41 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file xalloc.c
 * @brief Implementation of the allocation wrappers.
 */

#include "xalloc.h"

#include <stdckdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Name displayed in the out of memory message.
 *
 * Set by xmalloc_set_program_name(). Not copied.
 */
static char* program_name;

/**
 * @brief Let user modules register the global program name.
 *
 * @param[in] pname Program name. Not copied, it must outlive the module.
 * @pre Must be called before any allocation can fail (see xmalloc_failed()).
 */
void xmalloc_set_program_name(const char* pname) {
    program_name = (char*)pname;
}

/**
 * @brief Malloc function wrapper. Minimum size adjusted to 1. Print error
 * message and exit if fails.
 *
 * @param[in] size Size to be allocated.
 * @return Pointer to the new memory allocation, never @c nullptr.
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
 * @param[in] nmemb Number of elements to allocate.
 * @param[in] size Size of each element.
 * @return Pointer to the new zero-initialized memory allocation, never
 * @c nullptr.
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
 * @param[in] old_memory Source memory, or @c nullptr. Invalid after the call.
 * @param[in] size New memory size.
 * @return Pointer to the new allocation, never @c nullptr. The bytes past the
 * old size are not initialized.
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
 * @param[in] size Size of the failed memory allocation.
 * @note Does not return: exits with @c EXIT_FAILURE.
 */
void xmalloc_failed(size_t size) {
    fprintf(stderr, "\n%s%sout of memory allocating %lu bytes\n", program_name,
            *program_name ? ": " : "", (unsigned long)size);
    exit(EXIT_FAILURE);
}
