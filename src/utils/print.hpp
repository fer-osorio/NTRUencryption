#include<iostream>
#include<cstdint>
#include<cstring>

#ifndef PRINT_HPP
#define PRINT_HPP

#define HEXADECIMAL_BASE 16

union int64_to_char {                                                           // Allows to cast from an int64_t to an array of four bytes (char)
    int64_t int64;
    char    chars[8];
};
static int max(int a, int b) {
	if(a < b) return b;
	return a;
}
static int intToHexString(int n, char* dest) {                                  // String representation of unsigned integer it returns the length of the string
    int i = 0, j = 0, l = 0;
    char buff = 0;
    if(n < 0) {	dest[i++] = '-'; n = -n; j = 1; }
    do {
        buff = (char)(n % HEXADECIMAL_BASE);                                    // Taking last current digit
        if(buff < 10) dest[i++] = buff + 48;                                    // Saving last current digit
        else dest[i++] = buff + 55;
        n -= (int)buff; n /= HEXADECIMAL_BASE;                                  // Taking out last current digit from the number n
    } while(n > 0);
    l = i;
    dest[i--] = 0;                                                              // Putting a zero at the end and returning one place
    for(; j < i; j++,i--) {                                                     // The number is backwards; reversing the order of the digits
        buff = dest[j];
        dest[j] = dest[i];
        dest[i] = buff;
    }
    return l;                                                                   // Returning length
}
static int printSpaces(unsigned t) {
	while(t-- > 0) std::cout << ' ' ;
	return 0;
}										// Functions for printing
unsigned lengthHexadecimalInt(int a) {                                   // -Returns the number of decimal characters for the number a
    unsigned l = 0;
    do { a /= HEXADECIMAL_BASE; l++;
    }while(a > 0);
    return l;
}
void printIntArray(int* array, unsigned arrlen, unsigned columnlen, const char* name = "", const char* tail = "") {
    char buff[10];                                                              // Buffer necessary for the int -> string conversion
    int strLen = 0;                                                             // Start length in characters
    int i=0,j=0;
    int startLen = strlen(name);                                                // Length of the starting string
    int lastIndex = (int)arrlen - 1;

    if(startLen > 0) {
        std::cout << '\n' << name << "  |";
        startLen += 3;
        for(i = 0; i < 16; i++) {                                               // Printing the column number
            strLen = intToHexString(i, buff);                                   // Optimize by returning the length of the string
            printSpaces((unsigned)max((int)columnlen - strLen + 1,0));          // Padding with spaces. The +1 is for alignment with negative numbers
            std::cout << buff << '|';                                           // Printing current column number
        }
    }
    else {
        std::cout <<  "0    ";
        startLen = 5;
    }
    do {
        if(i != 0 && (i & 15) == 0) {                                           // Since 2^4 = 16, then i&15 = i % 16
            std::cout << '\n'; j++;                                             // New row; increasing number of rows
            if(i != lastIndex) {
                strLen = intToHexString(j, buff);
                std::cout << buff;                                              // Printing current line number
                printSpaces((unsigned)max(startLen - strLen - 1,0));            // Padding with spaces
                std::cout << '|';
            }
        }
        strLen = intToHexString(array[i], buff);                                // Optimize by returning the length of the string
        printSpaces((unsigned)max((int)columnlen - strLen + 1,0));              // Padding with spaces. The +1 is for alignment with negative numbers
        std::cout << buff;                                                      // Printing current coefficient
        if(i < lastIndex) std::cout << ',';
    }while(++i < (int)arrlen);
    std::cout << tail;
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

#endif