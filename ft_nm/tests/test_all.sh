#!/bin/bash
# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_all.sh                                        :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/03 11:48:55 by cgajean           #+#    #+#              #
#    Updated: 2026/11/03 11:50:41 by cgajean          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Usage: FT_NM=<binary> [RESULTS_DIR=<dir>] test_all.sh [nm options] [files...]
#
# Compare the output and the exit status of nm and ft_nm, on all the files at
# once.
#
# Environment:
#   FT_NM       Path to the ft_nm binary (required)
#   RESULTS_DIR Directory of the result files (default: .test_results/, at
#               the root of the project)

set -uo pipefail

# ---------------------------------------------------------------------------- #
# settings                                                                     #
# ---------------------------------------------------------------------------- #

: "${FT_NM:?FT_NM is not set}"
: "${RESULTS_DIR:=$(dirname "$0")/../.test_results}"

OUT_NM="$RESULTS_DIR/out_nm.txt"
OUT_FT_NM="$RESULTS_DIR/out_ft_nm.txt"
DIFF_NM="$RESULTS_DIR/diff_nm.txt"

# ---------------------------------------------------------------------------- #
# display                                                                      #
# ---------------------------------------------------------------------------- #

# Print the banner of the test module
print_banner() {
    cat << 'EOF'

********************************************************************************
*                                 Test module                                  *
********************************************************************************

EOF
}

# Print the name of the tested file
# $1: file name
print_file_name() {
    local file=$1
    cat << EOF
${file}:

EOF
}

# Print the separator that closes the test of a file
print_separator() {
    cat << 'EOF'

--------------------------------------------------------------------------------
EOF
}

# Print the result of a check, and count it in failures if it failed
# $1: check name, $2: OK or FAIL, $3: optional detail
print_result() {
    local check=$1
    local result=$2
    local detail=${3:-}

    printf '\t%-9s %s%s\n' "$check:" "$result" "${detail:+ ($detail)}"
    if [ "$result" = "FAIL" ]; then
        failures=$((failures + 1))
    fi
}


# ---------------------------------------------------------------------------- #
# tool                                                                         #
# ---------------------------------------------------------------------------- #

del_bfd_plugin() {
    local source=$1
    local tmp_file

    if ! tmp_file="$(mktemp)"; then
        echo "$0: del_bfd_plugin: cannot create temporary file" 1>&2
        exit 1
    fi

    cat "$source" | grep -v "bfd plugin" > "$tmp_file"
    cat "$tmp_file" > "$source"
    rm -f "$tmp_file"
}


# ---------------------------------------------------------------------------- #
# main                                                                         #
# ---------------------------------------------------------------------------- #

failures=0

mkdir -p "$RESULTS_DIR"
print_banner
print_file_name "All files"

# Execute both commands
nm "$@" 2>&1 > "$OUT_NM"
ret_nm=$?

del_bfd_plugin "$OUT_NM"

"$FT_NM" "$@" > "$OUT_FT_NM"
ret_ft_nm=$?

# Add a newline after the error messages of nm and ft_nm, if any
if [ "$ret_nm" -ne 0 ] || [ "$ret_ft_nm" -ne 0 ]; then
    echo
fi

# Compare both outputs
if diff "$OUT_NM" "$OUT_FT_NM" > "$DIFF_NM"; then
    print_result "diff" "OK"
else
    print_result "diff" "FAIL"
fi

# Compare both exit codes
if [ "$ret_nm" -eq "$ret_ft_nm" ]; then
    print_result "return" "OK"
else
    print_result "return" "FAIL" "nm: $ret_nm, ft_nm: $ret_ft_nm"
fi

print_separator

# Exit status of the script: success only if nothing failed
[ "$failures" -eq 0 ]