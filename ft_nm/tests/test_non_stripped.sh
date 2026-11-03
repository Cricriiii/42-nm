#!/bin/bash
# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    test_non_stripped.sh                               :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/03 20:12:46 by cgajean           #+#    #+#              #
#    Updated: 2026/11/03 20:12:47 by cgajean          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Usage: FT_NM=<binary> [RESULTS_DIR=<dir>] \
#            test_non_stripped.sh [nm options] [-- directories...]
#
# Run test_each.sh on the non stripped shared libraries (*.so.*) and object
# files (*.o) found in the given directories (not recursive).
#
# Arguments:
#   directories Directories to search (default: /lib64)
#
# Environment:
#   FT_NM       Path to the ft_nm binary (required)
#   RESULTS_DIR Directory of the result files (default: .test_results/, at
#               the root of the project)

set -uo pipefail

# ---------------------------------------------------------------------------- #
# settings                                                                     #
# ---------------------------------------------------------------------------- #

TEST="$(dirname "$0")/test_each.sh"
DEFAULT_DIRS=(/lib64)

# ---------------------------------------------------------------------------- #
# arguments                                                                    #
# ---------------------------------------------------------------------------- #

# Split the arguments: nm options before "--", directories after
opts=()
while [ $# -gt 0 ] && [ "$1" != "--" ]; do
    opts+=("$1")
    shift
done
if [ $# -gt 0 ]; then
    shift   # drop the "--"
fi

dirs=("$@")
if [ ${#dirs[@]} -eq 0 ]; then
    dirs=("${DEFAULT_DIRS[@]}")
fi

# ---------------------------------------------------------------------------- #
# main                                                                         #
# ---------------------------------------------------------------------------- #

# Keep the files that file(1) reports as "not stripped" (they have a .symtab)
mapfile -t files < <(
    find -L "${dirs[@]}" -maxdepth 1 -type f \
        \( -name '*.so.*' -o -name '*.o' \) -exec file {} + \
        | grep 'not stripped' | cut -d: -f1
)

if [ ${#files[@]} -eq 0 ]; then
    echo "$0: no non stripped file found in: ${dirs[*]}" >&2
    exit 1
fi

# Hand over to test_each.sh: its exit status becomes the one of this script
exec "$TEST" "${opts[@]}" -- "${files[@]}"
