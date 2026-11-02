# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/09 18:35:42 by cgajean           #+#    #+#              #
#    Updated: 2026/11/02 16:50:21 by cgajean          ###   ########.fr        #
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

## Build on the host machine (make all in the local workspace)
local:
	+$(MAKE) -C $(WORKSPACE_LOCAL) all

## Rebuild the dockerized environment (TODO)
re:
	
## Rebuild in the local workspace
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

## Compare ft_nm and nm output on all T_PROG at once
test_all:
	@mkdir -p $(TEST_DIR)
	@nm $(T_ARGS) $(T_PROG) > $(TEST_DIR)/out_nm.txt || true;
	@$(WORKSPACE_LOCAL)/$(PROGRAM) $(T_ARGS) $(T_PROG) > $(TEST_DIR)/out_ft_nm.txt || true;
	@if diff $(TEST_DIR)/out_nm.txt $(TEST_DIR)/out_ft_nm.txt > $(TEST_DIR)/diff_nm; then \
		echo "diff OK"; \
	else \
		echo "diff FAIL"; \
	fi; \

## Same as test_all, plus a Valgrind check per file (leaks, fds)
test_all_valgrind:
	@mkdir -p $(TEST_DIR)
	@nm $(T_ARGS) $(T_PROG) > $(TEST_DIR)/out_nm.txt || true;
	@$(VALGRIND_) $(WORKSPACE_LOCAL)/$(PROGRAM) $(T_ARGS) $(T_PROG) > $(TEST_DIR)/out_ft_nm.txt; \
	ret=$$?; \
	if [ $$ret -eq 42 ]; then \
		echo "valgrind: FAIL (see $(TEST_DIR)/valgrind.txt)"; \
	else \
		echo "valgrind: OK"; \
	fi; \
	if diff $(TEST_DIR)/out_nm.txt $(TEST_DIR)/out_ft_nm.txt > $(TEST_DIR)/diff_nm; then \
		echo "diff: OK"; \
	else \
		echo "diff: FAIL"; \
	fi; \

## Like test_all, but one file at a time (per-file verdict)
test_each:
	@mkdir -p $(TEST_DIR)
	@for file in $(T_PROG); do \
		if [ -d $$file ]; then \
			continue; \
		fi; \
		echo "*************************************************************\n"; \
		nm $(T_ARGS) $$file > $(TEST_DIR)/out_nm.txt; \
		ret_nm=$$?; \
		$(WORKSPACE_LOCAL)/$(PROGRAM) $(T_ARGS) $$file > $(TEST_DIR)/out_ft_nm.txt; \
		ret_ft_nm=$$?; \
		echo; \
		if diff $(TEST_DIR)/out_nm.txt $(TEST_DIR)/out_ft_nm.txt > $(TEST_DIR)/diff_nm; then \
			echo "$$file: diff OK"; \
		else \
			echo "$$file: diff FAIL"; \
		fi; \
		if [ $$ret_nm -eq $$ret_ft_nm ]; then \
			echo "$$file: return OK"; \
		else \
			echo "$$file: return FAIL"; \
		fi; \
		echo; \
	done; \
	echo "*************************************************************\n"


## Like test_each, plus Valgrind checks per file (leaks, fds)
test_each_valgrind:
	@mkdir -p $(TEST_DIR)
	@for file in $(T_PROG); do \
		if [ -d $$file ]; then \
			continue; \
		fi; \
		echo "*************************************************************\n"; \
		nm $(T_ARGS) $$file > $(TEST_DIR)/out_nm.txt; \
		$(VALGRIND_) $(WORKSPACE_LOCAL)/$(PROGRAM) $(T_ARGS) $$file > $(TEST_DIR)/out_ft_nm.txt; \
		ret_nm=$$?; \
		ret_ft_nm=$$?; \
		echo; \
		if [ $$ret_ft_nm -eq 42 ]; then \
			echo "\tvalgrind: FAIL (see $(TEST_DIR)/valgrind.txt)"; \
		else \
			echo "\tvalgrind: OK"; \
		fi; \
		if diff $(TEST_DIR)/out_nm.txt $(TEST_DIR)/out_ft_nm.txt > $(TEST_DIR)/diff_nm; then \
			echo "\tdiff    : OK"; \
		else \
			echo "\tdiff    : FAIL"; \
		fi; \
		if [ $$ret_nm -eq $$ret_ft_nm ]; then \
			echo "\treturn  : OK"; \
		else \
			echo "\treturn  : return FAIL"; \
		fi; \
		echo; \
	done ; \
	echo "*************************************************************\n"


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


## Clean object files in the local workspace
lclean:
	+$(MAKE) -C $(WORKSPACE_LOCAL) clean

## Full clean in the local workspace
lfclean:
	+$(MAKE) -C $(WORKSPACE_LOCAL) fclean

## Reset the dockerized environment (TODO)
reset:


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

.PHONY: all run local re lre test test_valgrind test_each test_each_valgrind clean lclean fclean lfclean reset lreset format help