all: NTRUencryption.exe NTRUdecryption.exe Statistics.exe

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
SOURCE   = Source/*.cpp Apps/Settings.cpp
HEADERS  = Source/*.hpp Apps/Settings.hpp

NTRUencryption.exe: Makefile $(HEADERS) $(SOURCE) Apps/encryption.cpp
	$(CXX) -o Apps/Executables/$@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) Apps/encryption.cpp $(SOURCE)

NTRUdecryption.exe: Makefile $(HEADERS) $(SOURCE) Apps/decryption.cpp
	$(CXX) -o Apps/Executables/$@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) Apps/decryption.cpp $(SOURCE)

Statistics.exe: Makefile Source/*.hpp Source/*.cpp Apps/Statistics.cpp
	$(CXX) -o Apps/Executables/$@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) Apps/Statistics.cpp Source/*.cpp -lgmpxx -lgmp

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

run_statistics:
	Apps/Executables/Statistics.exe
