/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 23:13:42 by cgajean           #+#    #+#             */
/*   Updated: 2026/10/21 13:20:07 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file print.c
 * @brief Usage and error messages, worded and formatted like nm.
 *
 * Every error message is prefixed with the program name and the name of the
 * file being processed, both registered beforehand.
 */

#include "print.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "resource_collector.h"
#include "xalloc.h"

/**
 * @brief Program name, prefixed to every message.
 *
 * Set by print_set_program_name(). Not copied.
 */
static char* program_name;

/**
 * @brief Name of the file being processed, prefixed to every error message.
 *
 * Set by print_set_filename(). Not copied.
 */
static char* filename;

/**
 * @brief Let user modules register the global program name.
 *
 * @param[in] pname Program name.
 * @warning The string is not copied, it must outlive the module.
 */
void print_set_program_name(const char* pname) {
    program_name = (char*)pname;
}

/**
 * @brief Let user modules register the currently processed file name.
 *
 * @param[in] fname File name.
 * @warning The string is not copied, it must stay valid until the next call.
 */
void print_set_filename(const char* fname) {
    filename = (char*)fname;
}

/**
 * @brief Print usage and terminate the program with the status value.
 *
 * @param[in] stream Output stream: @c stdout for @c --help, @c stderr for an
 *            invalid option.
 * @param[in] status Exit code value.
 * @note Does not return.
 */
[[noreturn]] void usage(FILE* stream, int status) {
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
 * @brief Allocate and return the <tt>"program_name: filename"</tt> part of an
 * error message.
 *
 * @return The allocated string. The caller must @c free() it.
 * @pre Both the program name and the file name are registered.
 */
static char* error_header(void) {
    size_t errlen =
        strlen(program_name) + strlen(filename) + 3; /* add ": " and `\0` */
    char* err = xcalloc(errlen, sizeof(char));
    sprintf(err, "%s: %s", program_name, filename);
    return err;
}

/**
 * @brief Print a <tt>"program_name: filename: error message"</tt> error string
 * on stderr, based on perror() and the current @c errno.
 *
 * A missing file (@c ENOENT) is worded like nm:
 * <tt>"program_name: 'filename': No such file"</tt>.
 */
void print_perror(void) {
    /* nm words a missing file differently from strerror() */
    if (errno == ENOENT) {
        fprintf(stderr, "%s: '%s': No such file\n", program_name, filename);
        return;
    }
    char* err_header = error_header();
    perror(err_header);
    free(err_header);
}

/**
 * @brief Print <tt>"program_name: Warning: 'filename' is a directory"</tt> on
 * stderr.
 */
void print_warning_directory(void) {
    fprintf(stderr, "%s: Warning: '%s' is a directory\n", program_name,
            filename);
}

/**
 * @brief Print a <tt>"program_name: filename: error message"</tt> error string
 * on stderr, based on a user defined error message.
 *
 * @param[in] errmsg User defined error message, usually one of the error
 *            message macros of print.h (#ELF_NOT_RECOGNIZED...).
 */
void print_err(const char* errmsg) {
    char* err_header = error_header();
    fprintf(stderr, "%s: %s\n", err_header, errmsg);
    free(err_header);
}