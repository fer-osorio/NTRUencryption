# Source Makefile - builds the main library
include ../config.mk
include ../mk/common.mk

UP = ../
BUILD_DIR_PATH = $(UP)$(BUILD_DIR)

# Find all sources files inside this ( . ) directory, assign object files
SOURCES = $(shell find . -name "*.cpp" 2>/dev/null)                             # "./*.cpp" #$(call find_sources,.,cpp)
CLEAN_SOURCES = $(patsubst ./%,%,$(SOURCES))                                    # Remove the leading ./
OBJECTS = $(patsubst %.cpp,$(BUILD_DIR_PATH)/src/obj/%.o,$(CLEAN_SOURCES))      # This would create build/src/obj/core/encryption.o, build/src/obj/math/R2Polynomial.o, etc.
$(info SOURCES: $(SOURCES))                                                     # Debugging purposes
$(info CLEAN_SOURCES: $(CLEAN_SOURCES))                                         # Debugging purposes
$(info OBJECTS: $(OBJECTS))                                                     # Debugging purposes
#$(call sources_to_objects,$(CLEAN_SOURCES),$(BUILD_DIR_PATH)/src)
DEPS = $(call objects_to_deps,$(OBJECTS))

STATIC_LIB = $(BUILD_DIR_PATH)/lib/libntru.a
#SHARED_LIB = $(BUILD_DIR_PATH)/lib/libntru.so

.PHONY: all clean deps-check

all: deps-check $(STATIC_LIB)# $(SHARED_LIB)

deps-check:
	$(call check_dependencies)

 # Expansion -> $(call create_static_lib,$(BUILD_DIR_PATH)/lib/libntru.a,$(call sources_to_objects,$(SOURCES),$(BUILD_DIR_PATH)/src)). $@ Target ;$^ All dependencies
$(STATIC_LIB): $(OBJECTS)
	$(call create_static_lib,$@,$^)

 # Expansion -> $(call create_shared_lib,$(BUILD_DIR_PATH)/lib/libntru.so,$(call sources_to_objects,$(SOURCES),$(BUILD_DIR_PATH)/src)). $@ Target ;$^ All dependencies
#$(SHARED_LIB): $(OBJECTS)
#	$(call create_shared_lib,$@,$^)

# Generic rule for compiling C++ files
$(BUILD_DIR_PATH)/src/obj/%.o: %.cpp
	$(call compile_cpp,$<,$@,-fPIC)

clean:
	$(call standard_clean,$(BUILD_DIR_PATH)/lib $(BUILD_DIR_PATH)/src)

# Include dependency files for automatic recompilation
-include $(DEPS) # Include directive with no error or warning message
