all: NTRUencryption.exe NTRUdecryption.exe

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
SRC = ../../Source/

NTRUencryption.exe: Makefile ../encryption.cpp $(SRC)NTRUencryption.cpp $(SRC)NTRUencryption.hpp
	$(CXX) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) ../encryption.cpp $(SRC)NTRUencryption.cpp

NTRUdecryption.exe: Makefile ../decryption.cpp $(SRC)NTRUencryption.cpp $(SRC)NTRUencryption.hpp
	$(CXX) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) ../decryption.cpp $(SRC)NTRUencryption.cpp

clean:
	rm -f *.exe

clean_all:
	rm -f *.exe *.ntru*

# Builder will call this to install the application before running.
install:
	echo "Installing is not supported"

# Builder uses this target to run NTRUencryption.exe
run_encryption:
	./NTRUencryption.exe

# Builder uses this target to run NTRUdecryption.exe
run_decryption:
	./NTRUdecryption.exe
