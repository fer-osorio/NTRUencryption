#include<iomanip>      // For std::hex, std::setw
#include"../../include/print_debug/print_array_table.hpp"
#include"../../include/ntru/polynomials.hpp"

size_t lenHexRep(int a) {                                   			        // -Returns the number of characters for the hexadecimal representation of a
    size_t l = size_t(a < 0 ? a = -a, 1 : 0);					                // -If 'a' is negative, length is at least 1, otherwise is 0 (at this point)
    do {
    	a >>= 4;								                                // -Equivalent to a/=16
    	l++;
    }while(a > 0);
    return l;
}

template void print_table(
	const short* data,
	size_t data_size,
	std::string_view title,
	std::string_view tail,
	const unsigned int column_width,
	const unsigned int columns);

template void print_table(
	const NTRU::R2Polynomial::Z2* data,
	size_t data_size,
	std::string_view title,
	std::string_view tail,
	const unsigned int column_width,
	const unsigned int columns);

template void print_table(
	const int64_t* data,
	size_t data_size,
	std::string_view title,
	std::string_view tail,
	const unsigned int column_width,
	const unsigned int columns);

// Templated function to print a table of values in hexadecimal
template <typename T> void print_table(
	const T* data,
	size_t data_size,
	std::string_view title,
	std::string_view tail,
	const unsigned int column_width,
	const unsigned int columns)
{
	if (data == NULL) return;

	std::string default_title = "                    ";
	const size_t row_header_sz = title.length() > default_title.length() ? title.length() : default_title.length(); // - Maximum between title lengths

	if (!title.empty()) std::cout << "\n" << std::left << std::setw(row_header_sz) << title << "  |";		            // - If provided, Print Title
	else std::cout << "\n" << std::left << std::setw(row_header_sz) << default_title << "  |";
	for (unsigned int i = 0; i < columns; ++i) {				                // - Print table header
		std::cout << std::setw((int)column_width) << std::hex << i << "|";	    // - Setting with to column_with
	}
	std::cout << '\n';

	for (size_t i = 0; i < data_size; ++i) {				                    // - Print Data Rows
		if (i % columns == 0) {						                            // - Print row header at the start of a new line
			std::stringstream ssh;
			if (i > 0) std::cout << '\n';
			// Use stringstream to format the line header
			ssh << std::hex << "[" << (i / columns) << "] " << "(" << i  << " -- " << i + columns << ")";
			std::cout << std::left << std::setw(row_header_sz) << ssh.str() << "  |" << std::right;
		}

		// Use stringstream to format the number and get its length for alignment
		std::stringstream ssd;
		ssd << std::hex;
		if(data[i] < 0) ssd << "-" << -data[i];
        else ssd << data[i];

		// Print with appropriate padding
		std::cout << std::setw((int)column_width) << ssd.str();

		// Print separator
		if (i < data_size - 1) {
			std::cout << ",";
		}
	}

	std::cout << tail << '\n';
}
