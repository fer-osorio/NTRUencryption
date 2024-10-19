all: NTRUencryption.exe NTRUdecryption.exe

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a

NTRUencryption.exe: Makefile Apps/encryption.cpp Source/NTRUencryption.cpp Source/NTRUencryption.hpp
	$(CXX) -o Executables/$@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) Apps/encryption.cpp Source/NTRUencryption.cpp

NTRUdecryption.exe: Makefile Apps/decryption.cpp Source/NTRUencryption.cpp Source/NTRUencryption.hpp
	$(CXX) -o Executables/$@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) Apps/decryption.cpp Source/NTRUencryption.cpp

clean:
	rm -f Executables/*.exe

clean_all:
	rm -f Executables/*

# Builder will call this to install the application before running.
install:
	echo "Installing is not supported"

# Builder uses this target to run NTRUencryption.exe
run_encryption:
	./Executables/NTRUencryption.exe

# Builder uses this target to run NTRUdecryption.exe
run_decryption:
	./Executables/NTRUdecryption.exe
