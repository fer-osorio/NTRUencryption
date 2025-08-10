#include<iostream>
#include<cstdint>
#include<cstring>
#include"../../include/print_debug/debug_helpers.hpp"

#define BYTE_UPPER_TWO_BITS 0xC0     // binary -->1100,0000
/*
    //  View the passed object as an array of bytes, print the entire array using the specified base.
    //  Returns: Number of characters printed.
*/
template <typename T>
size_t print_obj_byte_view(const T& obj, NumberBaseTwosPower::Base b){
    const unsigned char* byte_ptr = reinterpret_cast<const unsigned char*>(&obj); // Allows to view the object as an array of unsigned chars
    const size_t obj_sz = sizeof(obj);
    unsigned int buff = 0, lower_bits = static_cast<unsigned int>(b), bit_mask = 0, digit;
    const int bit_num = static_cast<int>(NumberBaseTwosPower::max_bit_amount(b));
    int right_displacement;                                                     //  Necessary amount of right bit shift to move the masked bits to the lower end.
    size_t num_printed_chars = 0;                                               //  Number of printed chars

    for(int i = obj_sz-1; i >= 0; i--){
        buff = byte_ptr[i];
        bit_mask = NumberBaseTwosPower::to_upper_bits(b);
        right_displacement = 8 - bit_num;
        if(b == NumberBaseTwosPower::Base::OCTAL) {                             //  Special case with octal base. A byte can be represented with three octal
            right_displacement = 6;                                             //  digits, but the left-most digit of this octal-representation is formed with the
            std::cout << ((buff & BYTE_UPPER_TWO_BITS) >> right_displacement);  //  two left-most bits of the byte (supposing the byte has its most significant
            bit_mask >>= 2;                                                     //  bits to the left).
            right_displacement -= bit_num;
            num_printed_chars += 2;
        }
        while(bit_mask >= lower_bits){
            digit = (buff & bit_mask) >> right_displacement;                    //  In each cycle, taking the corresponding (to the cycle) bits.
            if(digit < 10) std::cout << digit;
            else std::cout << static_cast<char>(digit + 55);                    //  In this case, print a hexadecimal character.
            bit_mask >>= bit_num;                                               //  Moving (inside the byte) to the next set of bits.
            right_displacement -= bit_num;
            num_printed_chars++;
        }
        if(i > 0){
            std::cout  << ',';                                                  //  Printing comma for better readabitily.
            num_printed_chars += 2;
        }
    }
    return num_printed_chars;
}

