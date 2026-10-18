/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   xalloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 14:34:54 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/18 22:44:09 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "xalloc.h"

#include <stdckdint.h>
#include <stdio.h>
#include <stdlib.h>

static char* program_name;

void xmalloc_set_program_name(const char* s) {
    program_name = (char*)s;
}

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

void xmalloc_failed(size_t size) {
    fprintf(stderr, "\n%s%sout of memory allocating %lu bytes\n", program_name,
            *program_name ? ": " : "", (unsigned long)size);
    exit(EXIT_FAILURE);
}
