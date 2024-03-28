all: NTRUencryption.exe

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a

NTRUencryption.exe: Makefile main.cpp NTRUencryption.cpp NTRUencryption.hpp NTRUPolynomials.cpp NTRUPolynomials.hpp
	$(CXX) -o $@ $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD) main.cpp NTRUencryption.cpp NTRUPolynomials.cpp

qmod3Array.o: Makefile qmod3ArrayGen.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o qmod3ArrayGen.c

qmod3Array: Makefile qmod3ArrayGen.o
	$(CC) $(LDFLAGS) -o $@ qmod3ArrayGen.o $(LOADLIBES) $(LDLIBS)

clean:
	rm -f NTRUencryption.exe

# Builder will call this to install the application before running.
install:
	echo "Installing is not supported"

# Builder uses this target to run your application.
run:
	./NTRUencryption.exe

