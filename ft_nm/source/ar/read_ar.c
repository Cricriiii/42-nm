/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_ar.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/31 23:12:48 by cgajean           #+#    #+#             */
/*   Updated: 2026/11/02 15:02:24 by cgajean          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "nm_ar.h"
#include "nm_elf.h"
#include "print.h"
#include "xalloc.h"

extern bool print_filename;

/**
 * @brief Classify an archive member from the name field of its header.
 *
 * @param hdr Header of the member to classify.
 * @return AR_IS_SYM_TAB_RET for the symbol table ("/"), AR_IS_STR_TAB_RET for
 * the long name table ("//"), AR_REG_TAB_RET for any other member.
 */
static int _ar_is_hdr(struct ar_hdr* hdr) {
    if (ar_is_symbol_tab(hdr->ar_name) || !memcmp(hdr->ar_name, "/SYM64/", 7)) {
        return AR_IS_SYM_TAB_RET;
    } else if (ar_is_string_tab(hdr->ar_name)) {
        return AR_IS_STR_TAB_RET;
    } else {
        return AR_REG_TAB_RET;
    }
}

/**
 * @brief Parse the decimal ar_size field of a member header. The field is not
 * null terminated and padded with spaces.
 *
 * @param field The ar_size field (10 bytes).
 * @param out Parsed size.
 * @return true if the field holds a number, false otherwise.
 */
static bool _parse_ar_size(const char* field, size_t* out) {
    char buf[sizeof(((struct ar_hdr*)0)->ar_size) + 1] = {};
    memcpy(buf, field, sizeof(buf) - 1);

    size_t len = sizeof(buf) - 1;
    while (len && buf[len - 1] == ' ') {
        buf[--len] = '\0';
    }
    if (len == 0) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)buf[i])) {
            return false;
        }
    }
    *out = strtoull(buf, nullptr, 10);
    return true;
}

/**
 * @brief Display the symbols of a regular archive member.
 *
 * Resolve the member name (inline, or "/<offset>" into the long name table),
 * strip the trailing '/' terminator, then print the symbols of the embedded
 * ELF file under that name. Every read stays inside the name field, the long
 * name table and the member data.
 *
 * @param mapped Memory mapping of the whole archive.
 * @param ofst Offset of the member header from the start of the mapping.
 * @param member_size Size of the member data, as declared by its header.
 * @param long_name_table Content of the "//" member, or nullptr if the
 *        archive has none.
 * @param long_name_len Size of the long name table.
 * @param file Archive path, registered back as the current file once the
 *        member has been handled.
 * @return 0 on success (a member that is not a valid ELF file is reported on
 * stderr and skipped), AR_MALFORMED_RET if its name can't be resolved.
 */
static int _display_ar_elf_segment(mappedmem_t* mapped, size_t ofst,
                                   size_t member_size, char* long_name_table,
                                   size_t long_name_len, char* file) {
    struct ar_hdr* hdr = ar_hdr(mapped->addr + ofst);
    const char* name = hdr->ar_name;
    size_t max = AR_NAME_LEN;

    /* Long name: "/<offset>" into the long name table */
    if (name[0] == '/' && isdigit((unsigned char)name[1])) {
        char num[AR_NAME_LEN] = {};
        memcpy(num, name + 1, AR_NAME_LEN - 1);
        size_t idx = strtoull(num, nullptr, 10);

        if (!long_name_table || idx >= long_name_len) {
            return AR_MALFORMED_RET;
        }
        name = long_name_table + idx;
        max = long_name_len - idx;
    }

    /* The name ends at its '/', or at the end of its line in the long table */
    const char* end = memchr(name, '/', max);
    if (!end) {
        end = memchr(name, '\n', max);
    }
    size_t len = end ? (size_t)(end - name) : max;
    while (!end && len && name[len - 1] == ' ') {
        --len;
    }

    mappedmem_t cur_memory = {.addr = (void*)(hdr + 1), .size = member_size};
    char* printed_name = xcalloc(len + 1, sizeof(char));
    memcpy(printed_name, name, len);

    /* Errors are reported under the member name, not the archive name */
    print_set_filename(printed_name);
    int fmt = is_elf_format(&cur_memory);
    if (fmt == ELF_ERR_NOT_IMPLEMENTED) {
        print_err(ELF_NOT_IMPLEMENTED);
    } else if (fmt != ELF_OK) {
        print_err(ELF_NOT_RECOGNIZED);
    } else {
        display_elf_file(&cur_memory, printed_name);
    }
    print_set_filename(file);

    free(printed_name);
    return 0;
}

