#include<fstream>
#include<random>
#include<chrono>
#include<ctime>
#include<cstring>
#include<exception>
#include"NTRUencryption.hpp"

/*#ifdef _MSC_VER
# include <intrin.h>
#else
# include <x86intrin.h>
#endif

// optional wrapper if you don't want to just use __rdtsc() everywhere
static uint64_t readTSC() {
    // _mm_lfence();  // optionally wait for earlier insns to retire before reading the clock
    uint64_t tsc = __rdtsc();
    // _mm_lfence();  // optionally block later instructions until rdtsc retires
    return tsc;
}*/

#define HEXADECIMAL_BASE 16
#define NUMBEROFROUNDS 16384                                                    // 2^14
#ifndef _N_
    #define _N_ 701
#endif
#ifndef _q_
    #define _q_ 8192
#endif

static void cerrMessageBeforeThrow(const char callerFunction[], const char message[]) {
    std::cerr << "In file Source/NTRUencryption.cpp, function " << callerFunction << ": " << message << '\n';
}

static void cerrMessageBeforeReThrow(const char callerFunction[], const char message[] = "") {
    std::cerr << "Called from: File Source/NTRUencryption.cpp, function " << callerFunction << " ..."<< message << '\n';
}

union int64_to_char {                                                           // Allows to cast from an int64_t to an array of four bytes (char)
    int64_t int64;
    char    chars[8];
};

inline static int max(int a, int b) {
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
}																				// Functions for printing
static int lengthString(const char* str) {										// Length of a string
	int l = -1;
	while(str[++l] != 0) {}
	return l;
}
static unsigned lengthHexadecimalInt(int a) {                                   // -Returns the number of decimal characters for the number a
    unsigned l = 0;
    do { a /= HEXADECIMAL_BASE; l++;
    }while(a > 0);
    return l;
}
static void printIntArray(int* array, unsigned arrlen, unsigned columnlen, const char* name = "", const char* tail = "") {
    char buff[10];                                                              // Buffer necessary for the int -> string conversion
    int strLen = 0;                                                             // Start length in characters
    int i=0,j=0;
    int startLen = lengthString(name);                                          // Length of the starting string
    int lastIndex = (int)arrlen - 1;

    if(startLen > 0) {
        printSpaces((unsigned)max(startLen + 4 - strLen,0));                    // Padding with spaces
        for(i = 0; i < 16; i++) {                                               // Printing the column number
            strLen = intToHexString(i, buff);                                   // Optimize by returning the length of the string
            printSpaces((unsigned)max((int)columnlen - strLen + 1,0));          // Padding with spaces. The +1 is for alignment with negative numbers
            std::cout << buff << ' ';                                                  // Printing current coefficient
        }
        std::cout << '\n' << name << " = [";
        startLen += 4;
    }
    else {
        std::cout <<  "0   [";
        startLen = 5;
    }
    do {
        if(i != 0 && (i & 15) == 0) {                                           // Since 2^4 = 16, then i&15 = i % 16
            std::cout << '\n'; j++;                                             // New row; increasing number of rows
            if(i != lastIndex) {
                strLen = intToHexString(j, buff);
                std::cout << buff;                                              // Printing current coefficient
                printSpaces((unsigned)max(startLen - strLen,0));                // Padding with spaces
            }
        }
        strLen = intToHexString(array[i], buff);                                // Optimize by returning the length of the string
        printSpaces((unsigned)max((int)columnlen - strLen + 1,0));              // Padding with spaces. The +1 is for alignment with negative numbers
        std::cout << buff;                                                      // Printing current coefficient
        if(i < lastIndex) std::cout << ',';
    }while(++i < (int)arrlen);
    std::cout << ']';
    std::cout << tail;
}

