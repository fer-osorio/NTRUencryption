all: NTRUencryption NTRUdecryption Test

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
SOURCE   = Source/*.cpp Apps/Settings.cpp
HEADERS  = Source/*.hpp Apps/Settings.hpp
EXE_ENCDEC_PATH = Apps/Executables/EncryptionDecryption
EXE_TESTS_PATH  = Apps/Executables/Testing

NVALS	 = 701 821 1087 1171 1499
QVALS	 = 4096 8192
N ?= 701
q ?= 4096

QEXPMSG = @echo "q parameter not supported. Valid values are $(QVALS)"
NEXPMSG = @echo "N parameter not supported. Valid values are $(NVALS)"

NTRUencryption: Makefile $(HEADERS) $(SOURCE) Apps/encryption.cpp
ifneq ($(filter $(N),$(NVALS)),)
ifneq ($(filter $(q),$(QVALS)),)
	$(CXX) -o $(EXE_ENCDEC_PATH)/$@$(N)$(q) $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=$(q) -D_N_=$(N) Apps/encryption.cpp $(SOURCE) -lgmpxx -lgmp
else
	$(QEXPMSG)
endif
else
	$(NEXPMSG)
endif

NTRUdecryption: Makefile $(HEADERS) $(SOURCE) Apps/decryption.cpp
ifneq ($(filter $(N),$(NVALS)),)
ifneq ($(filter $(q),$(QVALS)),)
	$(CXX) -o $(EXE_ENCDEC_PATH)/$@$(N)$(q) $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=$(q) -D_N_=$(N) Apps/decryption.cpp $(SOURCE) -lgmpxx -lgmp
else
	$(QEXPMSG)
endif
else
	$(NEXPMSG)
endif

Test: Makefile Source/*.hpp Source/*.cpp Apps/Statistics.cpp
ifneq ($(filter $(N),$(NVALS)),)
ifneq ($(filter $(q),$(QVALS)),)
	$(CXX) -o $(EXE_TESTS_PATH)/N$(N)q$(q)/$@$(N)$(q) $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=$(q) -D_N_=$(N) Apps/Statistics.cpp Source/*.cpp -lgmpxx -lgmp
else
	$(QEXPMSG)
endif
else
	$(NEXPMSG)
endif

#clean:
#	rm -f Apps/Executables/*.exe

#clean_all:
#	rm -f Apps/Executables/*.exe Apps/Executables/*.ntru*

#install:
#	@echo "Installing is not supported"

run_encryption:
	$(EXE_ENCDEC_PATH)/NTRUencryption$(q)$(N)

run_decryption:
	$(EXE_ENCDEC_PATH)/NTRUdecryption$(q)$(N)

run_test:
	$(EXE_TESTS_PATH)/N$(N)q$(q)/Test$(q)$(N)
