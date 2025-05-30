all: NTRUencryption.exe NTRUdecryption.exe Testing

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
SOURCE   = Source/*.cpp Apps/Settings.cpp
HEADERS  = Source/*.hpp Apps/Settings.hpp
Q2048    = q2048

NTRUencryption.exe: Makefile $(HEADERS) $(SOURCE) Apps/encryption.cpp
	$(CXX) -o Apps/Executables/$@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) Apps/encryption.cpp $(SOURCE)

NTRUdecryption.exe: Makefile $(HEADERS) $(SOURCE) Apps/decryption.cpp
	$(CXX) -o Apps/Executables/$@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) Apps/decryption.cpp $(SOURCE)

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
	Apps/Executables/NTRUencryption.exe

run_decryption:
	Apps/Executables/NTRUdecryption.exe

run_test:
	Apps/Executables/Testing$(q)$(N)
