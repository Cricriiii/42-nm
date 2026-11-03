# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/09 18:35:42 by cgajean           #+#    #+#              #
#    Updated: 2026/11/03 15:40:08 by cgajean          ###   ########.fr        #
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

# Doc target variable
T_DOC	= +$(MAKE) -C $(WORKSPACE_LOCAL) $@

## Generate the Doxygen documentation (local workspace)
doc:
	$(T_DOC)

## Open documentation in web browser (local workspace)
docopen:
	$(T_DOC)

## Delete the generated documentation (local workspace)
docclean:
	$(T_DOC)

## Recreate documentation (local workspace)
docre:
	$(T_DOC)

## Display Doxygen configuration (local workspace)
docconf:
	$(T_DOC)

## Display Doxygen help (local workspace)
dochelp:
	$(T_DOC)

# ---------------------------------------------------------------------------- #
# test                                                                         #
# ---------------------------------------------------------------------------- #

# T_ARGS reaches the sub-make by itself. T_PROG is made absolute, since the
# sub-make runs from the local workspace.
T_DELEGATE	= +$(MAKE) -C $(WORKSPACE_LOCAL) $@ T_PROG="$(abspath $(T_PROG))"

## Compare ft_nm and nm output on all T_PROG at once (local workspace)
test_all:
	$(T_DELEGATE)

## Same as test_all, plus a Valgrind check of ft_nm (local workspace)
test_all_valgrind:
	$(T_DELEGATE)

## Like test_all, but one file at a time (local workspace)
test_each:
	$(T_DELEGATE)

## Like test_each, plus Valgrind checks per file (local workspace)
test_each_valgrind:
	$(T_DELEGATE)

## Delete the test output files (local workspace)
test_clean:
	+$(MAKE) -C $(WORKSPACE_LOCAL) test_clean

# ---------------------------------------------------------------------------- #
# clean                                                                        #
# ---------------------------------------------------------------------------- #

## Clean object files (local build)
clean:
	+$(MAKE) lclean

## Clean objects, binary and test files
fclean:
	+$(MAKE) lfclean
	+$(MAKE) test_clean

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
		test_all \
		test_all_valgrind \
		test_each \
		test_each_valgrind \
		test_clean \
		clean \
		lclean \
		fclean \
		reset \
		lfclean \
		lreset \
		format \
		help