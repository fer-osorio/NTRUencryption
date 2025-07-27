# Source Makefile - builds the main library
include ../config.mk
include ../mk/common.mk

# Find all sources files inside this ( . ) directory, assign object files
SOURCES = $(call find_sources,.,cpp)
OBJECTS = (call sources_to_objects,$(SOURCES),$(BUILD_DIR)/src)
DEPS = $(call objects_to_deps,$(OBJECTS))

STATIC_LIB = $(BUILD_DIR)/lib/libntru.a
SHARED_LIB = $(BUILD_DIR)/lib/libntru.$(SHARED_EXT)
