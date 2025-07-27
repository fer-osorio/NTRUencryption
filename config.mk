# Configuration file - all build settings
CC = gcc
CXX = g++
AR = ar

# NTRU Parameters (can be overridden on command line)
NTRU_N ?= 509
NTRU_q ?= 2048

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a

# Base compiler flags
COMPILE_FLAGS = $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD)
LDFLAGS = -lgmp -lgmpxx

# Add NTRU parameters as preprocessor defines
CXXFLAGS += -DNTRU_N=$(NTRU_N) -DNTRU_q=$(NTRU_q) -DNTRU_P=$(NTRU_P)

# Directories
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
EXAMPLES_DIR = examples
