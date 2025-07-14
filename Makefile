all: basic_example NTRUencryption performance_test

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
SOURCE   = Source/*.cpp
HEADERS  = Source/*.hpp

EXES_PATH	= Apps/Executables
EXE_ENCRYPTION	= $(EXES_PATH)/encryption
EXE_PERF_TEST	= $(EXES_PATH)/performance_test

NVALS	 = 701 821 1087 1171 1499
QVALS	 = 4096 8192
N ?= 701
q ?= 4096

# Validation function
PARAMS_VALID = $(and $(filter $(N),$(NVALS)),$(filter $(q),$(QVALS)))

# Common compilation flags
COMPILE_FLAGS = $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=$(q) -D_N_=$(N)
LINK_FLAGS = -lgmpxx -lgmp

# Exception messages
ECHO_INVPARAM	= @echo "Invalid parameters. N must be one of: $(NVALS), q must be one of: $(QVALS)"
ECHO_CURRPARAM	= @echo "Current values: N=$(N), q=$(q)"
ECHO_HELP	= @echo "Run 'make help' for more information." \

.PHONY: all clean clean_all install run_encryption run_performance_test

basic_example: Makefile $(HEADERS) $(SOURCE) Apps/basic_example.cpp
	$(CXX) -o $(EXES_PATH)/$@ $(COMPILE_FLAGS) Apps/basic_example.cpp $(SOURCE) $(LINK_FLAGS)

NTRUencryption: Makefile $(HEADERS) $(SOURCE) Apps/encryption.cpp
ifneq ($(PARAMS_VALID),)
	@mkdir -p $(dir $(EXE_ENCRYPTION)/$@$N(N)q$(q))                         # Creates (if they not exist) directories and parents directories of the path EXE_ENCRYPTION.
	$(CXX) -o $(EXE_ENCRYPTION)/$@N$(N)q$(q) $(COMPILE_FLAGS) Apps/encryption.cpp $(SOURCE) $(LINK_FLAGS)
else
	$(ECHO_INVPARAM)
	$(ECHO_CURRPARAM)
	@false
endif

performance_test: Makefile $(HEADERS) $(SOURCE) Apps/performance_test.cpp
ifneq ($(PARAMS_VALID),)
	@mkdir -p $(dir $(EXE_PERF_TEST)/$@N$(N)q$(q))				# Creates (if they not exist) directories and parents directories of the path EXE_ENCRYPTION.
	$(CXX) -o $(EXE_PERF_TEST)/$@N$(N)q$(q) $(COMPILE_FLAGS) Apps/performance_test.cpp $(SOURCE) $(LINK_FLAGS)
else
	$(ECHO_INVPARAM)
	$(ECHO_CURRPARAM)
	@false
endif

clean:
	@echo "Cleaning executables..."
	-rm -f $(EXE_ENCRYPTION)/*
	-rm -f $(EXE_PERF_TEST)/*/*

clean_all: clean
	@echo "Cleaning all generated files..."
	-find $(EXES_PATH) -name "*.ntru*" -delete

install:
	@echo "Installing is not supported"

# Run targets with existence checks
run_basic_example:
	@if [ -f '$(EXES_PATH)/basic_example' ]; then \
		'$(EXES_PATH)/basic_example'; \
	else \
		echo "Basic example executable not found. Run 'make basic_example' first."; \
		$(ECHO_HELP) \
		false; \
	fi

run_encryption:
	@if [ -f "$(EXE_ENCRYPTION)/NTRUencryptionN$(N)q$(q)" ]; then \
		echo "In directory $(EXE_ENCRYPTION):"; \
		$(EXE_ENCRYPTION)/NTRUencryptionN$(N)q$(q); \
	else \
		echo "Executable not found. Run 'make NTRUencryption' first."; \
		$(ECHO_HELP) \
		false; \
	fi

run_performance_test:
	@if [ -f "$(EXE_PERF_TEST)/performance_testN$(N)q$(q)" ]; then \
		$(EXE_PERF_TEST)/performance_testN$(N)q$(q); \
	else \
		echo "performance_testN$(N)q$(q) executable not found. Run 'make performance_test' first."; \
		$(ECHO_HELP) \
		false; \
	fi

# Utility targets
show-config:
	@echo "Current configuration:"
	@echo "  N = $(N) (valid: $(NVALS))"
	@echo "  q = $(q) (valid: $(QVALS))"
	@echo "  Valid parameters: $(if $(PARAMS_VALID),YES,NO)"

help:
	@echo "Available targets:"
	@echo "  all              	- Build all executables"
	@echo "  NTRUencryption   	- Build encryption executable"
	@echo "  performance_test 	- Build performance_test executable"
	@echo "  clean            	- Remove executables"
	@echo "  clean_all        	- Remove all generated files"
	@echo "  run_encryption   	- Run encryption program"
	@echo "  run_performance_test	- Run performance_test program"
	@echo "  show-config      	- Show current parameter values"
	@echo "  help             	- Show this help"
	@echo ""
	@echo "Parameters:"
	@echo "  N=$(N) (valid: $(NVALS))"
	@echo "  q=$(q) (valid: $(QVALS))"
	@echo ""
	@echo "Usage examples:"
	@echo "  make N=821 q=8192"
	@echo "  make clean && make N=1171"
