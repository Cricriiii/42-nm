/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:13:42 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/18 23:26:46 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "print.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xalloc.h"

static char* program_name;

void init_print(const char* pname) {
    program_name = (char*)pname;
}

void usage(FILE* stream, int status) {
    fprintf(stream, "Usage: %s [option(s)] [file(s)]\n", program_name);
    fprintf(stream, " List symbols in [file(s)] (a.out by default).\n");
    fprintf(stream,
            " The options are:\n\
    -a, --debug-syms       Display debugger-only symbols\n\
    -g, --extern-only      Display only external symbols\n\
    -p, --no-sort          Do not sort the symbols\n\
    -r, --reverse-sort     Reverse the sense of the sort\n\
    -u, --undefined-only   Display only undefined symbols\n\
    -h, --help             Display this information\n\
        \n");
    exit(status);
}

char* errmsg(const char* filename) {
    size_t errlen =
        strlen(program_name) + strlen(filename) + 3; /* add ": " and `\0` */
    char* err = xcalloc(errlen, sizeof(char));
    sprintf(err, "%s: %s", program_name, filename);
    return err;
}

void print_err(const char* filename) {
    char* err = errmsg(filename);
    perror(err);
    free(err);
}