/*
    //  Prints the elements of interval specified by the at_index and slice_with parameters.
    //  arg at_index: Intended as the center of the specified interval, but there will be cases were this is not possible. As long as is smaller than v.size(), is
    //      guarantee to be printed.
    //  arg slice_with: Number of elements to be printed.
    //  Return: Viewing the printed output as a sequence of characters, returns the index of the starting character of the element specified by at_index.
*/
template <typename T>
size_t print_slice_centered(const std::vector<T>& v, size_t at_index, size_t slice_width, NumberBaseTwosPower::Base b){
    if(slice_width > v.size()) slice_width = v.size();                          // Ensuring 'slice_width <= v.size()' inequality
    if(at_index >= v.size()) return 0;                                          // Ensuring 'at_index < v.size()' inequality
    size_t sw_div2 = slice_width >> 1, sw_mod2 = slice_width & 1;               // Equivalent to slice_width / 2 and slice_width % 2
    size_t first_ind = 0;
    size_t last_ind  = 0;
    size_t obj_char_len = 0;
    size_t at_index_obj_poss = 0;

    // The idea is to have the output centered at 'at_index', but sometimes it will not be possible. The following if's handle those cases.
    if(sw_div2 >= at_index){                                                    // First case, not enough space to the left
        first_ind = 0;
        last_ind = slice_width;
    }else{
        if(at_index + sw_div2 + sw_mod2 >= v.size()){                           //  Second case, not enough space to the right.
            last_ind = v.size();                                                //  Notice how, in the impair width case, we are tilting the output to the right.
            first_ind = v.size() - slice_width;
        }else{                                                                  //  Third case, enough space in both sides.
            first_ind = at_index - sw_div2;
            last_ind = at_index + sw_div2 + sw_mod2;                            //  As mentioned before, in the impair width case, we are tilting the output to the
        }                                                                       //  right.
    }

    if(first_ind != 0) std::cout << "...";                                      //  Indicate that the array continues to the left
    else std::cout << "[  ";
    at_index_obj_poss += 3;
    // Sending coefficients to standard output.
    for(size_t i = first_ind, last_ind_1 = last_ind-1; i < last_ind; i++){
        obj_char_len = print_obj_byte_view(v[i], b);
        if(i < at_index) at_index_obj_poss += obj_char_len;
        if(i != last_ind_1){
            std::cout << ' ';                                                   //  Printing space for better readability
            if(i <= at_index) at_index_obj_poss++;
        }
    }
    if(last_ind != v.size()) std::cout << "...";                                //  Indicate that the array continues to the right
    else std::cout << "  ]";

    return at_index_obj_poss;
}

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
int print_first_difference(std::vector<char> v1, std::vector<char> v2, std::string diff_msg, std::string v1_slice_front, std::string v2_slice_front, size_t width){
    if(v1 == v2) return -2;
    size_t s1 = v1.size(), s2 = v2.size();
    if(s1 != s2) {
        std::cout << "Vectors differ in size. First vector size: " << s1 << ", second vector size: " << s2 << "\n";
        return -1;
    }
    size_t at_ind_elem_pos = 0;
    int num_spaces = 0;
    std::string first_occurr_msg = "Difference here";
    size_t left_offset = std::max( std::max(v1_slice_front.size(), v2_slice_front.size()), first_occurr_msg.size());
    size_t v1front_left_offset = left_offset - v1_slice_front.size();
    size_t v2front_left_offset = left_offset - v2_slice_front.size();
    size_t i;
    first_occurr_msg += " ~~^";                                                 //  This message will allow us to see the exact possition of the first difference
    for(i = 0; i < s1; i++){
        if(v1[i] != v2[i]){
            std::cout << diff_msg << '\n';
            std::cout << "Firs inequality found at index " << std::to_string(i) << '\n';
            std::cout << v1_slice_front;
            for(; v1front_left_offset > 0; v1front_left_offset--) std::cout << ' '; //  Padding to get an aligned output.
            at_ind_elem_pos = print_slice_centered(v1, i, width, NumberBaseTwosPower::Base::HEXADECIMAL);
            std::cout << '\n';
            std::cout << v2_slice_front;
            for(; v2front_left_offset > 0; v2front_left_offset--) std::cout << ' '; //  Padding to get an aligned output.
            print_slice_centered(v2, i, width, NumberBaseTwosPower::Base::HEXADECIMAL);
            std::cout << '\n';
            num_spaces = left_offset + at_ind_elem_pos - first_occurr_msg.length();
            for(; num_spaces > 0; num_spaces--) std::cout << ' ';
            std::cout << first_occurr_msg << std::endl;                         //  Indicating were the difference is.
            break;
        }
    }
    return static_cast<int>(i);
}


static void printByteArrayBin(const char byteArray[], size_t size){             // -Prints an array of bytes with no line break.
    uint8_t bit = 0x80;                                                         // -Byte 1000,0000
    size_t i = 0, t = 0, size_1 = size > 0? size-1: 0;
    if(byteArray == NULL) return;
    while(i < size){
        for(bit = 0x80; bit != 0; bit >>= 1){                                   // -Checking each byte bit by bit.
            t = bit & uint8_t(byteArray[i]);
            if(t != 0) std::cout << '1';
            else std::cout << '0';
        }
        if(i != size_1) std::cout << ',';                                       // -Putting a comma, except por the last byte.
        i++;
    }
}

static void printByteArrayChar(const char byteArray[], size_t size){            // -Printing byte array using ascii code
    uint8_t b = 0;
    if(byteArray == NULL) return;
    for(size_t i = 0; i < size; i++){
        b = (uint8_t)byteArray[i];
        if(b < 128) {                                                           // -Is inside the defined ascii code
            if(b > 32 && b < 127) printf("%c", byteArray[i]);                   // -If printable, prints character.
            else{
                printf("\033[0;33m<\033[0m");
                switch (b) {                                                    // -Handling whitespace characters
                    case '\t':
                        printf("\\t");                                          // -Tabulation
                        break;
                    case '\n':
                        printf("\\n");                                          // -New line
                        break;
                    case '\v':
                        printf("\\v");                                          // -Vertical tabulation
                        break;
                    case '\r':
                        printf("\\r");                                          // -Carriage return
                        break;
                    case '\f':
                        printf("\\f");                                          // -Form feed
                        break;
                    case ' ':
                        printf(" ");                                            // -Space
                        break;
                    default:
                        if (b == 2) printf("STX");                              // -Start of text
                        else if (b == 3)
                            printf("ETX");                                      // -End of text
                            else
                                printf("0x%02X", b);
                }
                printf("\033[0;33m>\033[0m");
            }
        } else printf("\033[0;33m<\033[0m0x%X\033[0;33m>\033[0m", b);           // -Not an ascii character
    }
}

