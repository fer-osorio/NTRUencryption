#!/bin/bash

# Script: binary_generator.sh
# Purpose: Create a binary file with arbitrary and random data. It uses a variety of methods for illustrative purposes. The idea is to have a source of files so we can run tests with other programs.
# Author: Fernando Osorio
# Date: 2025-07-08

set -euo pipefail                                                # Exit on: error [set -e], undefined variables [set -u], pipe failure [set -o]

usage(){                                                         # Function to display usage
	echo "Usage: $0 [-f filename] [-h]"
	echo "	-f filename: Specify output filename"
	echo "	-h: Shows this help message"
	exit 1
}

validate_filename(){                                             # Function to validate filename
	local filename="$1"
	if [[ -z "$filename" ]]; then                                  # Check for empty filename
		echo "Error: Filename cannot be empty" >&2                   # Sending output to file descriptor stderr
		return 1
	fi
	if [[ "$filename" =~ [[:space:]] ]]; then                      # Check for invalid characters (basic check). Using POSIX notation [[:class]] for defining a character class inside a character set
		echo "Warning: Filename contains spaces, this may cause issues" >&2	# Sending warning to stderr
	fi
	if [[ -f "$filename" ]]; then
		read -p "File '$filename' already exist. Overwrite) (y/N): " -r # When read is called without a variable name, it automatically stores the input in a special build-in variable called REPLY. Option -r prevents backslash escaping (treats backslashes literally)
		if [[ ! $REPLY =~ ^[Yy]$ ]]; then                            # ^ Start of string anchor; ensures the match begins at the very start. $ End of string anchor; ensures the match ends at the very end.
			echo "Operation cancelled"
			exit 0
		fi
	fi
}

create_binary_file(){                                            # Function to create binary file
	local filename="$1"

	echo "Creating binary file: $filename"
	echo "Structure:"
	echo "  - First 16 bytes: sequence {1...16}"
	echo "  - Next 30 bytes: random values"
	echo "  - Next 30 bytes: null bytes"
	echo "  - Next 11 bytes: \"Hello World\" in hex"
	echo "  - Last 16 bytes: sequence {240...255}"

	> "$filename"                                                  # Create empty file

	echo -en '\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0F\x10' >> "$filename" # 1. First 16 bytes: sequence from 215 to 254. '%02x' format specifier: %02x format as hexadecimal, at least two digits, pad with zeros if needed. printf '\x%02x' 215 216 217 218 ... 254 -> printf '\xd7\xd8\xd9\xda...\xfe'
	dd if=/dev/urandom of="$filename" bs=1 count=30 oflag=append conv=notrunc 2>/dev/null  # 2. 30 bytes of random data. if: input file /dev/urandom is a special device that provides random bytes; of: output file; bs: block size; count: number of blocks to copy; oflag=append: opens output file in append mode; conv=notrunc: do not truncate the output file (preserve existing content); 2>/dev/null: Redirect stderr to suppress dd's status messages
	dd if=/dev/zero of="$filename" bs=1 count=30 oflag=append conv=notrunc 2>/dev/null     # 3. 30 null bytes
	echo -n "48656C6C6F20576F726C64" | xxd -r -p >> "$filename"                            # 4. Hello world in Hexadecimal. Arbitrary data is represented as a hexadecimal string, xxd can convert it to a binary file
	echo -n "F0F1F2F3F4F4F6F7F8F9FAFBFCFDFEFF" | xxd -r -p >> "$filename"                  # 5. Final sequence
	echo "File created successfully!"
	echo "File size: $(wc -c < "$filename") bytes"
}

# Start of: Parse command line arguments
filename=""
while getopts "f:h" opt; do
	case $opt in
		f)
			filename="$OPTARG"
			;;
		h)
			usage
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			usage
			;;
	esac
done
# End of: Parse command line arguments

if [[ -z "$filename" ]]; then                                          # If no filename provided via arguments, prompt for it
	read -p "Enter filename for the binary file: " filename
fi

validate_filename "$filename"                                          # Validate filename
create_binary_file "$filename"                                         # Create file

echo ""
echo "File analysis:"
echo "Hex dump (first 50 bytes):"
xxd -l 50 "$filename"
