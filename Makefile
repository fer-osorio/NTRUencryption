all: NTRUencryption NTRUdecryption Testing

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
SOURCE   = Source/*.cpp Apps/Settings.cpp
HEADERS  = Source/*.hpp Apps/Settings.hpp
NVALS	 := 701 821 1087 1171 1499
QVALS	 := 4096 8192
N ?= 701
q ?= 4096

NTRUencryption: Makefile $(HEADERS) $(SOURCE) Apps/encryption.cpp
ifneq ($(filter $(N),$(NVALS)),)
ifneq ($(filter $(q),$(QVALS)),)
	$(CXX) -o Apps/Executables/EncryptionDecryption/$@$(N)$(q) $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=$(q) -D_N_=$(N) Apps/encryption.cpp $(SOURCE) -lgmpxx -lgmp
else
	echo "q parameter not supported"
endif
else
	echo "N parameter not supported"
endif

NTRUdecryption: Makefile $(HEADERS) $(SOURCE) Apps/decryption.cpp
ifneq ($(filter $(N),$(NVALS)),)
ifneq ($(filter $(q),$(QVALS)),)
	$(CXX) -o Apps/Executables/EncryptionDecryption/$@$(N)$(q) $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=$(q) -D_N_=$(N) Apps/decryption.cpp $(SOURCE) -lgmpxx -lgmp
else
	echo "q parameter not supported"
endif
else
	echo "N parameter not supported"
endif

Testing: Makefile Source/*.hpp Source/*.cpp Apps/Statistics.cpp
	for N in 509 677; do \
		$(CXX) -o Apps/Executables/$@2048$$N $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=2048 -D_N_=$$N Apps/Statistics.cpp Source/*.cpp -lgmpxx -lgmp; \
	done
	for N in 701 821; do \
		$(CXX) -o Apps/Executables/$@4096$$N $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=4096 -D_N_=$$N Apps/Statistics.cpp Source/*.cpp -lgmpxx -lgmp; \
	done
	for N in 701 821 1087 1171 1499; do \
		$(CXX) -o Apps/Executables/$@8192$$N $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) -D_q_=8192 -D_N_=$$N Apps/Statistics.cpp Source/*.cpp -lgmpxx -lgmp; \
	done
clean:
	rm -f Apps/Executables/*.exe

clean_all:
	rm -f Apps/Executables/*.exe Apps/Executables/*.ntru*

install:
	echo "Installing is not supported"

run_encryption:
	Apps/Executables/EncryptionDecryption/NTRUencryption

run_decryption:
	Apps/Executables/EncryptionDecryption/NTRUdecryption

run_test:
	Apps/Executables/Testing/Testing$(q)$(N)