void displayByteArray(const char byteArray[], size_t size, size_t columnSize, const char format[]){
    if(byteArray == NULL) return;
    if(size == 0) return;
    if(columnSize == 0) columnSize = size;
    size_t i = 0, columnSize_1 = columnSize-1;
    size_t completeRowsNum = size/columnSize, lastRowSz = size % columnSize, truncSize = completeRowsNum*columnSize;
    int form = -1;                                                              // -This will indicate the selected format. -1 is the default
    if(format == NULL) form = 0;                                                // -Zero for NULL array
    if(strcmp(format, "binary") == 0) form = 1;                                 // -1 for binary
    else if(strcmp(format, "char") == 0) form = 2;                              // -2 for ascii chars
    for(i = 0 ;i < truncSize; i+=columnSize){
        printf("\n[%03lu--%03lu]\t", i, i+columnSize_1);                        // -Printing labeled lines with 16 elements. For the moment (09-07-2025), this "3-digit-format" will work.
        if(form == 2) printByteArrayChar(byteArray + i, columnSize);            // -Case 2: ascii char
        else printByteArrayBin(byteArray + i, columnSize);                      // -Any other case will be considered as binary
    }
    if(lastRowSz > 0){
        printf("\n[%03lu--%03lu]\t", i, i+lastRowSz);                           // -Printing last row -in case of having it with non-zero elements-
        if(form == 2) printByteArrayChar(byteArray + i, lastRowSz);
        else printByteArrayBin(byteArray + i, lastRowSz);
    }
}

void displayByteArrayBin(const char byteArray[], size_t size){                  // -Printing array of bytes. Useful for debbuging
    displayByteArray(byteArray, size, 15, "binary");
}

void displayByteArrayChar(const char byteArray[], size_t size){                 // -Prints each byte of array as a character or hexadecimal value
    displayByteArray(byteArray, size, 20, "char");
}

/*int main(int argc, const char* argv[]){
    std::vector<char> char_arr_01 = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
    std::vector<char> char_arr_02 = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x1B,-0x02,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
    std::vector<char> char_arr_11 = {0x7F,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
    std::vector<char> char_arr_12 = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x1B,-0x02,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};

    std::vector<short> short_arr_1 = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,-0x14,0x15, 0x16,0x17,0x18};
    std::vector<short> short_arr_2 = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13, 0x14,0x15,-0x16,0x17,0x18};
    std::vector<int> int_arr_1 = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0xFFFF,0x0E,0x0F, 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
    std::vector<int> int_arr_2 = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x1B,0x0C,0x0D,  0x0E,0x0F,-0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};

    std::cout << "\nBinary: bits amount = " << NumberBaseTwosPower::max_bit_amount(NumberBaseTwosPower::Base::BINARY);
    printf("; upper bits: %X\n", (unsigned int)NumberBaseTwosPower::to_upper_bits(NumberBaseTwosPower::Base::BINARY));
    std::cout << "Quaternary: bits amount = " << NumberBaseTwosPower::max_bit_amount(NumberBaseTwosPower::Base::QUATERNARY);
    printf("; upper bits: %X\n\n", (unsigned int)NumberBaseTwosPower::to_upper_bits(NumberBaseTwosPower::Base::QUATERNARY));

    print_first_difference(char_arr_01, char_arr_02, "Vectors are diferent", "char_arr_01: ", "char_arr_02: ", 13);
    std::cout << std::endl;
    print_first_difference(char_arr_11, char_arr_12, "Vectors are diferent", "char_arr_11: ", "char_array_12: ", 18);
    std::cout << std::endl;

    print_slice_centered(short_arr_1, 22, 5, NumberBaseTwosPower::Base::BINARY);
    std::cout << std::endl;
    print_slice_centered(short_arr_1, 22, 8, NumberBaseTwosPower::Base::QUATERNARY);
    std::cout << std::endl;
    print_slice_centered(short_arr_2, 22, 8, NumberBaseTwosPower::Base::OCTAL);
    std::cout << std::endl;

    print_slice_centered(int_arr_1, 13, 9, NumberBaseTwosPower::Base::OCTAL);
    std::cout << std::endl;
    print_slice_centered(int_arr_2, 13, 9, NumberBaseTwosPower::Base::OCTAL);
    std::cout << std::endl;

    return 0;
}*/
