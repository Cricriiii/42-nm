/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:13:42 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/20 18:37:37 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "print.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "resource_collector.h"
#include "xalloc.h"

/**
 * @brief Global static variables.
 *
 * @param program_name Program name displayed by the print module.
 * @param filename Filename displayed by the print module.
 */
static char* program_name;
static char* filename;

/**
 * @brief Let user modules register the global program name.
 *
 * @param pname Program name.
 */
void print_set_program_name(const char* pname) {
    program_name = (char*)pname;
}

/**
 * @brief Let user modules register the currently processed file name.
 *
 * @param fname File name.
 */
void print_set_filename(const char* fname) {
    filename = (char*)fname;
}

/**
 * @brief Print usage and quit on a given status code value.
 *
 * @param stream Output FILE* stream.
 * @param status Exit code value.
 */
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

/**
 * @brief Internal function. Allocate and return the "program_name: filename"
 * part of an error message. The caller must free it.
 *
 * @return char*. The allocated string.
 */
static char* error_header(void) {
    size_t errlen =
        strlen(program_name) + strlen(filename) + 3; /* add ": " and `\0` */
    char* err = xcalloc(errlen, sizeof(char));
    sprintf(err, "%s: %s", program_name, filename);
    return err;
}

/**
 * @brief Prints a "program_name: filename: error message" error string based on
 * perror().
 *
 */
void print_perror(void) {
    char* err_header = error_header();
    perror(err_header);
    free(err_header);
}

/**
 * @brief Prints a "program_name: filename: error message" error string based on
 * a user defined error message.
 *
 * @param errmsg User defined error message.
 */
void print_err(const char* errmsg) {
    char* err_header = error_header();
    fprintf(stderr, "%s: %s\n", err_header, errmsg);
    free(err_header);
}