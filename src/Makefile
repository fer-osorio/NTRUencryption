# Source Makefile - builds the main library
include ../config.mk
include ../mk/common.mk

# Find all sources files inside this ( . ) directory, assign object files
SOURCES = $(call find_sources,.,cpp)
OBJECTS = $(call sources_to_objects,$(SOURCES),$(BUILD_DIR)/src)
DEPS = $(call objects_to_deps,$(OBJECTS))

STATIC_LIB = $(BUILD_DIR)/lib/libntru.a
SHARED_LIB = $(BUILD_DIR)/lib/libntru.$(SHARED_EXT)

.PHONY all clean deps-check

deps-check:
	$(call check_dependencies)

$(STATIC_LIB): $(OBJECTS)
	$(call create_static_lib,$@,$^) # Expansion -> $(call create_static_lib,$(BUILD_DIR)/lib/libntru.a,$(call sources_to_objects,$(SOURCES),$(BUILD_DIR)/src)). $@ Target ;$^ All dependencies

$(SHARED_LIB): $(OBJECTS)
	$(call create_shared_lib,$@,$^) # Expansion -> $(call create_shared_lib,$(BUILD_DIR)/lib/libntru.$(SHARED_EXT),$(call sources_to_objects,$(SOURCES),$(BUILD_DIR)/src)). $@ Target ;$^ All dependencies

# Generic rule for compiling C++ files
$(BUILD_DIR)/src/obj/%.o: %.cpp
	$(call compile_cpp,$<,$@,-fPIC)

clean:
	$(call standard_clean,$(BUILD_DIR)/lib $(BUILD_DIR)/src)

# Include dependency files for automatic recompilation
-include $(DEPS) # Include directive with no error or warning message
