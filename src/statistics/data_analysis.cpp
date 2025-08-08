#include<iostream>
#include"../../include/ntru/parameters_constants.hpp"
#include"../../include/ntru/encryption.hpp"

#define BYTE_UPPER_TWO_BITS 0xC0     // binary -->1100,0000

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
static size_t print_obj_byte_view(const T& obj, NumberBaseTwosPower::Base b){
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
static size_t print_slice_centered(const std::vector<T>& v, size_t at_index, size_t slice_width, NumberBaseTwosPower::Base b){
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
static int print_first_difference(std::vector<char> v1, std::vector<char> v2, std::string diff_msg, std::string v1_slice_front, std::string v2_slice_front, size_t width){
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

StatisticalMeasures::DataRandomness NTRU::Encryption::encryptedData(const NTRU::Encryption& e, const std::vector<std::byte>& plain_data){
    const size_t plain_blk_sz = e.inputPlainTextMaxSizeInBytes();               // Plain data block size
    const size_t cipher_blk_sz = e.cipherTextSizeInBytes();                     // Cipher block size
    const size_t number_of_rounds = plain_data.size() > 0 ? plain_data.size() / plain_blk_sz: 0;
    const size_t enc_data_sz = cipher_blk_sz*(number_of_rounds + 1);            // Encrypted data size
    // Containers of plain and encypted data
    std::vector<char> plain_blk;
    std::vector<char> cipher_blk(cipher_blk_sz);
    std::vector<char> decrypted_blk(plain_blk_sz);
    std::vector<std::byte> enc_data(enc_data_sz, std::byte{0});
    size_t i, j, k, l, n;
    int dec_fail_indx = -2;
    RqPolynomial enc;
    RpPolynomial dec;
    StatisticalMeasures::DataRandomness dr;

    std::cout << "Encryption::encryptedData: Parameters: N = "<< NTRU_N << ", q = " << NTRU_Q << " ---------------------------------------" << std::endl;
    std::cout << "Input length = " << plain_data.size() << ". Encrypted input length = " << enc_data_sz << ". Block size = " << plain_blk_sz << '\n';

    for(i = k = n = 0, j = plain_blk_sz; j < plain_data.size(); i += plain_blk_sz, j += plain_blk_sz, k += cipher_blk_sz, n++){
        plain_blk = std::vector<char>(plain_data.begin() + i, plain_data.begin() + j);
        enc = e.encrypt(plain_blk.data(), plain_blk.size());
        enc.toBytes(cipher_blk.data());
        for(l = 0; l < cipher_blk_sz; l++) enc_data[k+l] = static_cast<std::byte>(cipher_blk[l]); // Save encrypted data block
        enc = RqPolynomial(cipher_blk.data(), cipher_blk.size());
        dec = e.decrypt(enc);
        Encryption::RpPolynomialtoBytes(dec, decrypted_blk.data(), false);      // -Second parameter isPrivateKey = false
        if(decrypted_blk != plain_blk){
            std::string dec_fail_msg = "Decryption failure at block " + std::to_string(n) + ".";
            dec_fail_indx = print_first_difference(plain_blk, decrypted_blk, dec_fail_msg, "Plain: ", "Decrypted: ", 16);
            return dr;
        }
    }
    if(j > plain_data.size()){
        plain_blk = std::vector<char>(plain_data.begin() + i, plain_data.end());
        enc = e.encrypt(plain_blk.data(), plain_blk.size());
        enc.toBytes(cipher_blk.data());
        for(l = 0; l < cipher_blk_sz; l++) enc_data[k+l] = static_cast<std::byte>(cipher_blk[l]); // Save encrypted data block
        enc = RqPolynomial(cipher_blk.data(), cipher_blk.size());
        dec = e.decrypt(enc);
        Encryption::RpPolynomialtoBytes(dec, decrypted_blk.data(), false);      // -Second parameter isPrivateKey = false
        if(decrypted_blk != plain_blk){
            std::string dec_fail_msg = "Decryption failure at block " + std::to_string(n) + ".";
            dec_fail_indx = print_first_difference(plain_blk, decrypted_blk, dec_fail_msg, "Plain: ", "Decrypted: ", 16);
            return dr;
        }
    }
    dr = StatisticalMeasures::DataRandomness(enc_data);
    return dr;
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
