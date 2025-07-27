# Common Makefile rules and functions

# Ensure we have the basic variable set
ifndef BUILD_DIR
BUILD_DIR = build
endif

ifndef INCLUDE_DIR
INCLUDE_DIR = include
endif

# Colors for pretty output (not necessary but cool)
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
NC = \033[0m          # No Color

# Function to create directories
define create_dir
	@mkdir -p $(1)
endef

# Functions to print colored messages
define print_info
	@echo "$(BLUE)[INFO]$(NC) $(1)"
endef

define print_success
	@echo "$(GREEN)[SUCCESS]$(NC) $(1)"
endef

define print_warning
	@echo "$(YELLOW)[WARNING]$(NC) $(1)"
endef

define print_error
	@echo "$(RED)[ERROR]$(NC) $(1)"
endef

# Standard compilation rule for C++ files
# Usage: $(call compile_cpp, source_file, object_file, additional_flags)
define compile_cpp
	$(call print_info,Compiling $(1))
	$(call create_dir,$(dir $(2)))
	$(CXX) $(CXXFLAGS) $(3) -I(INCLUDE_DIR) -MMD -MP -c $(1) -o $(2) # -I: Compiler includes directory, MMD: Generates dependency fiel, -MP: Add pony targets for each dependency
endef

# Standard rule for creating static libraries
# Usage: $(call create_static_lib, library_file, object_files)
define create_static_lib
	$(call print_info,Creating static library $(1))
	$(call create_dir,$(dir $(1)))
	$(AR) rcs $(1) $(2) # AR: Archiver (usually ar), r: Insert files into archive (replace if they exist), c: Create archive if it does not exist (do not warn), s: Write an index (symbol table) for faster linking
	$(call print_success,Static library created; $(1))
endef

# Standard rule for creating shared libraries
# Usage: $(call create_shared_lib, library_file, object_files, additional_ldflags)
define create_shared_lib
	$(call print_info,Creating shared library $(1))
	$(call create_dir,$(dir $(1)))
	$(CXX) -shared -o $(1) $(2) $(LDFLAGS) $(3)
	$(call print_success,Shared library created: $(1))
endef

# Standard rule for creating executables
# Usage: $(call create_executable, exe_file, object_files, additional_ldflags)
define create_executable
	$(call print_info,Creating executable $(1))
	$(call create_dir,$(dir $(1)))
	$(CXX) -o $(1) $(2) $(LDFLAGS) $(3)
	$(call print_success,Executable created; $(1))
endef

# Function to find all source files in directory
# Usage: SOURCES = $(call find_sources, directory, extension)
define find_sources
$(shell find $(1) -name "*.$(2)" 2>/dev/null)
endef

# Function to convert source files to object files
# Usage: OBJECTS = (call sources_to_objects, sources, build_prefix)
define sources_to_objects
$(patsubst %.cpp,$(2)/obj/%.o,$(1)) # If $(1) has .cpp as suffix, the suffix gets "cutted" and the $(2)/obj/>>the rest of $(1)<<.o is the output
endef

# Function to get dependency files from object files
# Usage DEPS = $(call objects_to_deps,objects)
define objects_to_deps
$(patsubst %.o,%.d,$(1))
endef

# Standard clean rule
# Usage: $(call standard_clean, directories_to_clean)
define standard_clean
	$(call print_info,Cleaning $(1))
	rm -rf $(1) # -rf: recursive and force options
	$(call print_success,Cleaned $(1))
endef

# Check if program exist
# Usage: $(call check_program,program_name)
define check_program
$(shell which $(1) >/dev/null 2>&1 && echo "found" || echo "missing") # 'which $(1)' look for $(1); '>/dev/null 2>&1' redirect stdout and stderr to supress status message; ' && echo "found" || echo "missing" ' if which succed, outputs "found", if not outputs "missing"
endef

define check_dependencies
	$(call print_info,Checking dependencies...)
	@if [ "$(call check_program,$(CXX))" = "missing" ]; then \		# Check if GNU g++ compiler is installed
		$(call print_error,C++ compiler $(CXX) not found); \
		exit 1; \
	fi
	@if [ "$(call check_program,pkg-config)" = "found" ]; then \		# Check if pkg-config package is installed
		if ! pkg-config --exists gmp >/dev/null 2>&1; then \		# Check -through pkg-config- if gmp library is installed
			$(call print_warning,GMP library not found via pkg-config); \
		fi; \
	else \
		$(call print_warning,pkg-config not found, skipping GMP check); \
	fi
	$(call print_success,Dependencies check completed)
endef

# Help function - shows available targets
define show_help
    @echo "$(BLUE)Available targets:$(NC)"
    @echo "  $(GREEN)all$(NC)       - Build everything (libraries, examples)"
    @echo "  $(GREEN)lib$(NC)       - Build only the library"
    @echo "  $(GREEN)examples$(NC)  - Build example programs"
    @echo "  $(GREEN)test$(NC)      - Build and run tests"
#    @echo "  $(GREEN)docs$(NC)      - Generate documentation"
#    @echo "  $(GREEN)clean$(NC)     - Remove all build artifacts"
#    @echo "  $(GREEN)install$(NC)   - Install library system-wide"
    @echo "  $(GREEN)help$(NC)      - Show this help message"
    @echo ""
#    @echo "$(BLUE)Build options:$(NC)"
#    @echo "  $(YELLOW)DEBUG=1$(NC)   - Build with debug symbols"
#    @echo "  $(YELLOW)-j N$(NC)      - Build with N parallel jobs"
#    @echo ""
#    @echo "$(BLUE)Examples:$(NC)"
#    @echo "  make DEBUG=1           # Debug build"
#    @echo "  make -j8               # Parallel build with 8 jobs"
#    @echo "  make clean all         # Clean rebuild"
endef

# Make help the default if no target is specified
.DEFAULT_GOAL := help

# Common phony targets
.PHONY: help clean all
