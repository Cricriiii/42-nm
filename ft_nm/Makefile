# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/09 17:47:50 by cgajean           #+#    #+#              #
#    Updated: 2026/11/03 20:11:28 by cgajean          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


# ---------------------------------------------------------------------------- #
# compile                                                                      #
# ---------------------------------------------------------------------------- #

CC			:= cc
CC32		:= $(CC) -m32

C_STD		:= -std=c23
C_OPT		:= #-O3
C_ERR		:= -Wall -Wextra -Werror
C_DBG		:= -g3 # -fsanitize=address,undefined
C_DEP		:= -MD -MMD

C_PROF		:= #-pg

C_FLAGS		:= $(C_STD) $(C_OPT) $(C_ERR) $(C_DBG) $(C_DEP) $(C_PROF)

MAKEFLAGS	+= -j $(shell nproc)

# ---------------------------------------------------------------------------- #
# files                                                                        #
# ---------------------------------------------------------------------------- #

PROGRAM		?= ft_nm
PROGRAM32	:= $(PROGRAM)32
WORKSPACE	?= $(CURDIR)

SRC_DIR		:= $(WORKSPACE)/source
LIB_DIR		:= $(WORKSPACE)/libs/source
OBJ_DIR		:= $(WORKSPACE)/.object
OBJ_DIR_x64	:= $(OBJ_DIR)/x64
OBJ_DIR_x86	:= $(OBJ_DIR)/x86

FILES_C		:= $(shell find $(SRC_DIR) -type f -name '*.c')
FILES_O_x64	:= $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR_x64)/%.o, $(FILES_C))
FILES_O_x86	:= $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR_x86)/%.o, $(FILES_C))

LIBS_C		:= $(shell find $(LIB_DIR) -type f -name '*.c')
FILES_O_x64	+= $(patsubst $(LIB_DIR)/%.c, $(OBJ_DIR_x64)/libs/%.o, $(LIBS_C))
FILES_O_x86	+= $(patsubst $(LIB_DIR)/%.c, $(OBJ_DIR_x86)/libs/%.o, $(LIBS_C))

INCLUDES	:= $(WORKSPACE)/include
INCLUDES	+= $(WORKSPACE)/libs/include
INCLUDES	:= $(INCLUDES:%=-I %)

# ---------------------------------------------------------------------------- #
# build                                                                        #
# ---------------------------------------------------------------------------- #

## Build the 64-bit program (ft_nm)
all:
	+$(MAKE) $(PROGRAM)

$(PROGRAM): $(FILES_O_x64)
	$(CC) $(C_FLAGS) $(INCLUDES) $^ -o $@

$(OBJ_DIR_x64)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR_x64)/libs/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) $(INCLUDES) -c $< -o $@

## Build the 32-bit program (ft_nm32)
x86:
	+$(MAKE) $(PROGRAM32)
	
$(PROGRAM32): $(FILES_O_x86)
	$(CC32) $(C_FLAGS) $(INCLUDES) $^ -o $@

$(OBJ_DIR_x86)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC32) $(C_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR_x86)/libs/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	$(CC32) $(C_FLAGS) $(INCLUDES) -c $< -o $@

-include $(FILES_O_x64:.o=.d)
-include $(FILES_O_x86:.o=.d)

## Rebuild from scratch (fclean, then all)
re:
	+$(MAKE) fclean
	+$(MAKE) all

# ---------------------------------------------------------------------------- #
# doxygen                                                                      #
# ---------------------------------------------------------------------------- #

## Generate the Doxygen documentation in .docs/html (fails on any warning)
doc:
	@doxygen Doxyfile; \
	if [ $$? -eq 0 ]; then \
		echo "Documentation created. See in $(WORKSPACE)/.docs"; \
	else \
		echo "Documentation creation failed"; \
	fi

## Open documentation in web browser
docopen:
	@if [ ! -d $(WORKSPACE)/.docs ]; then \
		make doc; \
	fi; \
	xdg-open .docs/html/index.html > /dev/null 2>&1; \
	if [ $$? -ne 0 ]; then \
		echo "Documentation can't be opened. Try fixing it."; \
	else \
		echo "Documentation is opened, check your web browser."; \
	fi

