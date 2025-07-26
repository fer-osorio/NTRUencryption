# Configuration file - all build settings
CC = gcc
CXX = g++
AR = ar
WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a
COMPILE_FLAGS = $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD)
LDFLAGS = -lgmp -lgmpxx

# Directories
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
EXAMPLES_DIR = examples
