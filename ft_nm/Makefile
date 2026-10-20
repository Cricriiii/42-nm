# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/09 17:47:50 by cgajean           #+#    #+#              #
#    Updated: 2026/10/20 17:33:02 by cgajean          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


# ---------------------------------------------------------------------------- #
# compile                                                                      #
# ---------------------------------------------------------------------------- #

CC			:= cc

C_STD		:= -std=c23
C_ERR		:= -Wall -Wextra -Werror
C_DBG		:= -g3
C_DEP		:= -MD -MMD

C_FLAGS		:= $(C_STD) $(C_ERR) $(C_DBG) $(C_DEP)

MAKEFLAGS	+= -j $(shell nproc)

# ---------------------------------------------------------------------------- #
# files                                                                        #
# ---------------------------------------------------------------------------- #

PROGRAM		?= ft_nm
WORKSPACE	?= $(CURDIR)

SRC_DIR		:= $(WORKSPACE)/source
LIB_DIR		:= $(WORKSPACE)/libs/source
OBJ_DIR		:= $(WORKSPACE)/.object

FILES_C		:= $(shell find $(SRC_DIR) -type f -name '*.c')
FILES_O		:= $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(FILES_C))

LIBS_C		:= $(shell find $(LIB_DIR) -type f -name '*.c')
FILES_O		+= $(patsubst $(LIB_DIR)/%.c, $(OBJ_DIR)/libs/%.o, $(LIBS_C))

INCLUDES	:= $(WORKSPACE)/include
INCLUDES	+= $(WORKSPACE)/libs/include
INCLUDES	:= $(INCLUDES:%=-I %)

# ---------------------------------------------------------------------------- #
# build                                                                        #
# ---------------------------------------------------------------------------- #

## Build the program
all:
	+$(MAKE) $(PROGRAM)

$(PROGRAM): $(FILES_O)
	$(CC) $(C_FLAGS) $(INCLUDES) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/libs/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(C_FLAGS) $(INCLUDES) -c $< -o $@

-include $(FILES_O:.o=.d)

## Call 'fclean' then 'all'
re:
	+$(MAKE) fclean
	+$(MAKE) all


# ---------------------------------------------------------------------------- #
# clean                                                                        #
# ---------------------------------------------------------------------------- #

## Delete .object directory
clean:
	@rm -rf $(OBJ_DIR)

# Call 'clean' and delete executable
fclean:
	+$(MAKE) clean
	@rm -f $(WORKSPACE)/$(PROGRAM)

# ---------------------------------------------------------------------------- #
# misc                                                                         #
# ---------------------------------------------------------------------------- #

## Format files
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

.PHONY: all re clean fclean format help