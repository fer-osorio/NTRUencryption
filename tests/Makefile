# tests/Makefile - Build and run all tests
include ../config.mk
include ../mk/common.mk

UP = ../
BUILD_DIR_PATH = $(UP)$(BUILD_DIR)

# Library dependency
NTRU_LIB = $(BUILD_DIR_PATH)/lib/libntru.a

PERFORMANCE_SOURCES = $(call find_sources,performance,cpp)
PERFORMANCE_OBJECTS = $(call sources_to_objects,$(PERFORMANCE_SOURCES),$(BUILD_DIR_PATH)/tests)
PERFORMANCE_TESTS = $(patsubst performance/%.cpp,$(BUILD_DIR_PATH)/bin/%,$(PERFORMANCE_SOURCES))
$(info PERFORMANCE_TESTS: $(PERFORMANCE_TESTS))                                 # Debugging purposes

ALL_TESTS = $(PERFORMANCE_TESTS)
ALL_OBJECTS = $(PERFORMANCE_OBJECTS)
DEPS = $(call objects_to_deps,$(ALL_OBJECTS))

.PHONY: all performance clean

all: $(ALL_TESTS)

performance: $(PERFORMANCE_TESTS)

# Build performance test executables
$(BUILD_DIR_PATH)/bin/%: $(BUILD_DIR_PATH)/tests/obj/performance/%.o $(NTRU_LIB)
	$(call create_executable,$@,$^,)

# Generic compilation rule for test files
$(BUILD_DIR_PATH)/tests/obj/%.o: %.cpp
	$(call compile_cpp,$<,$@,-I../$(INCLUDE_DIR))

clean:
	$(call standard_clean,$(BUILD_DIR_PATH)/bin/performance* $(BUILD_DIR_PATH)/tests)

# Include dependency files
-include $(DEPS)
