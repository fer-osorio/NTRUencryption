all: NTRUencryption NTRUdecryption Test

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
SOURCE   = Source/*.cpp Apps/Settings.cpp
HEADERS  = Source/*.hpp Apps/Settings.hpp
EXE_EXAMP_PATH = Apps/Executables/basic_examples
EXE_ENCDEC_PATH = Apps/Executables/encryption_decryption
EXE_TESTS_PATH  = Apps/Executables/Testing

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
ECHO_INVPARAM = @echo "Invalid parameters. N must be one of: $(NVALS), q must be one of: $(QVALS)"
ECHO_CURRPARAM= @echo "Current values: N=$(N), q=$(q)"

.PHONY: all clean clean_all install run_encryption run_decryption run_test

basic_example: Makefile $(HEADERS) $(SOURCE) Apps/basic_example.cpp
ifneq ($(PARAMS_VALID),)
	@mkdir -p $(dir $(EXE_EXAMP_PATH)/$@$(N)$(q))
	$(CXX) -o $(EXE_EXAMP_PATH)/$@N$(N)q$(q) $(COMPILE_FLAGS) Apps/basic_example.cpp $(SOURCE) $(LINK_FLAGS)
else
	$(ECHO_INVPARAM)
	$(ECHO_CURRPARAM)
	@false
endif

NTRUencryption: Makefile $(HEADERS) $(SOURCE) Apps/encryption.cpp
ifneq ($(PARAMS_VALID),)
	@mkdir -p $(dir $(EXE_ENCDEC_PATH)/$@$(N)$(q))
	$(CXX) -o $(EXE_ENCDEC_PATH)/$@$(N)$(q) $(COMPILE_FLAGS) Apps/encryption.cpp $(SOURCE) $(LINK_FLAGS)
else
	$(ECHO_INVPARAM)
	$(ECHO_CURRPARAM)
	@false
endif

NTRUdecryption: Makefile $(HEADERS) $(SOURCE) Apps/decryption.cpp
ifneq ($(PARAMS_VALID),)
	@mkdir -p $(dir $(EXE_ENCDEC_PATH)/$@$(N)$(q))
	$(CXX) -o $(EXE_ENCDEC_PATH)/$@$(N)$(q) $(COMPILE_FLAGS) Apps/decryption.cpp $(SOURCE) $(LINK_FLAGS)
else
	$(ECHO_INVPARAM)
	$(ECHO_CURRPARAM)
	@false
endif

Test: Makefile $(HEADERS) $(SOURCE) Apps/Statistics.cpp
ifneq ($(PARAMS_VALID),)
	@mkdir -p $(dir $(EXE_TESTS_PATH)/N$(N)q$(q)/Test$(N)$(q))
	$(CXX) -o $(EXE_TESTS_PATH)/N$(N)q$(q)/Test$(N)$(q) $(COMPILE_FLAGS) Apps/Statistics.cpp $(SOURCE) $(LINK_FLAGS)
else
	$(ECHO_INVPARAM)
	$(ECHO_CURRPARAM)
	@false
endif

clean:
	@echo "Cleaning executables..."
	-rm -f $(EXE_ENCDEC_PATH)/*
	-rm -f $(EXE_TESTS_PATH)/*/*

clean_all: clean
	@echo "Cleaning all generated files..."
	-find Apps/Executables -name "*.ntru*" -delete

install:
	@echo "Installing is not supported"

# Run targets with existence checks
run_encryption:
	@if [ -f "$(EXE_ENCDEC_PATH)/NTRUencryption$(N)$(q)" ]; then \
		$(EXE_ENCDEC_PATH)/NTRUencryption$(N)$(q); \
	else \
		echo "Executable not found. Run 'make NTRUencryption' first."; \
		false; \
	fi

run_decryption:
	@if [ -f "$(EXE_ENCDEC_PATH)/NTRUdecryption$(N)$(q)" ]; then \
		$(EXE_ENCDEC_PATH)/NTRUdecryption$(N)$(q); \
	else \
		echo "Executable not found. Run 'make NTRUdecryption' first."; \
		false; \
	fi

run_test:
	@if [ -f "$(EXE_TESTS_PATH)/N$(N)q$(q)/Test$(N)$(q)" ]; then \
		$(EXE_TESTS_PATH)/N$(N)q$(q)/Test$(N)$(q); \
	else \
		echo "Test executable not found. Run 'make Test' first."; \
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
	@echo "  all              - Build all executables"
	@echo "  NTRUencryption   - Build encryption executable"
	@echo "  NTRUdecryption   - Build decryption executable"
	@echo "  Test             - Build test executable"
	@echo "  clean            - Remove executables"
	@echo "  clean_all        - Remove all generated files"
	@echo "  run_encryption   - Run encryption program"
	@echo "  run_decryption   - Run decryption program"
	@echo "  run_test         - Run test program"
	@echo "  show-config      - Show current parameter values"
	@echo "  help             - Show this help"
	@echo ""
	@echo "Parameters:"
	@echo "  N=$(N) (valid: $(NVALS))"
	@echo "  q=$(q) (valid: $(QVALS))"
	@echo ""
	@echo "Usage examples:"
	@echo "  make N=821 q=8192"
	@echo "  make clean && make N=1171"
