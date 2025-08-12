# Source Makefile - builds the main library
include ../config.mk
include ../mk/common.mk

UP = ../
BUILD_DIR_PATH = $(UP)$(BUILD_DIR)

# Find all sources files inside this ( . ) directory, assign object files
SOURCES = $(call find_sources,.,cpp)
#$(info SOURCES: $(SOURCES))                                                     # Debugging purposes
CLEAN_SOURCES = $(call clean_sources,$(SOURCES))                                # Remove the leading ./
#$(info CLEAN_SOURCES: $(CLEAN_SOURCES))                                         # Debugging purposes
OBJECTS = $(call sources_to_objects,$(CLEAN_SOURCES),$(BUILD_DIR_PATH)/src)     # This would create build/src/obj/core/encryption.o, build/src/obj/math/R2Polynomial.o, etc.
#$(info OBJECTS: $(OBJECTS))                                                     # Debugging purposes
DEPS = $(call objects_to_deps,$(OBJECTS))

STATIC_LIB = $(BUILD_DIR_PATH)/lib/libntru.a
#SHARED_LIB = $(BUILD_DIR_PATH)/lib/libntru.so

ifeq ($(INCLUDE_GMPXX),true)
GMPXX_DEPS_CHECK = check-gmpxx
else
GMPXX_DEPS_CHECK =
endif

.PHONY: all clean deps-check $(GMPXX_DEPS_CHECK)

all: deps-check $(GMPXX_DEPS_CHECK) $(STATIC_LIB)

deps-check:
	$(call check_dependencies)

# Conditional dependency checking
check-gmpxx:
ifeq ($(INCLUDE_GMPXX),true)
	$(call check_gmpxx_deps)
endif

# Expansion -> $(call create_static_lib,$(BUILD_DIR_PATH)/lib/libntru.a,$(call sources_to_objects,$(SOURCES),$(BUILD_DIR_PATH)/src)). $@ Target ;$^ All dependencies
$(STATIC_LIB): $(OBJECTS)
	$(call create_static_lib,$@,$^)

# Expansion -> $(call create_shared_lib,$(BUILD_DIR_PATH)/lib/libntru.so,$(call sources_to_objects,$(SOURCES),$(BUILD_DIR_PATH)/src)). $@ Target ;$^ All dependencies
#$(SHARED_LIB): $(OBJECTS)
#	$(call create_shared_lib,$@,$^)

# Generic rule for compiling C++ files
# -fPIC (Position-Independent Code), thougth for shared libraries
# Compiling ntru files with NTRULIB_FLAGS
$(BUILD_DIR_PATH)/src/obj/ntru/%.o: ntru/%.cpp
	$(call compile_cpp,$<,$@,-fPIC $(NTRULIB_FLAGS))
# The rest of the object files do not need NTRULIB_FLAGS
$(BUILD_DIR_PATH)/src/obj/metrics-analysis/%.o: metrics-analysis/%.cpp
	$(call compile_cpp,$<,$@,-fPIC)
$(BUILD_DIR_PATH)/src/obj/print_debug/%.o: print_debug/%.cpp
	$(call compile_cpp,$<,$@,-fPIC)

clean:
	$(call standard_clean,$(BUILD_DIR_PATH)/lib $(BUILD_DIR_PATH)/src)

# Include dependency files for automatic recompilation
-include $(DEPS) # Include directive with no error or warning message
