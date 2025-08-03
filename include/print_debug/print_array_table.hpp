#include<iostream>

size_t lenHexRep(int a);

// Templated function to print a table of values in hexadecimal
template <typename T> void print_table(
	const T* data,
	size_t data_size,
	std::string_view title = "",
	std::string_view tail = "",
	const unsigned int column_width = 5,
	const unsigned int columns = 16
);