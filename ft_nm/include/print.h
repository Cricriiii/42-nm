/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:13:53 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/19 11:31:49 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdio.h>

void print_set_program_name(const char* pname);
void usage(FILE* stream, int status);
char* errmsg(const char* filename);
void print_perror(const char* filename);
void print_err(const char* filename, const char* errmsg);