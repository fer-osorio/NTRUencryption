all: NTRUencryption.exe NTRUdecryption.exe

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
SOURCE   = ../../Source/*.cpp ../Settings.cpp
HEADERS  = ../../Source/*.hpp ../Settings.hpp

NTRUencryption.exe: Makefile $(HEADERS) $(SOURCE) ../encryption.cpp
	$(CXX) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) ../encryption.cpp $(SOURCE)

NTRUdecryption.exe: Makefile $(HEADERS) $(SOURCE) ../decryption.cpp
	$(CXX) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) ../decryption.cpp $(SOURCE)

clean:
	rm -f *.exe

clean_all:
	rm -f *.exe *.ntru*

install:
	echo "Installing is not supported"

run_encryption:
	./NTRUencryption.exe

run_decryption:
	./NTRUdecryption.exe
