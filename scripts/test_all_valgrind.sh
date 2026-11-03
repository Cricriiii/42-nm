#!/bin/bash
# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_all_valgrind.sh                               :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/03 11:48:55 by cgajean           #+#    #+#              #
#    Updated: 2026/11/03 11:48:58 by cgajean          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Usage: FT_NM=<binary> [TEST_DIR=<dir>] \
#            test_all_valgrind.sh [nm options] [files...]
#
# Same as test_all.sh, with ft_nm run under Valgrind (leaks, file descriptors).
#
# Environment:
#   FT_NM       Path to the ft_nm binary (required)
#   TEST_DIR    Directory of the output files (default: .test)

set -uo pipefail

# ---------------------------------------------------------------------------- #
# settings                                                                     #
# ---------------------------------------------------------------------------- #

: "${FT_NM:?FT_NM is not set}"
: "${TEST_DIR:=.test}"

OUT_NM="$TEST_DIR/out_nm.txt"
OUT_FT_NM="$TEST_DIR/out_ft_nm.txt"
DIFF_NM="$TEST_DIR/diff_nm.txt"

VALGRIND_ERR_CODE=42
VALGRIND_LOG="$TEST_DIR/valgrind.txt"
VALGRIND=(
    valgrind
    --quiet
    --leak-check=full
    --track-fds=yes
    "--error-exitcode=$VALGRIND_ERR_CODE"
    "--log-file=$VALGRIND_LOG"
)

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
# main                                                                         #
# ---------------------------------------------------------------------------- #

failures=0

mkdir -p "$TEST_DIR"
print_banner
print_file_name "All files"

# Execute both commands
nm "$@" > "$OUT_NM"
ret_nm=$?
"${VALGRIND[@]}" "$FT_NM" "$@" > "$OUT_FT_NM"
ret_ft_nm=$?

# Add a newline after the error messages of nm and ft_nm, if any
if [ "$ret_nm" -ne 0 ] || [ "$ret_ft_nm" -ne 0 ]; then
    echo
fi

# Compare both outputs
if diff "$OUT_NM" "$OUT_FT_NM" > "$DIFF_NM"; then
    print_result "diff" "OK"
else
    print_result "diff" "FAIL" "see $DIFF_NM"
fi

# Check the Valgrind report, then the exit codes: the real exit code of ft_nm
# is hidden when Valgrind reports an error
if [ "$ret_ft_nm" -eq "$VALGRIND_ERR_CODE" ]; then
    print_result "valgrind" "FAIL" "see $VALGRIND_LOG"
else
    print_result "valgrind" "OK"
    if [ "$ret_nm" -eq "$ret_ft_nm" ]; then
        print_result "return" "OK"
    else
        print_result "return" "FAIL" "nm: $ret_nm, ft_nm: $ret_ft_nm"
    fi
fi

print_separator

# Exit status of the script: success only if nothing failed
[ "$failures" -eq 0 ]
