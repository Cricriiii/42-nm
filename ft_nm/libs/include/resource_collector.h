/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resource_collector.h                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 20:38:17 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 10:55:28 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

void init_resource_collector(void);
void destroy_resource_collector(void);

void store_ptr_resource(void* ptr);
void del_ptr_resource(void* ptr);

void add_fd_resource(int fd);
void del_fd_resource(int fd);
