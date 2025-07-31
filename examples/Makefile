# examples/Makefile - Build example programs
include ../config.mk
include ../mk/common.mk

# Library dependency
BUILD_DIR_PATH = ../$(BUILD_DIR)
NTRU_LIB = $(BUILD_DIR_PATH)/lib/libntru.a

# Find all example sources
BASIC_SOURCES = basic_encryption.cpp
#$(call find_sources,basic_usage,cpp)
FILE_SOURCES = file_encryptor.cpp
#$(call find_sources,file_encryption,cpp)

# Example executables (remove directory prefix and .cpp extension)
#$(patsubst basic_usage/%.cpp,$(BUILD_DIR_PATH)/bin/example_%,$(BASIC_SOURCES))
#$(patsubst file_encryption/%.cpp,$(BUILD_DIR_PATH)/bin/file_%,$(FILE_SOURCES)))
BASIC_EXAMPLES = $(BUILD_DIR_PATH)/bin/basic_encryption
FILE_EXAMPLES = $(BUILD_DIR_PATH)/bin/file_encryptor

ALL_EXAMPLES = $(BASIC_EXAMPLES) $(FILE_EXAMPLES)

.PHONY: all basic file

all: $(ALL_EXAMPLES)

basic: $(BASIC_EXAMPLES)
file: $(FILE_EXAMPLES)

# Build rules for different example categories
$(BUILD_DIR_PATH)/bin/basic_encryption: $(BUILD_DIR_PATH)/examples/obj/basic_encryption.o $(NTRU_LIB)
	$(call create_executable,$@,$^)

$(BUILD_DIR_PATH)/bin/file_encryptor: $(BUILD_DIR_PATH)/examples/obj/file_encryptor.o $(NTRU_LIB)
	$(call create_executable,$@,$^)

# Generic compilation rule for example files
$(BUILD_DIR_PATH)/examples/obj/%.o: %.cpp
	$(call compile_cpp,$<,$@,-I../$(INCLUDE_DIR))

clean:
	$(call standard_clean,$(BUILD_DIR_PATH)/bin/basic_encryption $(BUILD_DIR_PATH)/bin/file_encryptor)

#	$(call standard_clean,$(BUILD_DIR_PATH)/bin/example_* $(BUILD_DIR_PATH)/bin/file_* $(BUILD_DIR_PATH)/bin/perf_* $(BUILD_DIR_PATH)/bin/advanced_* $(BUILD_DIR_PATH)/examples)

# Include dependency files
-include $(DEPS)