## Delete the generated documentation
docclean:
	@rm -rf $(WORKSPACE)/.docs

## Recreate documentation
docre:
	@+$(MAKE) docclean
	@+$(MAKE) doc

## Display Doxygen configutation
docconf:
	@doxygen -x Doxyfile

## Display Doxygen help
dochelp:
	@doxygen -g - | less


# ---------------------------------------------------------------------------- #
# test                                                                         #
# ---------------------------------------------------------------------------- #

TESTS_DIR	:= $(WORKSPACE)/tests
RESULTS_DIR	:= $(WORKSPACE)/.test_results

T_ARGS		?=
T_PROG		?=
# Newlines (e.g. from `find`) would split the recipe into separate commands
override T_PROG	:= $(strip $(T_PROG))

TEST_ENV	:= RESULTS_DIR=$(RESULTS_DIR) FT_NM=$(WORKSPACE)/$(PROGRAM)

## Compare ft_nm and nm output on all T_PROG at once
test_all: $(PROGRAM)
	@$(TEST_ENV) $(TESTS_DIR)/$@.sh $(T_ARGS) $(T_PROG)

## Same as test_all, plus a Valgrind check of ft_nm (leaks, fds)
test_all_valgrind: $(PROGRAM)
	@$(TEST_ENV) $(TESTS_DIR)/$@.sh $(T_ARGS) $(T_PROG)

## Like test_all, but one file at a time (per-file verdict)
test_each: $(PROGRAM)
	@$(TEST_ENV) $(TESTS_DIR)/$@.sh $(T_ARGS) -- $(T_PROG)

## Like test_each, plus Valgrind checks per file (leaks, fds)
test_each_valgrind: $(PROGRAM)
	@$(TEST_ENV) $(TESTS_DIR)/$@.sh $(T_ARGS) -- $(T_PROG)

## Delete the test output files
test_clean:
	@rm -rf $(RESULTS_DIR)

## Like test_each, on the non stripped files of T_PROG dirs (default: /lib64)
test_non_stripped: $(PROGRAM)
	@$(TEST_ENV) $(TESTS_DIR)/$@.sh $(T_ARGS) -- $(T_PROG)

## Like test_non_stripped, plus Valgrind checks per file (leaks, fds)
test_non_stripped_valgrind: $(PROGRAM)
	@$(TEST_ENV) $(TESTS_DIR)/$@.sh $(T_ARGS) -- $(T_PROG)

# ---------------------------------------------------------------------------- #
# clean                                                                        #
# ---------------------------------------------------------------------------- #

## Delete the .object directory
clean:
	@rm -rf $(OBJ_DIR)

## Delete .object and both executables (ft_nm, ft_nm32)
fclean:
	+$(MAKE) clean
	@rm -f $(WORKSPACE)/$(PROGRAM) $(WORKSPACE)/$(PROGRAM32)

## Same as clean, plus delete documentation and test files
reset:
	+$(MAKE) fclean
	+$(MAKE) docclean
	+$(MAKE) test_clean


# ---------------------------------------------------------------------------- #
# misc                                                                         #
# ---------------------------------------------------------------------------- #

## Format all .c and .h files with clang-format
format:
	@find . -name "*.c" -o -name "*.h" | xargs clang-format -i

# Provided by https://gitlab.com/depressiveRobot/make-help/blob/master/help.mk
## Show this help
help:
	@printf "\nAvailable targets:\n"
	@awk '/^[a-zA-Z\-_0-9]+:/ { \
		helpMessage = match(lastLine, /^## (.*)/); \
		if (helpMessage) { \
			helpCommand = substr($$1, 0, index($$1, ":")-1); \
			helpMessage = substr(lastLine, RSTART + 3, RLENGTH); \
			printf "  %-20s %s\n", helpCommand, helpMessage; \
		} \
	} \
	{ lastLine = $$0 }' $(MAKEFILE_LIST)
	@echo

.PHONY: all \
		x86 \
		re \
		prof \
		doc \
		docopen \
		docclean \
		docre \
		docconf \
		dochelp \
		test_all \
		test_all_valgrind \
		test_each \
		test_each_valgrind \
		test_non_stripped \
		test_non_stripped_valgrind \
		test_clean \
		clean \
		fclean \
		reset \
		format \
		help