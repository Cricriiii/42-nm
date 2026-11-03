/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resource_collector.h                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 20:38:17 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 23:08:43 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file resource_collector.h
 * @brief Registry of heap allocations and file descriptors, all released at
 * once at exit.
 *
 * Typical use: init_resource_collector() at startup, store_*_resource() after
 * each acquisition, del_*_resource() for an early release, and
 * destroy_resource_collector() registered with atexit().
 */

#pragma once

/** @name Lifetime
 *  @{ */
void init_resource_collector(void);
void destroy_resource_collector(void);
/** @} */

/** @name Heap allocations
 *  @{ */
void store_ptr_resource(void* ptr);
void del_ptr_resource(void* ptr);
/** @} */

/** @name File descriptors
 *  @{ */
void store_fd_resource(int fd);
void del_fd_resource(int fd);
/** @} */
