# examples/Makefile - Build example programs
include ../config.mk
include ../mk/common.mk

# Library dependency
BUILD_DIR_PATH = ../$(BUILD_DIR)
NTRU_LIB = $(BUILD_DIR_PATH)/lib/libntru.a

# Find all example sources
BASIC_SOURCES = $(call find_sources,basic-usage,cpp)
FILE_SOURCES = $(call find_sources,file-encryption,cpp)
ALL_SOURCES = $(BASIC_SOURCES) $(FILE_SOURCES)

ALL_OBJECTS = $(call sources_to_objects,$(ALL_SOURCES),$(BUILD_DIR_PATH)/examples)
$(info ALL_OBJECTS: $(ALL_OBJECTS))                                                     # Debugging purposes
DEPS = $(call objects_to_deps,$(ALL_OBJECTS))

# Example executables (remove directory prefix and .cpp extension)
BASIC_EXAMPLES = $(patsubst basic-usage/%.cpp,$(BUILD_DIR_PATH)/bin/basic-usage/%,$(BASIC_SOURCES))
FILE_EXAMPLES = $(patsubst file-encryption/%.cpp,$(BUILD_DIR_PATH)/bin/file-encryption/%,$(FILE_SOURCES))

ALL_EXAMPLES = $(BASIC_EXAMPLES) $(FILE_EXAMPLES)

.PHONY: all basic file

all: $(ALL_EXAMPLES)
basic: $(BASIC_EXAMPLES)
file: $(FILE_EXAMPLES)

# Build rules for different example categories
$(BUILD_DIR_PATH)/bin/basic-usage/%: $(BUILD_DIR_PATH)/examples/obj/basic-usage/%.o $(NTRU_LIB)
	$(call create_executable,$@,$^)

$(BUILD_DIR_PATH)/bin/file-encryption/%: $(BUILD_DIR_PATH)/examples/obj/file-encryption/%.o $(NTRU_LIB)
	$(call create_executable,$@,$^)

# Generic compilation rule for example files
$(BUILD_DIR_PATH)/examples/obj/%.o: %.cpp
	$(call compile_cpp,$<,$@,-I../$(INCLUDE_DIR))

clean:
	$(call standard_clean,$(BUILD_DIR_PATH)/bin/basic-usage/* $(BUILD_DIR_PATH)/bin/file-encryption/* $(BUILD_DIR_PATH)/examples)

# Include dependency files
-include $(DEPS)
