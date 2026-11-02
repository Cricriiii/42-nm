# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/09 18:35:42 by cgajean           #+#    #+#              #
#    Updated: 2026/11/02 11:23:19 by cgajean          ###   ########.fr        #
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

## Build on the host machine
local:
	+$(MAKE) -C $(WORKSPACE_LOCAL) all

## re docker
re:
	
## re local
lre:
	+$(MAKE) -C $(WORKSPACE_LOCAL) re


# ---------------------------------------------------------------------------- #
# test                                                                         #
# ---------------------------------------------------------------------------- #

T_PROG		?=
T_ARGS		?=
VALGRIND_	:= valgrind \
				--quiet \
				--leak-check=full \
				--track-fds=yes \
				--error-exitcode=42 \
				--log-file=$(TEST_DIR)/valgrind.txt

## Test ft_nm ouput against the real one
test:
	@mkdir -p $(TEST_DIR)
	@for file in $(T_PROG); do \
		nm $(T_ARGS) $$file > $(TEST_DIR)/out_nm.txt || true; \
		$(WORKSPACE_LOCAL)/$(PROGRAM) $(T_ARGS) $$file > $(TEST_DIR)/out_ft_nm.txt || true; \
		if diff $(TEST_DIR)/out_nm.txt $(TEST_DIR)/out_ft_nm.txt > $(TEST_DIR)/diff_nm; then \
			echo "$$file: diff OK"; \
		else \
			echo "$$file: diff FAIL"; \
		fi; \
	done	

## Equivalent to test with additional Valgrind checks
test_valgrind:
	@mkdir -p $(TEST_DIR)
	@for file in $(T_PROG); do \
		nm $(T_ARGS) $$file > $(TEST_DIR)/out_nm.txt || true; \
		$(VALGRIND_) $(WORKSPACE_LOCAL)/$(PROGRAM) $(T_ARGS) $$file > $(TEST_DIR)/out_ft_nm.txt; \
		ret=$$?; \
		if [ $$ret -eq 42 ]; then \
			echo "$$file: valgrind FAIL (see $(TEST_DIR)/valgrind.txt)"; \
		else \
			echo "$$file: valgrind OK"; \
		fi; \
		if diff $(TEST_DIR)/out_nm.txt $(TEST_DIR)/out_ft_nm.txt > $(TEST_DIR)/diff_nm; then \
			echo "$$file: diff OK"; \
		else \
			echo "$$file: diff FAIL"; \
		fi; \
		echo; \
	done

# ---------------------------------------------------------------------------- #
# clean                                                                        #
# ---------------------------------------------------------------------------- #

## clean
clean:
	+$(MAKE) lclean

## fclean
fclean:
	@rm -rf $(TEST_DIR)
	+$(MAKE) lfclean


## local clean
lclean:
	+$(MAKE) -C $(WORKSPACE_LOCAL) clean

## local fclean
lfclean:
	+$(MAKE) -C $(WORKSPACE_LOCAL) fclean

## reset
reset:

## local reset
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

.PHONY: all run test local re lre clean lclean fclean lfclean reset lreset format help