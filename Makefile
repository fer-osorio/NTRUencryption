# Root Makefile - orchestrates the entire build
include config.mk
include mk/common.mk

.PHONY: all clean test examples

all: lib examples test

lib:
	$(MAKE) -C src

examples: lib
	$(MAKE) -C examples

test: lib
	$(MAKE) -C tests
#	$(BUILD_DIR)/bin/run_all_tests

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	rm -rf $(BUILD_DIR)

help:
	$(call show_help)
