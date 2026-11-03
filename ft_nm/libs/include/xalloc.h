/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   xalloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 14:35:26 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 22:36:39 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file xalloc.h
 * @brief Allocation wrappers that exit the program on failure.
 *
 * They never return @c nullptr, so their result needs no check.
 */

#pragma once

#include <stddef.h>

void xmalloc_set_program_name(const char* pname);

void* xmalloc(size_t size);
void* xcalloc(size_t nmemb, size_t size);
void* xrealloc(void* old_memory, size_t size);

void xmalloc_failed(size_t size);
