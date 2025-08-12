# Configuration file - all build settings
CC = gcc
CXX = g++
AR = ar

# NTRU Parameters (can be overridden on command line)
NTRU_N ?= 701
NTRU_q ?= 8192

# Add NTRU parameters as preprocessor defines
NTRU_PARAMETERS = -DNTRU_N=$(NTRU_N) -DNTRU_q=$(NTRU_q) #-DNTRU_P=$(NTRU_P)

WARNINGS = -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors
DEBUG    = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O2
STANDARD = -std=c++2a

# Base compiler flags
# GMPXX Support Configuration
INCLUDE_GMPXX ?= false

# Conditional compilation flags
ifeq ($(INCLUDE_GMPXX),true)
GMPXX_FLAGS = -DGMPXX_INCLUDED
GMPXX_LIBS = -lgmp -lgmpxx
else
GMPXX_FLAGS =
GMPXX_LIBS =
endif

CXXFLAGS = $(WARNINGS) $(DEBUG) $(OPTIMIZE) $(STANDARD)
NTRULIB_FLAGS = $(GMPXX_FLAGS) $(NTRU_PARAMETERS)

LDFLAGS = $(GMPXX_LIBS)

# Directories
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
EXAMPLES_DIR = examples
