# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/09 18:35:42 by cgajean           #+#    #+#              #
#    Updated: 2026/11/03 14:47:41 by cgajean          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ---------------------------------------------------------------------------- #
# makefile                                                                     #
# ---------------------------------------------------------------------------- #

MAKEFLAGS	+= -j $(shell nproc)

ifndef COMPOSE
COMPOSE		:= $(shell docker compose version > /dev/null 2>&1 && echo "docker compose" || echo "podman-compose")
endif
COMPOSE_CMD	:= $(COMPOSE)


# ---------------------------------------------------------------------------- #
# compile                                                                      #
# ---------------------------------------------------------------------------- #

PROGRAM    		?= ft_nm
VERSION			?= 1.0

IMAGE_TAG		+= $(PROGRAM)_$(VERSION)

WORKSPACE_LOCAL	+= $(CURDIR)/$(PROGRAM)
WORKSPACE_DOCK	+= /$(PROGRAM)

TEST_DIR		:= $(CURDIR)/.test

SCRIPTS_DIR		:= $(CURDIR)/scripts

TARGET			?= dev

# ---------------------------------------------------------------------------- #
# build                                                                        #
# ---------------------------------------------------------------------------- #

## Build in a dockerized environment
all:
	@docker build \
		--build-arg PROGRAM=$(PROGRAM) \
		--build-arg WORKSPACE=$(WORKSPACE_DOCK) \
		--build-arg VERSION_TAG=$(VERSION) \
		--target $(TARGET) \
		-t $(PROGRAM):$(TARGET) .
	@+$(MAKE) run

## Run the dockerized environment
run:
	@mkdir -p $(WORKSPACE_LOCAL)
	@docker run \
		--rm \
		$(if $(filter dev,$(TARGET)),-v $(WORKSPACE_LOCAL):$(WORKSPACE_DOCK):Z) \
		-it $(PROGRAM):$(TARGET) \
		/bin/sh

## Build on the host machine (make all in the local workspace)
local:
	+$(MAKE) -C $(WORKSPACE_LOCAL) all

## Rebuild the dockerized environment (TODO)
re:
	
## Rebuild in the local workspace
lre:
	+$(MAKE) -C $(WORKSPACE_LOCAL) re


# ---------------------------------------------------------------------------- #
# doxygen                                                                      #
# ---------------------------------------------------------------------------- #

## Generate the Doxygen documentation (local workspace)
doc:
	+$(MAKE) -C $(WORKSPACE_LOCAL) doc

## Open documentation in web browser (local workspace)
docopen:
	+$(MAKE) -C $(WORKSPACE_LOCAL) docopen

## Delete the generated documentation (local workspace)
docclean:
	+$(MAKE) -C $(WORKSPACE_LOCAL) docclean

## Recreate documentation (local workspace)
docre:
	+$(MAKE) -C $(WORKSPACE_LOCAL) docre

## Display Doxygen configuration (local workspace)
docconf:
	+$(MAKE) -C $(WORKSPACE_LOCAL) docconf

## Display Doxygen help (local workspace)
dochelp:
	+$(MAKE) -C $(WORKSPACE_LOCAL) dochelp


# ---------------------------------------------------------------------------- #
# test                                                                         #
# ---------------------------------------------------------------------------- #

## Compare ft_nm and nm output on all T_PROG at once
test_all:
	@TEST_DIR=$(TEST_DIR) FT_NM=$(WORKSPACE_LOCAL)/$(PROGRAM) \
			$(SCRIPTS_DIR)/$@.sh $(T_ARGS) $(T_PROG)

## Same as test_all, plus a Valgrind check per file (leaks, fds)
test_all_valgrind:
	@TEST_DIR=$(TEST_DIR) FT_NM=$(WORKSPACE_LOCAL)/$(PROGRAM) \
			$(SCRIPTS_DIR)/$@.sh $(T_ARGS) $(T_PROG)

## Like test_all, but one file at a time (per-file verdict)
test_each:
	@TEST_DIR=$(TEST_DIR) FT_NM=$(WORKSPACE_LOCAL)/$(PROGRAM) \
			$(SCRIPTS_DIR)/$@.sh $(T_ARGS) -- $(T_PROG)


## Like test_each, plus Valgrind checks per file (leaks, fds)
test_each_valgrind:
	@TEST_DIR=$(TEST_DIR) FT_NM=$(WORKSPACE_LOCAL)/$(PROGRAM) \
			$(SCRIPTS_DIR)/$@.sh $(T_ARGS) -- $(T_PROG)

# ---------------------------------------------------------------------------- #
# clean                                                                        #
# ---------------------------------------------------------------------------- #

## Clean object files (local build)
clean:
	+$(MAKE) lclean

## Clean objects, binary and test files
fclean:
	@rm -rf $(TEST_DIR)
	+$(MAKE) lfclean

## Reset all
reset:
	+$(MAKE) lreset

## Clean object files in the local workspace
lclean:
	+$(MAKE) -C $(WORKSPACE_LOCAL) clean

## Full clean in the local workspace
lfclean:
	+$(MAKE) -C $(WORKSPACE_LOCAL) fclean

## Reset the local workspace (clean objects and documentation)
lreset:
	+$(MAKE) -C $(WORKSPACE_LOCAL) reset



# ---------------------------------------------------------------------------- #
# misc                                                                         #
# ---------------------------------------------------------------------------- #

## Format files
format:
	+$(MAKE) -C $(WORKSPACE_LOCAL) format

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
		run \
		local \
		re \
		lre \
		doc \
		docopen \
		docclean \
		docre \
		docconf \
		dochelp \
		test \
		test_valgrind \
		test_each \
		test_each_valgrind \
		clean \
		lclean \
		fclean \
		lfclean \
		lreset \
		format \
		help