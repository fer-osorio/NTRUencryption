# examples/Makefile - Build example programs
include ../config.mk
include ../mk/common.mk

# Library dependency
NTRU_LIB = $(BUILD_DIR)/lib/libntru.a

# Find all example sources
BASIC_SOURCES = basic_encryption.cpp #$(call find_sources,basic_usage,cpp)
FILE_SOURCES = file_encryptor.cpp #$(call find_sources,file_encryption,cpp)

# Example executables (remove directory prefix and .cpp extension)
BASIC_EXAMPLES = $(BUILD_DIR)/bin/basic_encryption #$(patsubst basic_usage/%.cpp,$(BUILD_DIR)/bin/example_%,$(BASIC_SOURCES))
FILE_EXAMPLES = $(BUILD_DIR)/bin/file_encryptor #$(patsubst file_encryption/%.cpp,$(BUILD_DIR)/bin/file_%,$(FILE_SOURCES)))

ALL_EXAMPLES = $(BASIC_EXAMPLES) $(FILE_EXAMPLES)

.PHONY: all basic file

all: $(ALL_EXAMPLES)

basic: $(BASIC_EXAMPLES)
file: $(FILE_EXAMPLES)

# Build rules for different example categories
$(BUILD_DIR)/bin/basic_encryption: $(BUILD_DIR)/examples/obj/basic_encryption.o $(NTRU_LIB)
	$(call create_executable,$@,$^)

$(BUILD_DIR)/bin/file_encryptor: $(BUILD_DIR)/examples/obj/file_encryptor.o $(NTRU_LIB)
	$(call create_executable,$@,$^)

# Generic compilation rule for example files
$(BUILD_DIR)/examples/obj/%.o: %.cpp
	$(call compile_cpp,$<,$@,-I../$(INCLUDE_DIR))

#clean:
#	$(call standard_clean,$(BUILD_DIR)/bin/example_* $(BUILD_DIR)/bin/file_* $(BUILD_DIR)/bin/perf_* $(BUILD_DIR)/bin/advanced_* $(BUILD_DIR)/examples)

# Include dependency files
-include $(DEPS)
