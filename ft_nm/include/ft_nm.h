/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_nm.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/14 17:34:06 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/15 15:11:51 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <elf.h>

void e_ident_interpret(const unsigned char e_ident[16]);

void ehdr_interpret(const Elf64_Ehdr* e);
void phdr_interpret(const Elf64_Phdr* p);
void shdr_interpret(const Elf64_Shdr* s);