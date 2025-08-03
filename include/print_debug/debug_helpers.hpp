#include<cstddef>

#ifndef PRINT_HPP
#define PRINT_HPP

// Printing array of bytes with an specific ofrmat. Format can be "binary" or char
// Format:
//	binary: Displays array binary notation
//	char: If posible, diplays bytes as printable character, otherwhise will print its hexadecimal value.
void displayByteArray(const char byteArray[], size_t size, size_t columnSize, const char format[]);
void displayByteArrayBin(const char byteArray[], size_t size);                  // -Printing array of bytes. Useful for debbuging
void displayByteArrayChar(const char byteArray[], size_t size);                 // -Prints each byte of array as a character or hexadecimal value
#endif