/**
 * @brief Walk an archive member by member and display each ELF member.
 *
 * The symbol table ("/") must come first and appear at most once, the long
 * name table ("//") must appear at most once. Any violation makes the archive
 * malformed, as does an invalid magic or size field. A header or a member that
 * doesn't fit in the file makes the file unrecognized. Members are aligned on
 * 2 bytes.
 *
 * @param mapped Memory mapping of the whole archive.
 * @param file Archive path, printed as a header when print_filename is set.
 * @return 0 on success, 1 if the archive is malformed or truncated (reported
 * on stderr). A member that is not a valid ELF file is reported but doesn't
 * change the status.
 */
int display_ar_file(mappedmem_t* mapped, char* file) {
    size_t ofst = SARMAG;
    int ar_sym_count = 0;
    int ar_str_count = 0;
    char* long_name_table = nullptr;
    size_t long_name_len = 0;
    int retval = 0;

    /* Like nm, the archive header is printed once the first member header is
     * known to be valid, not for a file rejected right away */
    bool header_done = false;

    bool saved_print_state = print_filename;
    print_filename = true;

    while (retval == 0 && ofst < mapped->size) {
        /* A whole header and the member it describes must fit in the file */
        if (mapped->size - ofst < sizeof(struct ar_hdr)) {
            retval = AR_TRUNCATED_RET;
            break;
        }
        struct ar_hdr* hdr = ar_hdr(mapped->addr + ofst);
        size_t member_size;
        if (!ar_is_fmag(hdr->ar_fmag) ||
            !_parse_ar_size(hdr->ar_size, &member_size)) {
            retval = AR_MALFORMED_RET;
            break;
        }
        if (member_size > mapped->size - ofst - sizeof(struct ar_hdr)) {
            retval = AR_TRUNCATED_RET;
            break;
        }

        if (!header_done && saved_print_state) {
            printf("\n%s:\n", file);
        }
        header_done = true;

        switch (_ar_is_hdr(hdr)) {
            case AR_IS_SYM_TAB_RET:
                if (ar_sym_count++ || ar_str_count)
                    retval = AR_MALFORMED_RET;
                break;

            case AR_IS_STR_TAB_RET:
                if (ar_str_count++)
                    retval = AR_MALFORMED_RET;
                long_name_table = (char*)(hdr + 1);
                long_name_len = member_size;
                break;

            default: {
                int ret = _display_ar_elf_segment(mapped, ofst, member_size,
                                                  long_name_table,
                                                  long_name_len, file);
                if (ret == AR_MALFORMED_RET) {
                    retval = AR_MALFORMED_RET;
                }
                break;
            }
        }

        /* Next header: members are aligned on 2 bytes */
        ofst += sizeof(struct ar_hdr) + member_size;
        ofst += ofst & 1;
    }

    if (!header_done && retval == 0 && saved_print_state) {
        printf("\n%s:\n", file);
    }
    print_filename = saved_print_state;

    /* Report the archive errors like nm does */
    if (retval == AR_MALFORMED_RET) {
        print_err(NM_MALFORMED_AR);
        return 1;
    }
    if (retval == AR_TRUNCATED_RET) {
        print_err(ELF_NOT_RECOGNIZED);
        return 1;
    }
    return 0;
}