static bool compareStrings(const char* str1, const char* str2) {
    int i = 0;
    if(str1 == NULL || str2 == NULL) return false;
    while(str1[i] == str2[i] && str1[i] != 0) { i++; }
    return str1[i] == str2[i];
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

static void displayByteArray(const char byteArray[], size_t size, size_t columnSize, const char format[]){
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

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| NTRU Parameters |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

static int64_t q_1     = (int64_t)_q_-1;										// Will hold q-1, this will help with mod q operation
static int64_t negq_1  = ~q_1;													// This will help in the centering process. Set as ~q_1
static int64_t q_div_2 = _q_>>1;
static int     log2q   = NTRU::ZqPolynomial::log2((NTRU_q)_q_);

static int64_t modq(int64_t t) {											// operation t % q
    if(t >= 0)	return t & q_1;									// Equivalent to t % q since q is a power of 2
    else 		return (t | negq_1);								// Computing q - (-t%q) because -t = -(Q-1)q + (q-r) 0 <= r < q
}
static int64_t modsq(int64_t a) {
    int64_t r;
    if(a >= 0) r = a & q_1;										// Equivalent to a % q since q is a power of 2
        else r = (a | negq_1) & q_1;							// Computing q - (-a%q) because -t = -(Q-1)q + (q-r) 0 <= r < q
    if(r < q_div_2) return r;										// At this point we know 0 <= r < q
    else return r | negq_1;										// This is equivalent to r - this->q when r < q
}

class RandInt {                                                                 // Little class for random integers. Taken from The C++ Programming Language 4th
    static unsigned _seed_;
    public:                                                                     // Edition Bjarne Stroustrup
    RandInt(int low, int high): re(this->_seed_), dist(low,high) {}
    int operator()() { return dist(re); }                                       // Draw an int
    ~RandInt() { this->_seed_++; }
    private:
    std::default_random_engine re;
    std::uniform_int_distribution<> dist;
};
unsigned RandInt::_seed_ = (unsigned)time(NULL);
static RandInt randomIntegersN(0, _N_ - 1);

// ____________________________________________________________________ NTRU Parameters ___________________________________________________________________________

int NTRU::get_N(){ return _N_; }
int NTRU::get_q(){ return _q_; }
size_t NTRU::ZpPolynomialSizeBytes(){ return _N_/5 + 1; }                       // -Bytes required to write a full ZpPolynomial.
size_t NTRU::inputPlainTextMaxSizeBytes() { return _N_/6; }                     // -Maximum amount of (whole) bytes that can be written in a ZpPolynomial.
size_t NTRU::cipherTextSizeBytes(){ return size_t(_N_*log2q/8+1); }

using namespace NTRU;

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ZpPolynomial |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

ZpPolynomial::ZpPolynomial() {
	this->coefficients = new Z3[_N_];
	for(int i = 0; i < _N_; i++) this->coefficients[i] = _0_;
}

ZpPolynomial::ZpPolynomial(const ZpPolynomial& P) {
	this->coefficients = new Z3[_N_];
	for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
}

ZpPolynomial::ZpPolynomial(const char data[], int dataLength, bool isPlainText) {
    int i,j,k,l,m;
    this->coefficients = new Z3[_N_];
    for(i = 0; i < _N_; i++) this->coefficients[i] = _0_;
    if(dataLength <= 0) return;                                                 // -Guarding against negative or null dataLength
    isPlainText == true ? m = 6 : m = 5;

    for(i = 0, j = 0; i < dataLength && j < _N_; i++) {                         // -i will run through data, j through coefficients
        l = (int)(unsigned char)data[i];
        for(k = 0; k < m && j < _N_; k++, l/=3) {                               // -Here we're supposing _p_ == 3. Basically we're changing from base 2 to base 3
            switch(l%3) {                                                       //  in big endian notation. Notice that the maximum value allowed is 242
                case  1:                                                        // -One idea to solve this issue is to have a "flag" value, say 242. Suppose b is
                    this->coefficients[j++] = _1_;                              //  a byte such that b >= 242; then we can write b = 242 + (b - 242). The
                break;                                                          //  inequality 0 <= b - 242 <= 13 holds, so we will need 3 3-base digits to write
                case  2:                                                        //  that new value.
                    this->coefficients[j++] = _2_;
                break;
                default:
                    this->coefficients[j++] = _0_;
            }
        }
    }                                                                           // -The rest of the coefficients will be left as zero
}

ZpPolynomial ZpPolynomial::randomTernary() {
    const int d = _N_/3;
    int i, j, dd = d;
    ZpPolynomial r;

	r.coefficients = new Z3[_N_];
	for(i = 0; i < _N_; i++) r.coefficients[i] = _0_;
	while(dd > 0) {                                                             // Putting the ones first
        j = randomIntegersN();
        if(r.coefficients[j] == _0_) {
            r.coefficients[j] = _1_;
            dd--;
        }
	}
	dd = d;
	while(dd > 0) {                                                             // Then the negative ones
        j = randomIntegersN();
        if(r.coefficients[j] == _0_) {
            r.coefficients[j] = _2_;                                            // Two is congruent with -1 modulo 3
            dd--;
        }
	}
	return r;
}

void ZpPolynomial::interchangeZeroFor(Z3 k) {
    int i = randomIntegersN(), j;
    while(this->coefficients[i] != _0_) i = randomIntegersN();                  // -Looking for a zero in a random position
    j = i;
    while(this->coefficients[i] != k) i = randomIntegersN();                    // -Looking for a k in a random position
    this->coefficients[j] = k; this->coefficients[i] = _0_;                     // -Changing zero for k
}

void ZpPolynomial::changeZeroForOne() {
    int i = randomIntegersN();
    while(this->coefficients[i] != _0_) i = randomIntegersN();                  // -Looking for a zero in a random position
    this->coefficients[i] = _1_;                                                // -Changing zero for one
}

ZpPolynomial ZpPolynomial::getPosiblePrivateKey() {
    ZpPolynomial r = ZpPolynomial::randomTernary();                             // -Setting d = N/3, r is a polynomial with d 1's and d -1's
    r.changeZeroForOne();                                                       // -Adding one more one
    return r;
}

ZpPolynomial& ZpPolynomial::operator = (const ZpPolynomial& P) {
    if(this != &P) {													        // Guarding against self assignment
        if(this->coefficients != NULL) delete[] this->coefficients;
        this->coefficients = new Z3[_N_];
	    for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

int ZpPolynomial::operator [] (int i) const{
	if(i < 0) i = -i;
	if(i > _N_) i %= _N_;
	return this->coefficients[i];
}

int ZpPolynomial::degree() const{
    int deg = _N_;
	while(this->coefficients[--deg] == 0 && deg > 0) {}
	return deg;
}

static int64_t multiplyBy_3(int64_t t) {
    return (t << 1) + t;                                                        // -This expression is equivalent to t*2 + t
}

ZqPolynomial ZpPolynomial::encrypt(ZqPolynomial publicKey) const{
    ZqPolynomial encryption;
    int*  randTernaryTimes_p = new int[_N_];                                      // -Will represent the random polynomial needed for encryption
    const int d = _N_/3;
    int _d_ = d, i, j, k;

    for(i = 0; i < _N_; i++) randTernaryTimes_p[i] = 0;

    while(_d_ > 0) {
        i = randomIntegersN();                                                  // -Filling with threes. It represent the random polynomial multiplied by p
        if(randTernaryTimes_p[i] == 0) {randTernaryTimes_p[i] =  1; _d_--;}     //  ...
    }
    _d_ = d;
    while(_d_ > 0) {
        i = randomIntegersN();                                                  // -Filling with negative threes
        if(randTernaryTimes_p[i] == 0) {randTernaryTimes_p[i] = -1; _d_--;}     //  ...
    }
	for(i = 0; i < _N_; i++) {                                                    // -Convolution process
	    k = _N_ - i;
	    if(randTernaryTimes_p[i] != 0) {
	        if(randTernaryTimes_p[i] == 1) {
	            for(j = 0; j < k; j++)                                          // -Ensuring we do not get out of the polynomial
                    encryption.coefficients[i+j] += publicKey[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        encryption.coefficients[k] += publicKey[j];                 // Notice i+j = i + (k+_N_-i), so i+j is congruent with k mod _N_
	        }
	        if(randTernaryTimes_p[i] == -1) {
	            for(j = 0; j < k; j++)                                          // Ensuring we do not get out of the polynomial
                    encryption.coefficients[i+j] -= publicKey[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        encryption.coefficients[k] -= publicKey[j];                 // Notice i+j = i + (k+_N_-i), so i+j is congruent with k mod _N_
	        }
	    }
	}
	for(i = 0; i < _N_; i++) {                                                    // -Adding this polynomial (adding message)
	    if(this->coefficients[i] == _1_) encryption.coefficients[i]++;
	    if(this->coefficients[i] == _2_) encryption.coefficients[i]--;
	}
	encryption.mods_q();                                                        // -Obtaining center modulus for each coefficient
	delete[] randTernaryTimes_p;
	return encryption;
}

size_t ZpPolynomial::sizeInBytes(bool isPlainText) const{
    return isPlainText == true ? size_t(_N_/6 + 1) : size_t(_N_/5 + 1);
}

void ZpPolynomial::toBytes(char dest[], bool isPlainText) const{
    int i,j,k,m;
    int N_mod_m;
    int N_florm;
    int buff;
    isPlainText == true ? m = 6 : m = 5;
    N_mod_m = _N_ % m;                                                          // -Since _N_ is a prime number, N_mod_m is always bigger than 0
    N_florm = _N_ - N_mod_m;
    for(i = 0, j = 0; i < N_florm; i += m, j++) {                               // i will run through dest, j through coefficients
        for(k = m-1, buff = 0; k >= 0; k--) buff = buff*3 + this->coefficients[i+k];// Here we're supposing _p_ == 3. Basically we're changing from base 3 to base 2
        dest[j] = (char)buff;                                                   // Supposing the numbers in base 3 are in big endian notation
    }
    for(k = N_mod_m-1, buff = 0; k >= 0; k--) buff = buff*3 + this->coefficients[i+k];// Supposing the numbers in base 3 are in big endian notation
    dest[j] = (char)buff;
}

mpz_class ZpPolynomial::toNumber() const{                                       // -Interprets this->coefficients as a number in base 3
    mpz_class r = 0, base = 3;
    for(int i = _N_-1; i >= 0; i--) r = r*base + this->coefficients[i];         // -Horner's algorithm
    return r;
}

void ZpPolynomial::print(const char* name, bool centered, const char* tail) const{
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    if(centered) for(i = 0; i < coeffAmount; i++) if(array[i] == 2) array[i] = -1; // Printing the polynomials with coefficient in {-1, 0, 1}
    printIntArray(array, (unsigned)coeffAmount, 3, name, tail);
    delete[] array;
}

void ZpPolynomial::println(const char* name, bool centered) const{
	this->print(name, centered, "\n");
}

void ZpPolynomial::save(const char* name, bool saveAsText) const{
    int byteArrSize = ZpPolynomialSizeBytes();
    char* byteArr = NULL;
    const char ntrup[] = "NTRUp";                                               // -This will indicate the binary file is saving a NTRU (NTRU) polynomial with
    short N = _N_, q = _q_;
    std::ofstream file;                                                         //  coefficients in Zp (p)
    if(name == NULL) {
        if(saveAsText)  file.open("ZpPolynomial.ntrup");
        else            file.open("ZpPolynomial.ntrup",std::ios::binary);
    } else{
        if(saveAsText)  file.open(name);
        else            file.open(name, std::ios::binary);
    }
    if(file.is_open()) {
        file.write(ntrup, 5);                                                   // -The first five bytes are for the letters 'N' 'T' 'R' 'U' 'p'
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the degree of the polynomial
        byteArr = new char[byteArrSize];                                        // -The following bytes are for the polynomials coefficients
        this->toBytes(byteArr, true);                                           //
        file.write(byteArr, byteArrSize);
    } else {
        cerrMessageBeforeThrow("void ZpPolynomial::save(const char* name) const", "Could not create file for ZpPolynomial.");
        throw std::runtime_error("File writing failed.");
    }
    if(byteArr != NULL) delete[] byteArr;
}

void ZpPolynomial::writeFile(const char* fileName, bool writeAsText) const{
    int byteArrSize = ZpPolynomialSizeBytes();
    int fileSize = 0;
    char* byteArr = NULL;
    std::ofstream file;
    if(fileName == NULL) {                                                      // -Guarding against NULL pointer
        if(writeAsText) file.open("FromZpPolynomial.txt");
        else            file.open("FromZpPolynomial.bin",std::ios::binary);
    } else{
        if(writeAsText) file.open(fileName);
        else            file.open(fileName, std::ios::binary);
    }
    if(file.is_open()) {
        byteArr = new char[byteArrSize];                                        // -The following bytes are for the polynomials coefficients
        this->toBytes(byteArr, true);                                           // -Writing bytes in plain text mode.
        for(fileSize = byteArrSize - 1; fileSize >= 0; fileSize--){             // -We are supposing that this polynomial holds the data of a file, where the end
            if(byteArr[fileSize] == (char)0x80) {                               //  of the file content is marked by a byte with value 0x80 (1000,0000). This for
                break;
            }
        }
        if(fileSize > 0) file.write(byteArr, fileSize);
        else {
            std::cout << "In file NTRUencryption.cpp,  function void ZpPolynomial::writeFile(const char* fileName, bool writeAsText) const.\n";
            std::cout << "I could not find the mark for the end of the file (0x80). I will proceed to write the file using the entire polynomial." << std::endl;
            fileSize = byteArrSize;
        }
    } else {
        cerrMessageBeforeThrow("void ZpPolynomial::writeFile(const char* fileName, bool writeAsText) const", "Could not write file from ZpPolynomial.");
        throw std::runtime_error("File writing failed.");
    }
    if(byteArr != NULL) delete[] byteArr;
}

ZpPolynomial ZpPolynomial::fromFile(const char* fileName){
    const char thisFunc[] = "ZpPolynomial ZpPolynomial::fromFile(const char* fileName)";
    const char ntrup[] = "NTRUp";                                               // -This will indicate the binary file is saving a NTRU (NTRU) ZpPolynomial
    int byteArrSize = inputPlainTextMaxSizeBytes() + 1, ntrupsz = strlen(ntrup);// -_N_ is necessarily prime, so _N_ % 5 > 0 holds true
    char* byteArr = NULL;
    short n = _N_, Q = _q_;
    ZpPolynomial out;
    std::ifstream file = std::ifstream();
    file.open(fileName, std::ios::binary);
    if(!file.is_open()){
        cerrMessageBeforeThrow(thisFunc, "Error opening file for polynomial creation");
        throw std::runtime_error("File opening failed");
    }
    byteArr = new char[ntrupsz+1];
    file.read(byteArr, ntrupsz);                                                // -Reading the firsts bytes hoping "NTRUpublicKey" or "NTRUprivatKey" apears
    byteArr[ntrupsz] = 0;                                                       // -End of string
    if(compareStrings(byteArr, ntrup)) {                                        // -Testing if file saves a NTRU private key
        delete[] byteArr; byteArr = NULL;
        file.read((char*)&n, 2);
        file.read((char*)&Q, 2);
        if(n != _N_ || Q != _q_) {
            delete[] byteArr; byteArr = NULL;
            cerrMessageBeforeThrow(thisFunc, "Parameters retreaved from file do not match with this program parameters");
            std::cout << "From file: N == " << n << ", q == " << Q << ". From this program: N == " << _N_ << ", q == " << _q_ << '\n';
            throw std::runtime_error("Could not agree on parameters");
        }
        byteArr = new char[byteArrSize];
        file.read(byteArr, byteArrSize);                                        // -Reading the coefficients of the polynomial
        file.close();
        out = ZpPolynomial(byteArr, byteArrSize, true);                         // -Building polynomials in plintext mode
        delete[] byteArr; byteArr = NULL;
    } else{
        cerrMessageBeforeThrow(thisFunc, "Not a valid NTRU::ZpPolynomial file.");
        throw std::runtime_error("Not valid input file.");
    }
    return out;
}

//_______________________________________________________________________ ZpPolynomial ____________________________________________________________________________

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Z2Polynomial ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

Z2Polynomial::Z2Polynomial() {
	this->coefficients = new Z2[_N_];
	for(int i = 0; i < _N_; i++) this->coefficients[i] = _0_;
}

Z2Polynomial::Z2Polynomial(const Z2Polynomial& P) {
    this->coefficients = new Z2[_N_];
    for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
}

Z2Polynomial::Z2Polynomial(const ZpPolynomial& P) {
	this->coefficients = new Z2[_N_];
	for(int i = 0; i < _N_; i++) {
	    if(P[i] != ZpPolynomial::_0_) this->coefficients[i] = _1_;              // Two won't go to zero because it's the additive inverse of one in Z3, therefore
	    else this->coefficients[i] = _0_;                                       // it must go to the additive inverse of one in Z2, which if itself
    }
}

Z2Polynomial& Z2Polynomial::operator = (const Z2Polynomial& P)  {
	if(this != &P) {													        // Guarding against self assignment
		if(this->coefficients != NULL) delete[] this->coefficients;
		this->coefficients = new Z2[_N_];
		for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

Z2Polynomial& Z2Polynomial::operator = (const ZpPolynomial& P) {
	if(this->coefficients != NULL) delete[] this->coefficients;                 // In case of having an object created with the default (private) constructor
	this->coefficients = new Z2[_N_];
	for(int i = 0; i < _N_; i++) {                                                // Fitting the ZpPolynomial in a Z2Polynomial
	    if(P[i] != ZpPolynomial::_0_) this->coefficients[i] = _1_;
	    else this->coefficients[i] = _0_;
	}
	return *this;
}

Z2Polynomial& Z2Polynomial::operator = (Z2 t)  {
	if(this->coefficients != NULL) delete[] this->coefficients;                 // In case of have been generated from the (private) default constructor
	this->coefficients = new Z2[_N_];
	this->coefficients[0] = t;
	for(int i = 1; i < _N_; i++) this->coefficients[i] = _0_;
	return *this;
}

Z2Polynomial Z2Polynomial::operator + (const Z2Polynomial& P) const {
    Z2Polynomial r;                                                             // -Initializing result with zeros
    for(int i = 0; i < _N_; i++)
        r.coefficients[i] = this->coefficients[i] + P.coefficients[i];          // -Addition element by element till the smallest degree of the arguments
    return r;
}

Z2Polynomial Z2Polynomial::operator - (const Z2Polynomial& P) const{
    return *this + P;                                                           // In this polynomial ring, subtraction coincide with addition
}

Z2Polynomial Z2Polynomial::operator * (const Z2Polynomial& P) const{ // Classical polynomial multiplication algorithm
    Z2Polynomial r;                                                             // -Initializing with zeros
    int i, j, k, l;

	for(i = 0; i < _N_; i++) {
		if(this->coefficients[i] != _0_) {                                      // -Polynomial over binary field, here we know  this->coefficients[i] is 1
		    k = _N_ - i;
		    for(j = 0; j < k; j++) {                                            // -Adding and multiplying while the inequality i+j < N holds
		        l = i+j;
			    if(P.coefficients[j] != r.coefficients[l]) r.coefficients[l] = _1_;
			    else r.coefficients[l] = _0_;
		    }
		    for(k = 0; k < i; j++, k++) {                                       // Using the definition of convolution polynomial rings
			    if(P.coefficients[j] != r.coefficients[k]) r.coefficients[k] = _1_; // Notice i+j = i +(k+N-i), so i+j is congruent with k mod N
			    else r.coefficients[k] = _0_;
		    }
		}
	}
	return r;
}

void Z2Polynomial::division(const Z2Polynomial& P, Z2Polynomial result[2]) const{
    const char thisFunc[] = "void Z2Polynomial::division(const Z2Polynomial& P, Z2Polynomial result[2]) const";
    if(P == _0_) {
        cerrMessageBeforeThrow(thisFunc, "Trying divide by zero.");
        throw std::runtime_error("Division by Zero");
    }
    if(*this == _0_) {                                                          // Case zero divided by anything
        result[0] = _0_;                                                        // Zero polynomial
        result[1] = _0_;                                                        // Zero polynomial
        return;
    }

    const int dividendDegree = this->degree();
    const int divisorDegree  = P.degree();
    const Z2 leadCoeffDivsrInv = P.coefficients[divisorDegree];                 // In Z3, each element is its own inverse
    int degreeDiff;                                                             // Difference between degrees
    int remDeg;                                                                 // Remainder degree
    int i;                                                                      // For counting

    if(dividendDegree < divisorDegree) {                                        // Case dividend has smaller degree than divisor
        result[0] = _0_; result[1] = *this;
        return;
    }                                                                           // At this point we know leading coefficient has an inverse in Zq

    degreeDiff = dividendDegree - divisorDegree;                                // At this point we know degreeDiff >= 0
    remDeg = dividendDegree;
    result[1] = *this;                                                          // Initializing remainder with dividend (this)
    result[0] = Z2Polynomial();
    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        result[0].coefficients[degreeDiff] =
        leadCoeffDivsrInv * result[1].coefficients[remDeg];                     // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] -=
            result[0].coefficients[degreeDiff]*P.coefficients[i-degreeDiff];
        }

        if(result[1].coefficients[remDeg] != _0_) {                             // No congruence with 0 mod q, throwing exception
            cerrMessageBeforeThrow(thisFunc, "result[1]. coefficients[remDeg] != 0.");
            throw std::runtime_error("Exception in division process.");
        }                                                                       // At this point we know result[1].coefficients[remDeg] = 0
        while(remDeg >= 0 && result[1].coefficients[remDeg] == _0_) remDeg--;   // Updating value of the degree of the remainder
    }
}

Z2Polynomial Z2Polynomial::gcdXNmns1(Z2Polynomial& thisBezout) const{
    Z2Polynomial gcd;                                                           // Initializing result with zeros
    Z2Polynomial remainders;
    Z2Polynomial tmp[2] = {Z2Polynomial(), Z2Polynomial()};
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    Z2 leadCoeff = this->coefficients[deg];                                     // Lead coefficient of this polynomial
    Z2Polynomial quoRem[2]={Z2Polynomial(), Z2Polynomial()};

    quoRem[0].coefficients[_N_-deg] = leadCoeff;                                  // Start of division algorithm between virtual polynomial x^N-1 and this
    for(i = deg-1, j = _N_ - 1; i >= 0; i--, j--) {                               // First coefficient of quotient and first subtraction
        quoRem[1].coefficients[j] = this->coefficients[i];                      // All x in Z2, -x = x
    }
    quoRem[1].coefficients[0] = _1_;                                            // Putting the -1 that is at the end of the polynomial x^N-1. 1 == -1 in Z2
    for(i = _N_-1 - deg, j = _N_-1; j >= deg; i = j - deg) {                        // Continuing with division algorithm; i is the place of the next coefficient of
        quoRem[0].coefficients[i] = leadCoeff * quoRem[1].coefficients[j];      // the quotient, j is the degree of the remainders.
        for(k = deg, l = j; k >= 0; k--, l--) {                                 // Multiplication-subtraction step
            quoRem[1].coefficients[l] -=
            quoRem[0].coefficients[i] * this->coefficients[k];
        }
        while(quoRem[1].coefficients[j] == _0_) {j--;};
    }                                                                           // End of division algorithm between virtual polynomial x^N-1 and this

    thisBezout = _1_;                                                           // Initializing values for the execution of the rest of the EEA
    tmp[1] = quoRem[0];                                                         // In this ring, -a = a for all polynomial a
    tmp[0] = tmp[1];                                                            // v[-1] = 0, v[0] = 1 ==> v[1] = v[-1] - q[1]*v[0] = - q[1]
    gcd = *this;                                                                // ...
    remainders = quoRem[1];                                                     // ...

	while(remainders != _0_) {                                                  // EEA implementation (continuation)
        try{ gcd.division(remainders, quoRem); }
        catch(const std::runtime_error&) {
            cerrMessageBeforeReThrow("ZpPolynomial ZpPolynomial::gcdXNmns1(ZpPolynomial& thisBezout) const");
            throw;
        }
        tmp[1] = thisBezout - quoRem[0]*tmp[0];                                 // u[k+2] = u[k] - q[k+2]*u[k+1]
        thisBezout = tmp[0];                                                    // Updating values
        tmp[0] = tmp[1];                                                        // ...
        gcd = remainders;                                                       // ...
        remainders = quoRem[1];                                                 // ...
	}
	return gcd;
}

bool Z2Polynomial::operator == (const Z2Polynomial& P) const{
	for(int i = 0; i < _N_; i++) if(this->coefficients[i] != P.coefficients[i])  return false;
	return true;
}

Z2Polynomial::Z2 Z2Polynomial::operator[](int i) const{
	if(i < 0) i = -i;
	if(i >= _N_) i %= _N_;
	return this->coefficients[i];
}

int Z2Polynomial::degree() const{												// Returns degree of polynomial
	int deg = _N_;
	while(this->coefficients[--deg] == _0_ && deg > 0) {}
	return deg;
}

void Z2Polynomial::print(const char* name,const char* tail) const{
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printIntArray(array, (unsigned)coeffAmount, 2, name, tail);
    delete[] array;
}

void Z2Polynomial::println(const char* name) const{
    this->print(name, "\n");
}

//________________________________________________________________________Z2Polynomial_____________________________________________________________________________

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ZqPolynomial ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

ZqPolynomial::ZqPolynomial() {
    this->coefficients = new int64_t[_N_];
    for(int i = 0; i < _N_; i++) this->coefficients[i] = 0;
}

ZqPolynomial::ZqPolynomial(const ZqPolynomial& P) {
    this->coefficients = new int64_t[_N_];
    for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
}

ZqPolynomial::ZqPolynomial(const char data[], int dataLength) {
    int bitsOcupiedInBuff = 0;
    int i , j, dataByteIndex;
    int offset = 0;
    int64_to_char aux;
    uint64_t buff = 0;

    this->coefficients = new int64_t[_N_];

    for(i = 0, dataByteIndex = 0; dataByteIndex < dataLength && i < _N_ ;) {
        aux.int64 = 0;
        for(j = 0; bitsOcupiedInBuff <= 56 && dataByteIndex < dataLength; bitsOcupiedInBuff += 8) {// -If we are using at most 56 bits of the buffer, we can
            aux.chars[j++] = data[dataByteIndex++];                             //  still allocate one more byte
        }
        buff |= (uint64_t)aux.int64 << offset;
        for(; bitsOcupiedInBuff >= log2q; bitsOcupiedInBuff -= log2q, i++) {
            this->coefficients[i] = (int64_t)buff & q_1;                        // -Taking the first log2q bits of buff. The result will be positive
            if(this->coefficients[i] >= q_div_2) this->coefficients[i] |= negq_1; // This is equivalent to r - this->q when r < q
            buff >>= log2q;
        }
        offset = bitsOcupiedInBuff;
    }
    if(bitsOcupiedInBuff > 0) {
        if(i < _N_) {
            this->coefficients[i] = (int64_t)buff & q_1;                        // -Taking the first log2q bits of buff. The result will be positive
            if(this->coefficients[i] >= q_div_2) this->coefficients[i] |= negq_1; // This is equivalent to r - this->q when r < q
            buff >>= log2q;
            i++;
        }
    }
    for(;i < _N_; i++) this->coefficients[i] = 0;                                 // -Padding the rest of the polynomial with zeros
}

ZqPolynomial& ZqPolynomial::operator = (const ZqPolynomial& P) {
	if(this != &P) {														    // Guarding against self assignment
		if(this->coefficients != NULL) delete[] this->coefficients;
		this->coefficients = new int64_t[_N_];
		for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

int64_t ZqPolynomial::operator [] (int i) const{
	if(i < 0) i = -i;
	if(i > _N_) i %= _N_;
	return this->coefficients[i];
}

ZqPolynomial ZqPolynomial::operator + (const ZqPolynomial& P) const{
    ZqPolynomial r;                                                             // -Initializing with the zero polynomial
    for(int i = 0; i < _N_; i++)
        r.coefficients[i] = modq(this->coefficients[i] + P.coefficients[i]);          // -Addition element by element till the smallest degree of the arguments
    return r;
}

ZqPolynomial ZqPolynomial::operator * (const ZqPolynomial& P) const{
    ZqPolynomial r;
    int i, j, k;

	for(i = 0; i < _N_; i++) {
		k = _N_ - i;
	    for(j = 0; j < k; j++)                                                  // Ensuring we do not get out of the polynomial
		    r.coefficients[i+j] += this->coefficients[i] * P.coefficients[j];
	    for(k = 0; k < i; j++, k++)                                             // Using the definition of convolution polynomial ring
		    r.coefficients[k] += this->coefficients[i] * P.coefficients[j];
	}
	r.mod_q();                                                                  // Applying mod q
	return r;
}

ZqPolynomial NTRU::operator - (int64_t t, const ZqPolynomial& P) {
    ZqPolynomial r;
    r.coefficients[0] = modq(t - P.coefficients[0]);
    for(int i = 1; i < _N_; i++) r.coefficients[i] = -P.coefficients[i];
    return r;
}

ZpPolynomial NTRU::mods_p(ZqPolynomial P) {
    ZpPolynomial r;
    for(int i = 0, buff = 0; i < _N_; i++) {
        buff = P[i] % 3;
        if(buff == -2 || buff == 1) r.coefficients[i] = ZpPolynomial::_1_;
        if(buff == -1 || buff == 2) r.coefficients[i] = ZpPolynomial::_2_;
    }
    return r;
}

ZqPolynomial NTRU::convolutionZq(const Z2Polynomial& z2P, const ZpPolynomial& zpP) {
    ZqPolynomial r;
    int i, j, k;

    for(i = 0; i < _N_; i++) {
        if(z2P.coefficients[i] != Z2Polynomial::Z2::_0_) {
            k = _N_ - i;
		    for(j = 0; j < k; j++)
		        if(zpP.coefficients[j] != 0) {
		            if(zpP.coefficients[j] == 1) ++r.coefficients[i+j];
		            else --r.coefficients[i+j];
		        }
		    for(k = 0; k < i; j++, k++)
		        if(zpP.coefficients[j] != 0) {
		            if(zpP.coefficients[j] == 1) ++r.coefficients[k];
		            else --r.coefficients[k];
		        };
        }
    }
    r.mod_q();                                                                  // Applying mods q
    return r;
}

ZqPolynomial NTRU::convolutionZq(const Z2Polynomial& z2P, const ZqPolynomial& zqP) {
    ZqPolynomial r;
    int i, j, k;

    for(i = 0; i < _N_; i++) {
        if(z2P.coefficients[i] != Z2Polynomial::Z2::_0_) {
            k = _N_ - i;
		    for(j = 0; j < k; j++)
		        r.coefficients[i+j] += zqP.coefficients[j];
		    for(k = 0; k < i; j++, k++)
		        r.coefficients[k] += zqP.coefficients[j];
        }
    }
    r.mod_q();                                                                 // Applying mods q
    return r;
}

ZqPolynomial NTRU::convolutionZq(const ZpPolynomial& p1, const ZqPolynomial& p2) {
    ZqPolynomial r;
    int i, j, k;

	for(i = 0; i < _N_; i++) {
		if(p1.coefficients[i] != ZpPolynomial::_0_) {                           // -Taking advantage this polynomials have a big proportion of zeros
		    if(p1.coefficients[i] == ZpPolynomial::_1_) {                       // -The other two cases are one, as in this line is showed
		        k = _N_ - i;
		        for(j = 0; j < k; j++)
		            r.coefficients[i+j] += p2.coefficients[j];
		        for(k = 0; k < i; j++, k++)
		            r.coefficients[k] += p2.coefficients[j];
		    } else {                                                            // -The only other case is two, which is interpreted as -1
		        k = _N_ - i;
		        for(j = 0; j < k; j++)
		            r.coefficients[i+j] -= p2.coefficients[j];
		        for(k = 0; k < i; j++, k++)
		            r.coefficients[k] -= p2.coefficients[j];
		    }
		}
	}
	r.mod_q();                                                                 // Applying mods q
	return r;
}

int ZqPolynomial::degree() const{											    // -Returns degree of polynomial
	int deg = _N_;
	while(this->coefficients[--deg] == 0 && deg > 0) {}
	return deg;
}

bool ZqPolynomial::equalsOne() const{
    if(this->coefficients[0] != 1) return false;
    for(size_t i = 1; i < _N_; i++) if(this->coefficients[i] != 0) return false;
    return true;
}

void ZqPolynomial::mod_q() const{
    for(int i = 0; i < _N_; i++) this->coefficients[i] = modq(this->coefficients[i]);
}

void ZqPolynomial::mods_q() const{
    for(int i = 0; i < _N_; i++) this->coefficients[i] = modsq(this->coefficients[i]);
}

int ZqPolynomial::log2(NTRU_q q) {                                              // -Returns logarithm base 2 of a NTRU_q value
    int log2q = 0, qq = q;
    for(; qq > 1; qq >>= 1, log2q++) {}
    return log2q;
}

int ZqPolynomial::lengthInBytes() const{
    return _N_*log2q/8 + 1;
}

ZqPolynomial ZqPolynomial::getNTRUpublicKey() {                                 // -Returns public key provided this ZqPolynomial object is the inverse mod q in
    ZqPolynomial publicKey;                                                     //  Z[x]/X^N-1 of the private key
    int* randTernary = new int[_N_];
    const int d_ = _N_/3;
    int _d_ = d_;
    int i, j, k;

    for(i = 0; i < _N_; i++) randTernary[i] = 0;

    while(_d_ > 0) {                                                            // -Building ternary polynomial multiplied by p
        i = randomIntegersN();                                                  // -Filling with p
        if(randTernary[i] == 0) {randTernary[i] =  1; _d_--;}                   // ...
    }
    _d_ = d_;
    while(_d_ > 0) {
        i = randomIntegersN();                                                  // -Filling with negative p
        if(randTernary[i] == 0) {randTernary[i] = -1; _d_--;}                   // ...
    }
	for(i = 0; i < _N_; i++) {                                                    // -Convolution process
	    k = _N_ - i;
	    if(randTernary[i] != 0) {
	        if(randTernary[i] == 1) {
	            for(j = 0; j < k; j++)                                          // Ensuring we do not get out of the polynomial
                    publicKey.coefficients[i+j] += this->coefficients[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        publicKey.coefficients[k] += this->coefficients[j];         // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
	        } else {
	            for(j = 0; j < k; j++)                                          // Ensuring we do not get out of the polynomial
                    publicKey.coefficients[i+j] -= this->coefficients[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        publicKey.coefficients[k] -= this->coefficients[j];         // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
	        }
	    }
	}
	delete[] randTernary;
	publicKey.mods_q();                                                         // -Applying mods q
	return publicKey;
}

ZqPolynomial NTRU::ZqPolynomial::timesThree(const ZpPolynomial& p){
    ZqPolynomial r;
    for(int i = 0; i < _N_; i++){
        if(p[i] != 0) r.coefficients[i] = multiplyBy_3(p[i]);                   // -Getting 3p
    }
    return r;
}

void ZqPolynomial::toBytes(char dest[]) const{                                  // -Supposing dest is pointing to a suitable memory location
    const int buffBitsSize = 64;
    int i = 0, j = 0;                                                           // -log2q will hold the logarithm base two of q. Here we are assuming q < 2^32)
    int bitsAllocInBuff = 0;                                                    // -Amount of bits allocated (copied from coefficients array) in buffer
    int bytesAllocInDest = 0;
    int64_to_char buffer = {0};                                                 // -buffer will do the cast from int to char[]
    int64_t aux;
    //std::cout << "ZqPolynomial::toBytes(char dest[]) const: log2q = " << log2q << '\n'; // -Debugging purposes
    for(bitsAllocInBuff = 0, aux = 0; i < _N_;) {
        buffer.int64 >>= (bytesAllocInDest << 3);                               // -l*8; Ruling out the bits allocated in the last cycle
        while(bitsAllocInBuff < buffBitsSize - log2q) {                         // -Allocating bits from coefficients to buffer
            aux = this->coefficients[i++];
            if(aux < 0) aux += _q_;
            buffer.int64 |= aux << bitsAllocInBuff;                             // -Allocating log2q bits in buffer._int_;
            bitsAllocInBuff += log2q;                                           // -increasing amount of bits allocated in buffer
            if(i >= _N_) break;
        }
        for(bytesAllocInDest = 0; bitsAllocInBuff >= 8; bytesAllocInDest++, bitsAllocInBuff -= 8)
            dest[j++] = buffer.chars[bytesAllocInDest];                         // -Writing buffer in destination (as long there are at least 8 bits in buffer)
    }
    if(bitsAllocInBuff > 0) dest[j++] = buffer.chars[bytesAllocInDest];         // -The bits that remain in buffer, we allocate them in the last byte
}

void ZqPolynomial::print(const char* name,const char* tail) const{
    unsigned len_q = lengthHexadecimalInt(_q_);
    int coeffAmount = this->degree() + 1;                                       // -This three lines is a "casting" from int64_t array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printIntArray(array, (unsigned)coeffAmount, len_q+1, name, tail);
    delete[] array;
}

void ZqPolynomial::println(const char* name) const{
    this->print(name, "\n");
}

void ZqPolynomial::save(const char* name, bool saveAsText) const{
    int byteArrSize = cipherTextSizeBytes();
    char* byteArr = NULL;
    const char ntruq[] = "NTRUq";                                               // -This will indicate the binary file is saving a Zq NTRU (NTRU) polynomial
    short N = _N_, q = _q_;

    std::ofstream file;                                                         //  coefficients in Zp (p)
    if(name == NULL) {
        if(saveAsText)  file.open("ZpPolynomial.ntrup");
        else            file.open("ZpPolynomial.ntrup",std::ios::binary);
    } else{
        if(saveAsText)  file.open(name);
        else            file.open(name, std::ios::binary);
    }
    if(file.is_open()) {
        file.write(ntruq, strlen(ntruq));
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the q value
        byteArr = new char[byteArrSize];
        this->toBytes(byteArr);
        file.write(byteArr, byteArrSize);
    } else {
        cerrMessageBeforeThrow("void ZpPolynomial::save(const char* name) const", "Could not create file for ZqPolynomial.");
        throw std::runtime_error("File writing failed.");
    }
    if(byteArr != NULL) delete[] byteArr;
}

ZqPolynomial ZqPolynomial::fromFile(const char* fileName){
    const char thisFunc[] = "ZqPolynomial ZqPolynomial::fromFile(const char* fileName)";
    const char ntruq[] = "NTRUq";                                               // -This will indicate the binary file is saving a NTRU (NTRU) ZqPolynomial
    int byteArrSize = cipherTextSizeBytes(), ntruqsz = strlen(ntruq);
    char* byteArr = NULL;
    short n = _N_, Q = _q_;
    ZqPolynomial out;
    std::ifstream file;
    file.open(fileName, std::ios::binary);
    if(!file.is_open()){
        cerrMessageBeforeThrow(thisFunc, "Error opening file for polynomial creation");
        throw std::runtime_error("File opening failed");
    }
    byteArr = new char[ntruqsz+1];
    file.read(byteArr, ntruqsz);                                                // -Reading the firsts bytes hoping "NTRUpublicKey" or "NTRUprivatKey" apears
    byteArr[ntruqsz] = 0;                                                       // -End of string
    if(compareStrings(byteArr, ntruq)) {                                        // -Testing if file saves a NTRU private key
        delete[] byteArr; byteArr = NULL;
        file.read((char*)&n, 2);
        file.read((char*)&Q, 2);
        if(n != _N_ || Q != _q_) {
            delete[] byteArr; byteArr = NULL;
            cerrMessageBeforeThrow(thisFunc, "Parameters retreaved from file do not match with this program parameters");
            std::cout << "From file: N == " << n << ", q == " << Q << ". From this program: N == " << _N_ << ", q == " << _q_ << '\n';
            throw std::runtime_error("Could not agree on parameters");
        }
        byteArr = new char[byteArrSize];
        file.read(byteArr, byteArrSize);                                        // -Reading the coefficients of the polynomial
        file.close();
        out = ZqPolynomial(byteArr, byteArrSize);                               // -Building polynomials in plintext mode
        delete[] byteArr; byteArr = NULL;
        return out;
    } else{
        cerrMessageBeforeThrow(thisFunc, "Not a valid NTRU::ZqPolynomial file.");
        throw std::runtime_error("Not valid input file.");
    }
}

ZqPolynomial& ZqPolynomial::timesThree(){
    for(int i = 0; i < _N_; i++){
        this->coefficients[i] = multiplyBy_3(this->coefficients[i]);            // -Getting 3p
    }
    return *this;
}

//_______________________________________________________________________ ZqPolynomial ___________________________________________________________________________

// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Encryption ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

Encryption::Encryption() {                                                      // -Just for type declaration. Should not be used just like this
    try {
	    this->setKeys();
	}catch(const std::runtime_error&) {
	    this->validPrivateKey = false;
	    cerrMessageBeforeReThrow("Encryption::Encryption()");
	    throw;
	}
	this->validPrivateKey = true;
}

Encryption::Encryption(const char* NTRUkeyFile) {
    const char thisFunc[] = "Encryption::Encryption(const char* NTRUkeyFile)";
    const char NTRUpublicKey[] = "NTRUpublicKey";                               // -This will indicate the binary file is saving a NTRU public key
    const char NTRUprivatKey[] = "NTRUprivatKey";                               // -This will indicate the binary file is saving a NTRU private key
    char* coeffBytes = NULL;
    char* fileHeader = NULL;
    int   sz = 0, headerSz = lengthString("NTRUpublicKey");                     // -Notice how "NTRUpublicKey" and "NTRUprivatKey" strings have the same length
    bool  isPrivateKey = false;
    short n;
    short Q;
    std::ifstream file;
    file.open(NTRUkeyFile, std::ios::binary);
    if(file.is_open()) {
        fileHeader = new char[headerSz + 1];
        file.read(fileHeader, headerSz);                                        // -Reading the firsts bytes hoping "NTRUpublicKey" or "NTRUprivatKey" apears
        fileHeader[headerSz] = 0;                                               // -End of string
        if(compareStrings(fileHeader, NTRUprivatKey) || compareStrings(fileHeader, NTRUpublicKey)) { // -Testing if file saves a NTRU private key
            file.read((char*)&n, 2);
            file.read((char*)&Q, 2);
            if(n != _N_ || Q != _q_) {
                delete[] fileHeader; fileHeader = NULL;
                cerrMessageBeforeThrow(thisFunc, "Parameters retreaved from file do not match with this program parameters");
                std::cout << "From file: N == " << n << ", q == " << Q << ". From this program: N == " << _N_ << ", q == " << _q_ << '\n';
                throw std::runtime_error("Could not agree on parameters");
            }
            if(compareStrings(fileHeader, NTRUprivatKey)) {
                isPrivateKey = true;
                sz = this->privateKeySizeInBytes();
                coeffBytes = new char[sz];
                file.read(coeffBytes, sz);                                      // -Reading the coefficients of the polynomial
                this->privatKey = ZpPolynomial(coeffBytes, sz, false);          // -Create ZpPolynomial as in private key mode
                try {
                    this->setKeysFromPrivKey();
                } catch(const std::runtime_error&) {
                    delete[] coeffBytes; coeffBytes = NULL;
                    delete[] fileHeader; fileHeader = NULL;
                    cerrMessageBeforeReThrow(thisFunc);
                    throw;
                }
            } else {
                std::cout << "\tSetting NTRU::Encryption object in 'encryption only' mode" << std::endl;
                isPrivateKey = false;
                sz = this->publicKeySizeInBytes();
                coeffBytes = new char[sz];
                file.read(coeffBytes, sz);                                      // -Reading the coefficients of the polynomial
                this->publicKey = ZqPolynomial(coeffBytes, sz);
            }
        } else {
            delete[] fileHeader; fileHeader = NULL;
            cerrMessageBeforeThrow(thisFunc, "Not a valid ntruPrivateKey file.");
            throw std::runtime_error("Not valid input file.");
        }
    } else {
        cerrMessageBeforeThrow(thisFunc, "Could not open file for the creation of the keys.");
        throw std::runtime_error("File opening failed.");
    }
    if(coeffBytes != NULL) delete[] coeffBytes;
    if(fileHeader != NULL) delete[] fileHeader;
    this->validPrivateKey = isPrivateKey;                                       // -Only encryption if we got just the public key
}

size_t Encryption::plainTextMaxSizeInBytes() const{ return size_t(_N_/6); }
size_t Encryption::cipherTextSizeInBytes()   const{ return size_t(_N_*log2q/8 + 1); }
size_t Encryption::privateKeySizeInBytes()   const{ return size_t(_N_/5 + 1); }
size_t Encryption::publicKeySizeInBytes()    const{ return size_t(_N_*log2q/8 + 1); }

void Encryption::saveKeys(const char publicKeyName[], const char privateKeyName[]) const{
    const char thisFunc[] = "void Encryption::saveKeys(const char publicKeyName[], const char privateKeyName[]) const";
    int publicKeySize  = this->publicKeySizeInBytes();
    int privateKeySize = this->privateKeySizeInBytes();
    short N = _N_, q = _q_;
    char* publicKeyBytes  = NULL;
    char* privateKeyBytes = NULL;
    const char NTRUpublicKey[]  = "NTRUpublicKey";                              // -This will indicate the binary file is saving a NTRU public key
    const char NTRUprivateKey[] = "NTRUprivatKey";                              // -This will indicate the binary file is saving a NTRU private key
    std::ofstream file;
    if(publicKeyName == NULL) file.open("Key.ntrupub", std::ios::binary);
    else                      file.open(publicKeyName, std::ios::binary);
    if(file.is_open()) {
        file.write((char*)NTRUpublicKey, lengthString(NTRUpublicKey));          // -Initiating the file with the string "NTRUpublicKey"
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the degree of the polynomial
        publicKeyBytes = new char[publicKeySize];                               // -The following bytes are for the polynomials coefficients
        this->publicKey.toBytes(publicKeyBytes);
        file.write(publicKeyBytes, publicKeySize);
        delete[] publicKeyBytes;
        publicKeyBytes = NULL;
    } else {
        cerrMessageBeforeThrow(thisFunc, "Could not create file for public key.");
        throw std::runtime_error("File writing failed.");
    }
    file.close();
    if(this->validPrivateKey) {                                                 // -If the object is in only encryption mode, private key will not be saved,
        if(privateKeyName == NULL) file.open("Key.ntruprv", std::ios::binary);  //  because there is no valid private key
        else                       file.open(privateKeyName, std::ios::binary);
        if(file.is_open()) {
            file.write((char*)NTRUprivateKey, lengthString(NTRUprivateKey));    // -Initiating the file with the string "NTRUprivateKey"
            file.write((char*)&N, 2);                                          // -A short int for the degree of the polynomial
            file.write((char*)&q, 2);                                          // -A short int for the degree of the polynomial
            privateKeyBytes = new char[privateKeySize];                         // -The following bytes are for the polynomials coefficients
            this->privatKey.toBytes(privateKeyBytes, false);
            file.write(privateKeyBytes, privateKeySize);
            delete[] privateKeyBytes;
            privateKeyBytes = NULL;
        } else {
            cerrMessageBeforeThrow(thisFunc, "Could not create file for private key.");
            throw std::runtime_error("File writing failed.");
        }
    }
    if(publicKeyBytes  != NULL) delete[] publicKeyBytes;
    if(privateKeyBytes != NULL) delete[] privateKeyBytes;
}

void Encryption::printKeys(const char publicKeyName[], const char privateKeyName[]) const{
    if(publicKeyName != NULL) this->publicKey.println(publicKeyName);
    else this->publicKey.println("Public Key");
    std::cout << '\n';
    if(this->validPrivateKey) {
        if(privateKeyName != NULL) this->privatKey.println(privateKeyName);
        else this->privatKey.println("Private Key");
    } else {
        if(privateKeyName != NULL) std::cout << privateKeyName;
        else std::cout << "Private Key";
        std::cout << ": No private key available." << std::endl;
    }
}

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Encryption keys |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

ZqPolynomial Encryption::productByPrivatKey(const ZqPolynomial& P) const{
    return convolutionZq(this->privatKey, P).timesThree() + P;                  // Private key f has the form 1+p·F0, so f*P = P+p(P*F0)
}

ZqPolynomial Encryption::productByPrivatKey(const Z2Polynomial& P) const{
    ZqPolynomial r = convolutionZq(P, this->privatKey).timesThree();            // Private key f has the form 1+p·F0, so f*P = P+p(P*F0)
    for(int i = 0; i < _N_; i++) if(P.coefficients[i] != 0) r.coefficients[i]++;
    return r;
}

void Encryption::setKeys() {
    const char thisFunc[] = "void Encryption::setKeys()";
	Z2Polynomial Z2_privateKeyInv;
	Z2Polynomial Z2_gcdXNmns1;
	this->privatKey = NTRU::ZpPolynomial::randomTernary();
	Z2Polynomial Z2_privateKey(this->privatKey);
	Z2_privateKey.negFirstCoeff();                                              // -Embeding 3·privatKey0 + 1 in Z2Polynomial. For i > 0 and int polynoimal F:
	int counter, k = 2, l = 1;                                                  //  (3·F[i]) mod 2 -> [(3 mod 2)·F[i]] mod 2 -> [1·F[i]] mod 2 = F[i] mod 2.In
                                                                                //  other hand, (F[0]+1) mod 2 -> [(F[0] mod 2)+1)] mod 2 -> neg[(F[0]+1) mod 2]
    try{
        Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv);
    }catch(const std::runtime_error&) {
        cerrMessageBeforeReThrow(thisFunc);
        throw;
    }
	counter = 1;
	while(Z2_gcdXNmns1 != 1) {                                                  // -Looking for a valid private key.
        if((counter & 1) == 0)  this->privatKey.interchangeZeroFor(ZpPolynomial::_1_);
        else                    this->privatKey.interchangeZeroFor(ZpPolynomial::_2_);
        Z2_privateKey = this->privatKey;
        //if((counter&3)==0){
            //std::cout << "Source/NTRUencryption.cpp; void Encryption::setKeys(bool showKeyCreationTime). Counter = " << counter << "\n";\\Debuggin purposes
        //}
        Z2_privateKey.negFirstCoeff();
        try{ Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv); }
        catch(const std::runtime_error&) {
            cerrMessageBeforeReThrow(thisFunc);
            throw;
    	}
        counter++;
    }                                                                           // -Shearch finished.
    // -This hole section can be optimized by making it a function like lift R2-Rq for private key
    this->publicKey = convolutionZq(Z2_privateKeyInv, 2 - Encryption::productByPrivatKey(Z2_privateKeyInv));
    k <<= l; l <<= 1;
	while(k < _q_) {
        this->publicKey = this->publicKey*(2 - Encryption::productByPrivatKey(publicKey));
        k <<= l; l <<= 1;
    }                                                                           // -At this line, we have just created the private key and its inverse
    ZqPolynomial t = productByPrivatKey(this->publicKey);                     // -Testing the if the statement above is true (Debugging purposes)
    /*t.mods_q();
    if(!t.equalsOne()) {
        std::cout << thisFunc << "::Parameters: N = "<< _N_ << ", q = " << _q_ << " --------------" << std::endl;
        t.println("this->publicKey*this->privateKey");
        cerrMessageBeforeThrow(thisFunc,"Public key inverse in Zq[x]/x^N-1 finding failed");
        throw std::runtime_error("Exception in public key inverse creation");
    }*/
    this->publicKey = convolutionZq(ZpPolynomial::randomTernary(), this->publicKey).timesThree(); // -Multiplication by the g polynomial.
    this->publicKey.mods_q();
}

void Encryption::setKeysFromPrivKey() {                                         // -In this function we're assuming we already have a valid private key,
    const char thisFunc[] = "void Encryption::setKeysFromPrivKey()";            //  this is, a privatKey with inverse mod 2.
    Z2Polynomial Z2_privateKey(this->privatKey);
    Z2Polynomial Z2_privateKeyInv;
    Z2Polynomial Z2_gcdXNmns1;
    int k = 2, l = 1;

    Z2_privateKey.negFirstCoeff();
    try{ Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv); }
    catch(const std::runtime_error&) {
        cerrMessageBeforeReThrow(thisFunc);
        throw;
    }
    // -This hole section can be optimized by making it a function like lift R2-Rq for private key
    this->publicKey = convolutionZq(Z2_privateKeyInv, 2 - Encryption::productByPrivatKey(Z2_privateKeyInv));
    k <<= l; l <<= 1;
	while(k < _q_) {
        this->publicKey = this->publicKey*(2 - Encryption::productByPrivatKey(publicKey));
        k <<= l; l <<= 1;
    }                                                                           // -At this line, we have just created the private key and its inverse
    /*ZqPolynomial t = productByPrivatKey(this->publicKey);                     // -Testing the if the statement above is true (Debugging purposes)
    t.mods_q();
    if(t != 1) {
        t.println("this->publicKey*this->privateKey");
        cerrMessageBeforeThrow(thisFunc,"Public key inverse in Zq[x]/x^N-1 finding failed");
        throw std::runtime_error("Exception in public key inverse creation");
    }*/
    this->publicKey = convolutionZq(ZpPolynomial::randomTernary(), this->publicKey).timesThree(); // -Multiplication by the g polynomial.
    this->publicKey.mods_q();
}

// ____________________________________________________________________ Encryption keys ___________________________________________________________________________

ZqPolynomial Encryption::encrypt(const char bytes[], int size) const{
    ZpPolynomial msg(bytes, size, true);
    ZqPolynomial encryptedMsg = msg.encrypt(this->publicKey);
    return encryptedMsg;
}

ZqPolynomial Encryption::encrypt(const ZpPolynomial& msg) const{
    ZqPolynomial encryptedMsg = msg.encrypt(publicKey);
    return encryptedMsg;
}

ZqPolynomial Encryption::encryptFile(const char fileName[]) const{              // -Reads file content, writes in array, append a 0x80 at the end and encrypts.
    const char thisFunc[] = "ZqPolynomial Encryption::encryptFile(const char fileName[])";
    const size_t plainTextMaxSize = inputPlainTextMaxSizeBytes()-1;             // -Minus one because we will use one byte to mark the end of the file content.
    size_t fileInputArrSize = 0;
    char* fileInput = NULL;
    std::streampos  fileSize;
    std::ifstream inputFile;
    NTRU::ZqPolynomial  enc_msg;
    // -Start of: Getting file size.
    inputFile.open(fileName, std::ios::binary | std::ios::ate);                 // -Bynary mode. Positioning at the end of the file.
    if(inputFile.is_open()) fileSize = inputFile.tellg();                       // -Getting current position of the "get" pointer.
    else{
        cerrMessageBeforeThrow(thisFunc, "Error opening file for encryption");
        throw std::runtime_error("File opening failed");
    }
    if((size_t)fileSize > plainTextMaxSize) {
        cerrMessageBeforeThrow(thisFunc, "");
        std::cerr << "File size exceeds the limit size (" << plainTextMaxSize << " bytes)\n";
        std::cerr << "I will proceed to truncate the file." << std::endl;
        fileSize = (std::streamoff)plainTextMaxSize;                            // -From here, input data has plainTextMaxSize bytes.
    }
    // -End of: Getting file size.
    inputFile.seekg(0, std::ios::beg);                                          // -Move back to the beginning
    // -Start of: Reading input file.
    fileInputArrSize = (size_t)fileSize+1;
    fileInput = new char[fileInputArrSize];
    inputFile.read(fileInput, fileSize);
    fileInput[fileSize] = (char)0x80;                                           // -Writting 1000,0000 at the end of file content. This will be important at the
                                                                                //  moment of the decryption.
    //displayByteArrayChar(fileInput, fileInputArrSize);std::cout << '\n';           // -Debugging purposes
    //displayByteArrayChar(fileInput, fileInputArrSize);std::cout << '\n';          // -Debuggin purposes
    // -End of: Reading input file.
    enc_msg = this->encrypt(fileInput, fileInputArrSize);                       // -Encrypting the bytes from the file and its size
    delete[] fileInput;
    return enc_msg;
}

ZpPolynomial Encryption::decrypt(const ZqPolynomial& e_msg) const{
    if(!this->validPrivateKey) {
        std::cerr << "\nThis object has no valid private key, therefore is only capable of encryption. Returning zero ZpPolynomial.\n";
        return ZpPolynomial();
    }
    ZqPolynomial msg_ = productByPrivatKey(e_msg);
    msg_.mods_q();
    ZpPolynomial msg = mods_p(msg_);
    return msg;
}

ZpPolynomial Encryption::decrypt(const char bytes[], int size) const{
    return this->decrypt(ZqPolynomial(bytes, size));
}

NTRU_N Encryption::get_N() const { return (NTRU_N)_N_; }
NTRU_q Encryption::get_q() const { return (NTRU_q)_q_; }

/************************************************************************* Statistics ****************************************************************************/

Encryption::Statistics::Time::Time(const uint64_t time_data[], size_t size){
    this->Maximum = this->maximum(time_data, size);
    this->Minimum = this->minimum(time_data, size);
    this->Average = this->average(time_data, size);
    this->Variance= this->variance(time_data, size);
    this->AvrAbsDev=this->avrAbsDev(time_data, size);
}

double Encryption::Statistics::Time::maximum(const uint64_t time_data[], size_t size) const{
    uint64_t max = time_data[0];
    for(size_t i = 0; i < size; i++) if(max < time_data[i]) max = time_data[i];
    return max;
}

double Encryption::Statistics::Time::minimum(const uint64_t time_data[], size_t size) const{
    uint64_t min = time_data[0];
    for(size_t i = 0; i < size; i++) if(min > time_data[i]) min = time_data[i];
    return min;
}

double Encryption::Statistics::Time::average(const uint64_t data[], size_t size) const{
    double avr = 0.0;
    for(size_t i = 0; i < size; i++) avr += (double)data[i];
    avr /= (double)size;
    return avr;
}

double Encryption::Statistics::Time::variance(const uint64_t time_data[], size_t size) const{
    double var = 0.0;
    double avr = this->average(time_data, size);
    for(size_t i = 0; i < size; i++) {
        var += (time_data[i] - avr) *(time_data[i] - avr);
    }
    var /= (double)size;
    return var;
}

double Encryption::Statistics::Time::avrAbsDev(const uint64_t time_data[], size_t size) const{
    double aad = 0.0, buff = 0.0;
    double avr = this->average(time_data, size);
    for(size_t i = 0; i < size; i++) {
        buff = time_data[i] - avr;
        buff < 0 ? aad -= buff : aad += buff;
    }
    aad /= (double)size;
    return aad;
}

Encryption::Statistics::Time Encryption::Statistics::Time::keyGeneration(){
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    /*uint64_t begin;
    uint64_t end;*/
	uint64_t times[NUMBEROFROUNDS], i;
	Encryption e;

    std::cout << "Encryption::Statistics::Time::keyGeneration::Parameters: N = "<< _N_ << ", q = " << _q_ << " --------------" << std::endl;
	for(i = 0; i < NUMBEROFROUNDS; i++, e.privatKey = ZpPolynomial::getPosiblePrivateKey()){
        if((i & 31) == 0) std::cout<<"Source/NTRUencryption.cpp, Encryption::Statistics::Time Encryption::Statistics::Time::keyGeneration(): Round "<<i<<std::endl;
	    begin = std::chrono::steady_clock::now();
	    e.setKeys();
	    end  = std::chrono::steady_clock::now();
	    times[i] = (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
	    /*begin = readTSC();
	    end   = readTSC();
	    times[i] = end-begin;*/
    }
    return Encryption::Statistics::Time(times, NUMBEROFROUNDS);
}

Encryption::Statistics::Time Encryption::Statistics::Time::ciphering(const NTRU::Encryption* e, const NTRU::ZpPolynomial* msg){
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    /*uint64_t begin; uint64_t end;*/
    uint64_t times[NUMBEROFROUNDS];
    const NTRU::Encryption* ptr_e = NULL;
    const NTRU::ZpPolynomial* ptr_msg = NULL;
    if(e == NULL){                                                              // If NULL reference, creating our own encryption object
        std::cout << "Warning: In File NTRUencryption, function NTRU::Encryption::Statistics::Time Encryption::Statistics::Time::ciphering"
        "(const NTRU::Encryption*, const NTRU::ZpPolynomial*). >>const NTRU::Encryption*<< argument is NULL; I will proceed to create my own.\n";
        ptr_e = new NTRU::Encryption();
    } else {
        ptr_e = e;
    }
    if(msg == NULL){
        std::cout << "Warning: In File NTRUencryption, function NTRU::Encryption::Statistics::Time Encryption::Statistics::Time::ciphering"
        "(const NTRU::Encryption*, const NTRU::ZpPolynomial*). >>const NTRU::ZpPolynomial*<< argument is NULL; I will proceed to create my own.\n";
        ptr_msg = new NTRU::ZpPolynomial();
    } else {
        ptr_msg = msg;
    }

    std::cout << "Encryption::Statistics::Time::ciphering::Parameters: N = "<< _N_ << ", q = " << _q_ << " --------------" << std::endl;
    for(size_t i = 0; i < NUMBEROFROUNDS; i++){
        if((i&63) == 0) std::cout << "Source/NTRUencryption.cpp, Encryption::Statistics::Time Encryption::Statistics::Time::ciphering(): Round " << i << std::endl;
        begin= std::chrono::steady_clock::now();
	    ptr_e->encrypt(*ptr_msg);
	    end  = std::chrono::steady_clock::now();
	    times[i] = (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
	    /*begin = readTSC(); ptr_e->encrypt(dummy, dummy_sz); end = readTSC(); times[i] = end-begin;*/
    }
    if(e == NULL) delete[] ptr_e;                                               // If e is NULL, then ptr_e was created inside this function with the new operator
    if(msg == NULL) delete[] ptr_msg;                                           // If msg is NULL, then ptr_msg was created inside this function with the new operator
    return Encryption::Statistics::Time(times, NUMBEROFROUNDS);
}

Encryption::Statistics::Time Encryption::Statistics::Time::deciphering(const NTRU::Encryption* e, const NTRU::ZqPolynomial* emsg){
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    /*uint64_t begin; uint64_t end;*/
    uint64_t times[NUMBEROFROUNDS];
    const NTRU::Encryption* ptr_e = NULL;
    const NTRU::ZqPolynomial* ptr_emsg = NULL;
    if(e == NULL){                                                              // If NULL reference, creating our own encryption object
        std::cout << "Warning: In File:\n"
                        "\tNTRUencryption.cpp\n"
                     "Function\n"
                        "\tNTRU::Encryption::Statistics::Time Encryption::Statistics::Time::deciphering(const NTRU::Encryption*, const NTRU::ZqPolynomial*)\n"
                     ">>const NTRU::Encryption*<< argument is NULL; I will proceed to create my own.\n";
        ptr_e = new NTRU::Encryption();
    } else {
        ptr_e = e;
    }
    if(emsg == NULL){
        std::cout << "Warning: In File:\n"
                        "\tNTRUencryption.cpp\n"
                     "Function\n"
                        "\tNTRU::Encryption::Statistics::Time Encryption::Statistics::Time::deciphering(const NTRU::Encryption*, const NTRU::ZqPolynomial*)\n"
                     ">>const NTRU::ZqPolynomial*<< argument is NULL; I will proceed to create my own.\n";
        ptr_emsg = new NTRU::ZqPolynomial();
    } else {
        ptr_emsg = emsg;
    }

    std::cout << "Encryption::Statistics::Time::deciphering::Parameters: N = "<< _N_ << ", q = " << _q_ << " --------------" << std::endl;
    for(size_t i = 0; i < NUMBEROFROUNDS; i++){
        if((i & 63) == 0)
            std::cout << "Source/NTRUencryption.cpp, Encryption::Statistics::Time Encryption::Statistics::Time::deciphering(): Round " << i << std::endl;
        begin= std::chrono::steady_clock::now();
	    ptr_e->decrypt(*ptr_emsg);
	    end  = std::chrono::steady_clock::now();
	    times[i] = (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
	    /*begin = readTSC(); ptr_e->encrypt(dummy, dummy_sz); end = readTSC(); times[i] = end-begin;*/
    }
    if(e == NULL) delete ptr_e;                                                 // If e is NULL, then ptr_e was created inside this function with the new operator
    if(emsg == NULL) delete ptr_emsg;                                           // If emsg is NULL, then ptr_emsg was created inside this function with the new operator
    return Encryption::Statistics::Time(times, NUMBEROFROUNDS);
}

//________________________________________________________________________ Encryption _____________________________________________________________________________

void Encryption::Statistics::Data::setbyteValueFrequence(const char data[], size_t size){
    if(!this->byteValueFrequenceStablisched) {
        for(size_t i = 0; i < size; i++) this->byteValueFrequence[(uint8_t)data[i]]++;
        this->byteValueFrequenceStablisched = true;
    }
}

double Encryption::Statistics::Data::entropy(const char data[], size_t size){
    double  entropy =  0.0 ;
    double  p[256]  = {0.0};
    size_t  i = 0;
    this->setbyteValueFrequence(data, size);
    for(i = 0; i < 256; i++) p[i] = this->byteValueFrequence[i]/(double)size;
    for(i = 0; i < 256; i++) if(p[i] != 0) entropy -= p[i]*log2(p[i]);

    return entropy;
}

double Encryption::Statistics::Data::xiSquare(const char data[], size_t size) {
    double xiSquare = 0.0;
    this->setbyteValueFrequence(data, size);
    for(int i = 0; i < 256; i++)
        xiSquare += (double)(this->byteValueFrequence[i]*this->byteValueFrequence[i]);
    xiSquare *= 256.0/size; xiSquare -= size;
    return xiSquare;
}

double Encryption::Statistics::Data::correlation(const char data[], size_t size, size_t offset){
    double average = 0.0;
    double variance = 0.0;
    double covariance = 0.0;
    size_t i, j;
    for(i = 0; i < size; i++) average += (uint8_t)data[i];
    average /= double(size);

    for(i = 0, j = offset; i < size; i++, j++){
        if(j >= size) j = 0;
        variance   += ((uint8_t)data[i] - average)*((uint8_t)data[i] - average);
        covariance += ((uint8_t)data[i] - average)*((uint8_t)data[j] - average);
    }
    variance   /= (double)size;
    covariance /= (double)size;

    return covariance/variance;
}

Encryption::Statistics::Data::Data(const char data[], size_t size){
    this->Entropy = this->entropy(data, size);
    this->Correlation = this->correlation(data, size, 1);
    this->XiSquare = this->xiSquare(data, size);
}

static void printHex(const char a[], size_t size, const char front[] = "", const char back[] = ""){
    size_t sz = size > 0 ? size - 1: 0;
    size_t i;
    unsigned char ua;
    std::cout << front;
    printf("[");
    for(i = 0; i < sz; i++){
        ua = (unsigned char)a[i];
        if(ua < 0x10) printf("0");
        printf("%X", ua);
    }
    ua = (unsigned char)a[i];
    if(ua < 0x10) printf("0");
    printf("%X", ua);
    printf("]");
    std::cout << back;
}

Encryption::Statistics::Data Encryption::Statistics::Data::encryption(const NTRU::Encryption* ptr_e){ // plainTextSize = N/5, cipherTextSize = N*log2(q)/8
    size_t blockSize = ptr_e->plainTextMaxSizeInBytes();                        // cipherTextSize/plainTextSize = 5*log2(q)/8
    const size_t cipherBlockSize= ptr_e->cipherTextSizeInBytes();               // cipherTextSize = plainTextSize*5*log2(q)/8
    const size_t numberOfRounds = 8*3*512*512/(5*blockSize*(size_t)log2q);      // cipherTextSize*numberOfRounds = 512*512*3 (size of a 256x256 pixel image)
    const size_t dummyEncLen = cipherBlockSize*(numberOfRounds);                // (plainTextSize*5*log2(q)/8)*numberOfRounds = 512*512*3
                                                                                // dummyLen = plainTextSize*numberOfRounds = 512*512*3*8/[5*log2(q)]
    blockSize++;                                                                // numberOfRounds = dummyLen/plainTextSize
    char dummy[_1499_];                                                         // -A byte more for blocks to avoid <<out of range>> error.
    char dummy_dec[_1499_];
    char* dummy_enc = new char[dummyEncLen];
    size_t i, j, k, l, r;
    int    a = 0;
    ZqPolynomial enc;
    ZpPolynomial dec;

    std::cout << "Encryption::Statistics::Data::encryption::Parameters: N = "<< _N_ << ", q = " << _q_ << " ---------------------------------------" << std::endl;

    for(i = 0; i < blockSize; i++)   dummy[i]     = 0;
    for(i = 0; i < dummyEncLen; i++) dummy_enc[i] = 0;
    for(i = 0; i < blockSize; i++)   dummy_dec[i] = 0;

    blockSize--;                                                                // -Comming back to original size.
    std::cout << "Source/NTRUencryption.cpp, Encryption::Statistics::Data::encryption(): "
                 "Input length = " << blockSize*numberOfRounds << ". Encrypted input length = " << dummyEncLen << ". Block size = " << blockSize << '\n';

    for(j = 0, k = 0, r = 0; k < numberOfRounds; j += cipherBlockSize, k++) {
        if((k&7)==0) std::cout << "Encryption::Statistics::Data::encryption(): Round " << k << std::endl;
        enc = ptr_e->encrypt(dummy, blockSize);
        enc.toBytes(dummy_enc + j);
        enc = ZqPolynomial(dummy_enc + j, cipherBlockSize);
        dec = ptr_e->decrypt(enc);
        dec.toBytes(dummy_dec, true);                                           // -Second parameter isPlainText = true
        for(l = 0; l < blockSize; l++){
            if(dummy[l] != dummy_dec[l]) {
                a = int(l<<1) - 4;
                std::cout << "At block " << k << ": Decryption failure in byte number " << "l = " << l << std::endl; // -Showing firs decryption failure
                if(l < 8) {
                    printHex(dummy,    16, "Block[0,16]           = ", "\n");
                    printHex(dummy_dec,16, "Dec(Enc(Block))[0,16] = ", "\n");
                    if(a > 0) for(; a > 0; a--) std::cout << ' ';
                    std::cout <<           " First occurrence here ~~^" << std::endl;
                } else if(blockSize - l < 8) {
                    printHex(dummy+(blockSize-16),    16, "Block[SZ-16,SZ-1]           = ", "\n");
                    printHex(dummy_dec+(blockSize-16),16, "Dec(Enc(Block))[SZ-16,SZ-1] = ", "\n");
                    if(a > 0) for(; a > 0; a--) std::cout << ' ';
                    std::cout <<                      "       First occurrence here ~~^" << std::endl;
                } else{
                    printHex(dummy+(l-8),    16, "Block[l-8,l+8]           = ", "\n");
                    printHex(dummy_dec+(l-8),16, "Dec(Enc(Block))[l-8,l+8] = ", "\n");
                    for(a = 0; a <16; a++) std::cout << ' ';
                    std::cout <<                 "    First occurrence here ~~^" << std::endl;
                }
                r++;
                break;
            }
        }
        if(dummy[i] == (char)255) i++;
        else dummy[i]++;
    }
    std::cout << "Total amount of rounds: " <<  numberOfRounds << '\n';
    std::cout << "Total amount of decryption failures: " << r;
    Encryption::Statistics::Data stats(dummy_enc, dummyEncLen);
    delete[] dummy_enc;
    return stats;
}
