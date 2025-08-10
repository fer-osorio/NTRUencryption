#include<vector>
#include<string>

#ifndef _DEBUG_HELPERS_
#define _DEBUG_HELPERS_

/*
    //  Representing the number bases that has the form 2^k, where k = 1,2,3,4.
    //  Main goal: To have convenient number bases to represent the bit values in a byte, using just bit masking and bit shifting.
    //  Considerations: Not intended as a general 2^k-form number base. Each digit should be capable of representing at most a number formed with four bits.
*/
struct NumberBaseTwosPower{
    enum struct Base{                                                           // Represent the number base with the maximum value that a single digit can hold,
        BINARY = 1,         // 0000,0001                                        // this result convenient at the moment of masking.
        QUATERNARY = 3,     // 0000,0011
        OCTAL = 7,          // 0000,0111
        HEXADECIMAL = 15    // 0000,1111
    };
    /*
        //  Maximum number of bits that a single digit hold, e,g, for OCTAL the output is 3.
    */
    static size_t max_bit_amount(Base b){
        size_t b_ = static_cast<size_t>(b), r;
        for(r = 1 ; b_ > 1; b_ >>= 1, r++) {}                                   // Count and delete (through right shift) each bit.
        return r;
    }
    /*
        //  Takes the 1's in the lower end and move them to the upper end, e.g. for QUATERNAY 0000,0011 -> 1100,0000.
        //  Considerations: Takes advantage of the form of its input (the bits are stacked to the right).
    */
    static unsigned char to_upper_bits(Base b){
        unsigned char r = static_cast<unsigned char>(b);
        while((r & 0x80) == 0) r <<= 1;                                         // Not guarding against b == 0, which in this case should not be possible.
        return r;
    }
};

/*
    //  View the passed object as an array of bytes, print the entire array using the specified base.
    //  Returns: Number of characters printed.
*/
template <typename T>
size_t print_obj_byte_view(const T& obj, NumberBaseTwosPower::Base b);

/*
    //  Prints the elements of interval specified by the at_index and slice_with parameters.
    //  arg at_index: Intended as the center of the specified interval, but there will be cases were this is not possible. As long as is smaller than v.size(), is
    //      guarantee to be printed.
    //  arg slice_with: Number of elements to be printed.
    //  Return: Viewing the printed output as a sequence of characters, returns the index of the starting character of the element specified by at_index.
*/
template <typename T>
size_t print_slice_centered(const std::vector<T>& v, size_t at_index, size_t slice_width, NumberBaseTwosPower::Base b);

/*
    //  Prints an interval of legth 'with' were the first difference [if exist] is found. If possible, the element were the first difference is found will be
    //      in the center of the interval.
    //  arg v1_slice_front: String printed in front (right) to the slice obtained from v1
    //  arg v2_slice_front: String printed in front (right) to the slice obtained from v2
    //  Returns:
    //      Case -2: Vectors are equal.
    //      Case -1: Vectors have different sizes.
    //      Case ">= 0": Vectors are different at the returned index.
*/
int print_first_difference(std::vector<char> v1, std::vector<char> v2, std::string diff_msg, std::string v1_slice_front, std::string v2_slice_front, size_t width);

// Printing array of bytes with an specific ofrmat. Format can be "binary" or char
// Format:
//	binary: Displays array binary notation
//	char: If posible, diplays bytes as printable character, otherwhise will print its hexadecimal value.
void displayByteArray(const char byteArray[], size_t size, size_t columnSize, const char format[]);
void displayByteArrayBin(const char byteArray[], size_t size);                  // -Printing array of bytes. Useful for debbuging
void displayByteArrayChar(const char byteArray[], size_t size);                 // -Prints each byte of array as a character or hexadecimal value
#endif