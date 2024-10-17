all: NTRUencryption.exe NTRUdecryption.exe

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a

NTRUencryption.exe: Makefile encryption.cpp NTRUencryption.cpp NTRUencryption.hpp
	$(CXX) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) encryption.cpp NTRUencryption.cpp

NTRUdecryption.exe: Makefile decryption.cpp NTRUencryption.cpp NTRUencryption.hpp
	$(CXX) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) decryption.cpp NTRUencryption.cpp

clean:
	rm -f NTRUencryption.exe NTRUdecryption.exe

# Builder will call this to install the application before running.
install:
	echo "Installing is not supported"

# Builder uses this target to run your application.
run:
	./NTRUencryption.exe

