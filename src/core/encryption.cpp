#include<iostream>
#include<fstream>
#include<random>
#include"../../include/ntru/encryption.hpp"
#include"../../include/ntru/exceptions.hpp"
#include"parameter_validation.hpp"

using namespace NTRU;

Encryption::Encryption() {                                                      // -Just for type declaration. Should not be used just like this
    try {
	    this->setKeys();
	}catch(MathException& exp) {
	    this->validPrivateKey = false;
	    exp.add_trace_point("Caught in Encryption::Encryption() while building keys");
	    throw;
	}
	this->validPrivateKey = true;
}

Encryption::Encryption(const char* NTRUkeyFile) {
    const std::string thisFunc = "Encryption::Encryption(const char*)";
    const char NTRUpublicKey[] = "NTRUpublicKey";                               // -This will indicate the binary file is saving a NTRU public key
    const char NTRUprivatKey[] = "NTRUprivatKey";                               // -This will indicate the binary file is saving a NTRU private key
    char* coeffBytes = NULL;
    char* fileHeader = NULL;
    int   sz = 0, headerSz = strlen("NTRUpublicKey");                     // -Notice how "NTRUpublicKey" and "NTRUprivatKey" strings have the same length
    bool  isPrivateKey = false;
    short n;
    short Q;
    std::ifstream file;
    file.open(NTRUkeyFile, std::ios::binary);
    if(file.is_open()) {
        fileHeader = new char[headerSz + 1];
        file.read(fileHeader, headerSz);                                        // -Reading the firsts bytes hoping "NTRUpublicKey" or "NTRUprivatKey" apears
        fileHeader[headerSz] = 0;                                               // -End of string
        if(strcmp(fileHeader, NTRUprivatKey) == 0 || strcmp(fileHeader, NTRUpublicKey) == 0) { // -Testing if file saves a NTRU private key
            file.read((char*)&n, 2);
            file.read((char*)&Q, 2);
            if(n != NTRU_N || Q != NTRU_Q) {
                delete[] fileHeader; fileHeader = NULL;
                std::stringstream ss;
                ss << "In " << thisFunc << ": Parameters retreaved from file do not match with this program parameters\n";
                ss << "From " << NTRUkeyFile << ": N == " << n << ", q == " << Q << ". From this program: N == " << NTRU_N << ", q == " << NTRU_Q << "\n";
                ss << "Could not agree on parameters";
                throw ParameterMismatchException(ss.str());
            }
            if(strcmp(fileHeader, NTRUprivatKey) == 0) {
                isPrivateKey = true;
                sz = this->privateKeySizeInBytes();
                coeffBytes = new char[sz];
                file.read(coeffBytes, sz);                                      // -Reading the coefficients of the polynomial
                this->privatKey = RpPolynomialFromBytes(coeffBytes, (size_t)sz, true); // -Create RpPolynomial as in private key mode
                try {
                    this->setKeysFromPrivKey();
                } catch(MathException& exp) {
                    delete[] coeffBytes; coeffBytes = NULL;
                    delete[] fileHeader; fileHeader = NULL;
                    exp.add_trace_point("Caught in " + thisFunc + ": while building public key from private key provided by " + std::string(NTRUkeyFile));
                    throw;
                }
            } else {
                std::cout << "\tSetting Encryption object in 'encryption only' mode" << std::endl;
                isPrivateKey = false;
                sz = this->publicKeySizeInBytes();
                coeffBytes = new char[sz];
                file.read(coeffBytes, sz);                                      // -Reading the coefficients of the polynomial
                this->publicKey = RqPolynomial(coeffBytes, sz);
            }
        } else {
            delete[] fileHeader; fileHeader = NULL;
            throw FileIOException("In " + thisFunc + ": " + std::string(NTRUkeyFile) + " is not a valid ntruPrivateKey file.");
        }
    } else {
        throw FileIOException("In " + thisFunc + ": I could not open " + std::string(NTRUkeyFile));
    }
    if(coeffBytes != NULL) delete[] coeffBytes;
    if(fileHeader != NULL) delete[] fileHeader;
    this->validPrivateKey = isPrivateKey;                                       // -Only encryption if we got just the public key
}

/*
   -When representing an -input- array of bytes [let us call it A, where A[i] is the ith-element] with a RpPolynomial, the main idea is to represent the value of
    each of the A[i] with a number of 6 digits in base 3, then take those digits as coefficients of the polynomial we are building. Why not use just % digits?
    Because the maximum value for a 5-digits number in base 3 is 242, therefore, in the case of having an element A[i] with a value bigger than 242, then a
    5-digit representation will not be sufficient for its representation, then the number will be "truncated" and some information will be lost; this is important
    since we are not assuming anything about the input array.
*/
size_t Encryption::inputPlainTextMaxSizeInBytes()  { return size_t(NTRU_N/6 - 1); }// -Each byte will be represented with 6 coefficientes, and one byte will be used to mark the end of the contents, therefor max size is NTRU_N/6 - 1
size_t Encryption::outputPlainTextMaxSizeInBytes() { return size_t(NTRU_N/6 + 1); }// -Here we are supposing that the polynomial was created using 6-coefficients byte representation. The +1 is for the last N%6 bytes.
/*
   -The protocol for the creation of a RqPolynomial from a byte array is more simple. Each coefficient of a RpPolynomial can be represented using exactly log2q
    bits, therefor we can -roughly speaking- copy the bits inside the byte array and paste them directly into the coefficients of the polynomial. The procedure
    is more complicated, but that is the main idea.
*/
size_t Encryption::cipherTextSizeInBytes() { return size_t(NTRU_N*getlog2q()/8 + 1); }
/*
   -Private key is generated randomly inside the program, so a 6-digits number where the digits are a pack of 6 consecutive coefficients may be greater than 255
    (the maximum value of a 6-digits number is 728). Here, the process from private key in polynomial form to byte array takes 5 consecutive coefficients and
    "converts" it to a 5-digits number in base 3. Thats the reason why the private key needs NTRU_N/5 + 1 bytes to be represente -under this protocol-.
*/
size_t Encryption::privateKeySizeInBytes(){ return size_t(NTRU_N/5 + 1); }
size_t Encryption::publicKeySizeInBytes() { return size_t(NTRU_N*getlog2q()/8 + 1); }

void Encryption::saveKeys(const char publicKeyName[], const char privateKeyName[]) const{
    const std::string thisFunc = "void Encryption::saveKeys(const char publicKeyName[], const char privateKeyName[]) const";
    int publicKeySize  = this->publicKeySizeInBytes();
    int privateKeySize = this->privateKeySizeInBytes();
    short N = NTRU_N, q = NTRU_Q;
    char* publicKeyBytes  = NULL;
    char* privateKeyBytes = NULL;
    const char NTRUpublicKey[]  = "NTRUpublicKey";                              // -This will indicate the binary file is saving a NTRU public key
    const char NTRUprivateKey[] = "NTRUprivatKey";                              // -This will indicate the binary file is saving a NTRU private key
    std::ofstream file;
    if(publicKeyName == NULL) file.open("Key.ntrupub", std::ios::binary);
    else                      file.open(publicKeyName, std::ios::binary);
    if(file.is_open()) {
        file.write((char*)NTRUpublicKey, strlen(NTRUpublicKey));          // -Initiating the file with the string "NTRUpublicKey"
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the degree of the polynomial
        publicKeyBytes = new char[publicKeySize];                               // -The following bytes are for the polynomials coefficients
        this->publicKey.toBytes(publicKeyBytes);
        file.write(publicKeyBytes, publicKeySize);
        delete[] publicKeyBytes;
        publicKeyBytes = NULL;
    } else {
        throw FileIOException("In " + thisFunc + ": Could not create file for " + std::string(publicKeyName));
    }
    file.close();
    if(this->validPrivateKey) {                                                 // -If the object is in only encryption mode, private key will not be saved,
        if(privateKeyName == NULL) file.open("Key.ntruprv", std::ios::binary);  //  because there is no valid private key
        else                       file.open(privateKeyName, std::ios::binary);
        if(file.is_open()) {
            file.write((char*)NTRUprivateKey, strlen(NTRUprivateKey));    // -Initiating the file with the string "NTRUprivateKey"
            file.write((char*)&N, 2);                                          // -A short int for the degree of the polynomial
            file.write((char*)&q, 2);                                          // -A short int for the degree of the polynomial
            privateKeyBytes = new char[privateKeySize];                         // -The following bytes are for the polynomials coefficients
            RpPolynomialtoBytes(privatKey, privateKeyBytes, true);
            file.write(privateKeyBytes, privateKeySize);
            delete[] privateKeyBytes;
            privateKeyBytes = NULL;
        } else {
            throw FileIOException("In " + thisFunc + ": Could not create file for " + std::string(privateKeyName));
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

/********************************************************************* End: File Handling ***********************************************************************/

RpPolynomial Encryption::RpPolynomialFromBytes(const char bytes[], size_t size, bool isPrivateKey) {
    size_t i,j,k,l;
    RpPolynomial r;
    const size_t maxSize = isPrivateKey ? privateKeySizeInBytes() : inputPlainTextMaxSizeInBytes();// -This will guarantee we do not run out of coefficients.
    const size_t m = isPrivateKey == true ? 5 : 6;                              // -If bytes represent a private key, m=5, otherwise it is plain text, then m=6
    if(size == 0) return r;                                                     // -Guarding against negative or null dataLength
    if(size > maxSize) {
        std::cerr << "In file NTRUencryption.cpp, funtion RpPolynomial Encryption::RpPolynomialFromBytes(const char bytes[], size_t size, bool isPrivateKey)\n";
        std::cerr << "Input byte array exceeds the limit size (" << maxSize << " bytes)\n";
        std::cerr << "I will proceed to truncate the input array." << std::endl;
        size = maxSize;                                                         // -From here, input data has plainTextMaxSize bytes.
    }
    for(i = 0, j = 0; i < size && j < NTRU_N; i++) {                               // -i will run through data, j through coefficients
        l = (unsigned char)bytes[i];
        for(k = 0; k < m && j < NTRU_N; k++, l/=3) {                               // -Here we're supposing _p_ == 3. Basically we're changing from base 2 to base 3
            switch(l%3) {                                                       //  in big endian notation. Notice that the maximum value allowed is 242
                case  1:                                                        // -One idea to solve this issue is to have a "flag" value, say 242. Suppose b is
                    r.coefficients[j++] = RpPolynomial::_1_;                    //  a byte such that b >= 242; then we can write b = 242 + (b - 242). The
                break;                                                          //  inequality 0 <= b - 242 <= 13 holds, so we will need 3 3-base digits to write
                case  2:                                                        //  that new value.
                    r.coefficients[j++] = RpPolynomial::_2_;
                break;
                default:                                                        // -Leaving current coefficient as zero
                    j++;                                                        // -Jumping to next coefficient
            }
        }
    }
    if(!isPrivateKey){                                                          // -If not private key, interpret it as plain text.
        r.coefficients[j++] = RpPolynomial::_2_;                                // -Appending the value 11202, this value corresponds to 0x80 (1000,0000 in binary).
        r.coefficients[j++] = RpPolynomial::_0_;                                // -The main idea is to "mark" the end of the of the incoming data, this method becomes
        r.coefficients[j++] = RpPolynomial::_2_;                                //  handy at the moment of encrypting and decrypting files because in the decryption
        r.coefficients[j++] = RpPolynomial::_1_;                                //  process, the program "knows" where the file ends.
        r.coefficients[j] = RpPolynomial::_1_;                                  // -In summary, the data that the polynomial represents is the input byte array
    }                                                                           //  with the byte 1000,0000 appended at the end.
    return r;
}

RpPolynomial Encryption::RpPolynomialPlainTextFromFile(const char* fileName){
    const std::string thisFunc = "Encryption::RpPolynomialPlainTextFromFile(const char*)";
    const size_t plainTextMaxSize = inputPlainTextMaxSizeInBytes();             // -Minus one because we will use one byte to mark the end of the file content.
    size_t fileInputArrSize = 0;
    char* fileInput = NULL;
    std::streampos fileSize;
    std::ifstream inputFile;
    // -Start of: Getting file size.
    inputFile.open(fileName, std::ios::binary | std::ios::ate);                 // -Bynary mode. Positioning at the end of the file.
    if(inputFile.is_open()) fileSize = inputFile.tellg();                       // -Getting current position of the "get" pointer.
    else{
        throw FileIOException("In " + thisFunc + ": Exception while opening " + std::string(fileName));
    }
    if((size_t)fileSize > plainTextMaxSize) {
        std::stringstream ss;
        ss << "In " << thisFunc << "File size exceeds the limit size (" << plainTextMaxSize << " bytes)\n";
        throw FileIOException(ss.str());
        //ss << "I will proceed to truncate the file\n";
        //fileSize = (std::streamoff)plainTextMaxSize;                            // -From here, input data has plainTextMaxSize bytes.
    }
    // -End of: Getting file size.
    inputFile.seekg(0, std::ios::beg);                                          // -Move back to the beginning
    // -Start of: Reading input file.
    fileInputArrSize = (size_t)fileSize;
    fileInput = new char[fileInputArrSize];
    inputFile.read(fileInput, fileSize);
    return RpPolynomialFromBytes(fileInput, (size_t)fileSize, false);
}

void Encryption::RpPolynomialtoBytes(const RpPolynomial& org, char dest[], bool isPrivateKey){
    int i,j,k,m;
    int N_mod_m;
    int N_florm;
    int buff;
    isPrivateKey == true ? m = 5 : m = 6;
    N_mod_m = NTRU_N % m;                                                          // -Since NTRU_N is a prime number, N_mod_m is always bigger than 0
    N_florm = NTRU_N - N_mod_m;
    for(i = 0, j = 0; i < N_florm; i += m, j++) {                               // i will run through dest, j through coefficients
        for(k = m-1, buff = 0; k >= 0; k--) buff = buff*3 + org.coefficients[i+k]; // Here we're supposing _p_== 3. Basically we're changing from base 3 to base 2
        dest[j] = (char)buff;                                                   // Supposing the numbers in base 3 are in big endian notation
    }
    for(k = N_mod_m-1, buff = 0; k >= 0; k--) buff = buff*3 + org.coefficients[i+k]; // Supposing the numbers in base 3 are in big endian notation
    dest[j] = (char)buff;
}

void Encryption::RpPolynomialPlainTextSave(const RpPolynomial& org, const char* name, bool saveAsText){
    int byteArrSize = outputPlainTextMaxSizeInBytes();
    char* byteArr = NULL;
    std::string fileOutputName = name == NULL ? "NTRURpPolynomial" : name;
    const char ntrup[] = "NTRUp";                                               // -This will indicate the binary file is saving a NTRU polynomial with
    short N = NTRU_N, q = NTRU_Q;                                                     //  coefficients in Zp
    std::ofstream file;
    if(saveAsText)  file.open(fileOutputName);
    else            file.open(fileOutputName,std::ios::binary);
    if(file.is_open()) {
        file.write(ntrup, 5);                                                   // -The first five bytes are for the letters 'N' 'T' 'R' 'U' 'p'
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the degree of the polynomial
        byteArr = new char[byteArrSize];                                        // -The following bytes are for the polynomials coefficients
        RpPolynomialtoBytes(org, byteArr, false);                               // -Assuming data has no format.
        file.write(byteArr, byteArrSize);                                       // -Saving plain data.
    } else {
        throw FileIOException("In void RpPolynomial::save(const char*) const: Could not create file for " + fileOutputName);
    }
    if(byteArr != NULL) delete[] byteArr;
}

void Encryption::RpPolynomialWriteFile(const RpPolynomial& org, const char* fileName, bool writeAsText){
    int byteArrSize = outputPlainTextMaxSizeInBytes();
    int fileSize = 0;
    char* byteArr = NULL;
    std::string fileOutputName = fileName == NULL ? "FromRpPolynomial" : fileName;
    std::ofstream file;
    if(writeAsText) file.open(fileName);
    else            file.open(fileName,std::ios::binary);
    if(file.is_open()) {
        byteArr = new char[byteArrSize];                                        // -The following bytes are for the polynomials coefficients
        RpPolynomialtoBytes(org, byteArr, false);                               // -Writing bytes in plain text mode.
        for(fileSize = byteArrSize - 1; fileSize >= 0; fileSize--){             // -We are supposing that this polynomial holds the data of a file, where the end
            if(byteArr[fileSize] == (char)0x80) {                               //  of the file content is marked by a byte with value 0x80 (1000,0000). This for
                break;
            }
        }
        if(fileSize > 0) file.write(byteArr, fileSize);
        else {
            std::cout << "In file NTRUencryption.cpp,  function void Encryption::RpPolynomialWriteFile(const RpPolynomial&, const char*, bool) const.\n";
            std::cout << "I could not find the mark that flags the end of the content. I will proceed to write the file using the entire polynomial." << std::endl;
            fileSize = byteArrSize;
        }
    } else {
        throw FileIOException("Encryption::RpPolynomialWriteFile(const RpPolynomial&, const char*, bool): Could not write " + fileOutputName + " from RpPolynomial.");
    }
    if(byteArr != NULL) delete[] byteArr;
}

void Encryption::RqPolynomialSave(const RqPolynomial& pl, const char* name, bool saveAsText){
    int byteArrSize = Encryption::cipherTextSizeInBytes();
    char* byteArr = NULL;
    const char ntruq[] = "NTRUq";                                               // -This will indicate the binary file is saving a Rq polynomial
    short N = NTRU_N, q = NTRU_Q;

    std::ofstream file;                                                         //  coefficients in Zp (p)
    if(name == NULL) {
        if(saveAsText)  file.open("RpPolynomial.ntrup");
        else            file.open("RpPolynomial.ntrup",std::ios::binary);
    } else{
        if(saveAsText)  file.open(name);
        else            file.open(name, std::ios::binary);
    }
    if(file.is_open()) {
        file.write(ntruq, strlen(ntruq));
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the q value
        byteArr = new char[byteArrSize];
        pl.toBytes(byteArr);
        file.write(byteArr, byteArrSize);
    } else {
        throw FileIOException("In RqPolynomial::save(const char*) const: Could not create file for RqPolynomial");
    }
    if(byteArr != NULL) delete[] byteArr;
}

RqPolynomial Encryption::RqPolynomialFromFile(const char* fileName){
    const std::string thisFunc = "RqPolynomial::fromFile(const char*)";
    const char ntruq[] = "NTRUq";                                               // -This will indicate the binary file is saving a NTRU (NTRU) RqPolynomial
    int byteArrSize = Encryption::cipherTextSizeInBytes(), ntruqsz = strlen(ntruq);
    char* byteArr = NULL;
    short n = NTRU_N, Q = NTRU_Q;
    RqPolynomial out;
    std::ifstream file;
    file.open(fileName, std::ios::binary);
    if(!file.is_open()){
        throw FileIOException("In " + thisFunc + ":File opening failed");
    }
    byteArr = new char[ntruqsz+1];
    file.read(byteArr, ntruqsz);                                                // -Reading the firsts bytes hoping "NTRUpublicKey" or "NTRUprivatKey" apears
    byteArr[ntruqsz] = 0;                                                       // -End of string
    if(strcmp(byteArr, ntruq) == 0) {                                           // -Testing if file saves a NTRU private key
        delete[] byteArr; byteArr = NULL;
        file.read((char*)&n, 2);
        file.read((char*)&Q, 2);
        if(n != NTRU_N || Q != NTRU_Q) {
            delete[] byteArr; byteArr = NULL;
            std::stringstream ss;
            ss << "In " << thisFunc << ": Parameters retreaved from file do not match with this program parameters\n";
            ss << "From " << fileName << ": N == " << n << ", q == " << Q << ". From this program: N == " << NTRU_N << ", q == " << NTRU_Q << "\n";
            ss << "Could not agree on parameters";
            throw ParameterMismatchException(ss.str());
        }
        byteArr = new char[byteArrSize];
        file.read(byteArr, byteArrSize);                                        // -Reading the coefficients of the polynomial
        file.close();
        out = RqPolynomial(byteArr, byteArrSize);                               // -Building polynomials in plintext mode
        delete[] byteArr; byteArr = NULL;
        return out;
    } else{
        throw FileIOException("In " + thisFunc + ": Not a valid NTRU::RqPolynomial file.");
    }
}

/********************************************************************* End: File Handling ***********************************************************************/

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Encryption keys |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

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
static RandInt randomIntegersN(0, NTRU_N - 1);

RpPolynomial Encryption::randomTernary() {
    const int d = NTRU_N/3;
    int i, j, dd = d;
    RpPolynomial r;
	r.coefficients = new RpPolynomial::Z3[NTRU_N];
	for(i = 0; i < NTRU_N; i++) r.coefficients[i] = RpPolynomial::Z3::_0_;
	while(dd > 0) {                                                             // Putting the ones first
        j = randomIntegersN();
        if(r.coefficients[j] == RpPolynomial::Z3::_0_) {
            r.coefficients[j] = RpPolynomial::Z3::_1_;
            dd--;
        }
	}
	dd = d;
	while(dd > 0) {                                                             // Then the negative ones
        j = randomIntegersN();
        if(r.coefficients[j] == RpPolynomial::Z3::_0_) {
            r.coefficients[j] = RpPolynomial::Z3::_2_;                          // Two is congruent with -1 modulo 3
            dd--;
        }
	}
	return r;
}

void Encryption::interchangeZeroFor(RpPolynomial::Z3 t, RpPolynomial& pl){
    int i = randomIntegersN(), j;
    while(pl.coefficients[i] != RpPolynomial::Z3::_0_) i = randomIntegersN();   // -Looking for a zero in a random position
    j = i;
    while(pl.coefficients[i] != t) i = randomIntegersN();                       // -Looking for a k in a random position
    pl.coefficients[j] = t; pl.coefficients[i] = RpPolynomial::Z3::_0_;         // -Changing zero for k
}

RqPolynomial Encryption::productByPrivatKey(const RqPolynomial& P) const{
    return convolutionRq(this->privatKey, P).timesThree() + P;                  // Private key f has the form 1+p·F0, so f*P = P+p(P*F0)
}

RqPolynomial Encryption::productByPrivatKey(const R2Polynomial& P) const{
    RqPolynomial r = convolutionRq(P, this->privatKey).timesThree();            // Private key f has the form 1+p·F0, so f*P = P+p(P*F0)
    for(int i = 0; i < NTRU_N; i++) if(P.coefficients[i] != 0) r.coefficients[i]++;
    return r;
}

void Encryption::setKeys() {
    const std::string thisFunc = "void Encryption::setKeys()";
	R2Polynomial R2_privateKeyInv;
	R2Polynomial R2_gcdXNmns1;
	this->privatKey = this->randomTernary();
	R2Polynomial R2_privateKey(this->privatKey);
	R2_privateKey.negFirstCoeff();                                              // -Embeding 3·privatKey0 + 1 in R2Polynomial. For i > 0 and int polynoimal F:
	int counter, k = 2, l = 1;                                                  //  (3·F[i]) mod 2 -> [(3 mod 2)·F[i]] mod 2 -> [1·F[i]] mod 2 = F[i] mod 2.In
                                                                                //  other hand, (F[0]+1) mod 2 -> [(F[0] mod 2)+1)] mod 2 -> neg[(F[0]+1) mod 2]
    try{
        R2_gcdXNmns1 = R2_privateKey.gcdXNmns1(R2_privateKeyInv);
    }catch(MathException& exp) {
        exp.add_trace_point("Caught in " + thisFunc + " while finding inverse in Z2[x]/(x^N-1)");
        throw;
    }
	counter = 1;
	while(R2_gcdXNmns1 != 1) {                                                  // -Looking for a valid private key.
        if((counter & 1) == 0)  this->interchangeZeroFor(RpPolynomial::_1_,privatKey);
        else                    this->interchangeZeroFor(RpPolynomial::_2_,privatKey);
        R2_privateKey = this->privatKey;
        //if((counter&3)==0){
            //std::cout << "Source/NTRUencryption.cpp; void setKeys(bool showKeyCreationTime). Counter = " << counter << "\n";\\Debuggin purposes
        //}
        R2_privateKey.negFirstCoeff();
        try{ R2_gcdXNmns1 = R2_privateKey.gcdXNmns1(R2_privateKeyInv); }
        catch(MathException& exp) {
            exp.add_trace_point("Caught in " + thisFunc + " while finding inverse in Z2[x]/(x^N-1)");
            throw;
    	}
        counter++;
    }                                                                           // -Shearch finished.
    // -This hole section can be optimized by making it a function like lift R2-Rq for private key
    this->publicKey = convolutionRq(R2_privateKeyInv, 2 - productByPrivatKey(R2_privateKeyInv));
    k <<= l; l <<= 1;
	while(k < NTRU_Q) {
        this->publicKey = this->publicKey*(2 - productByPrivatKey(publicKey));
        k <<= l; l <<= 1;
    }                                                                           // -At this line, we have just created the private key and its inverse
    RqPolynomial t = productByPrivatKey(this->publicKey);                       // -Testing the if the statement above is true (Debugging purposes)
    /*t.mods_q();
    if(!t.equalsOne()) {
        std::cout << thisFunc << "::Parameters: N = "<< NTRU_N << ", q = " << NTRU_Q << " --------------" << std::endl;
        t.println("this->publicKey*this->privateKey");
        cerrMessageBeforeThrow(thisFunc,"Public key inverse in Zq[x]/x^N-1 finding failed");
        throw std::runtime_error("Exception in public key inverse creation");
    }*/
    this->publicKey = convolutionRq(this->randomTernary(), this->publicKey).timesThree(); // -Multiplication by the g polynomial.
    this->publicKey.mods_q();
}

void Encryption::setKeysFromPrivKey() {                                         // -In this function we're assuming we already have a valid private key,
    const std::string thisFunc = "void Encryption::setKeysFromPrivKey()";       //  this is, a privatKey with inverse mod 2.
    R2Polynomial R2_privateKey(this->privatKey);
    R2Polynomial R2_privateKeyInv;
    R2Polynomial R2_gcdXNmns1;
    int k = 2, l = 1;
    R2_privateKey.negFirstCoeff();
    try{ R2_gcdXNmns1 = R2_privateKey.gcdXNmns1(R2_privateKeyInv); }
    catch(MathException& exp) {
        exp.add_trace_point("Caught in " + thisFunc + " while finding inverse in Z2[x]/(x^N-1)");
        throw;
    }
    // -This hole section can be optimized by making it a function like lift R2-Rq for private key
    this->publicKey = convolutionRq(R2_privateKeyInv, 2 - productByPrivatKey(R2_privateKeyInv));
    k <<= l; l <<= 1;
	while(k < NTRU_Q) {
        this->publicKey = this->publicKey*(2 - productByPrivatKey(publicKey));
        k <<= l; l <<= 1;
    }                                                                           // -At this line, we have just created the private key and its inverse
    /*RqPolynomial t = productByPrivatKey(this->publicKey);                     // -Testing the if the statement above is true (Debugging purposes)
    t.mods_q();
    if(t != 1) {
        t.println("this->publicKey*this->privateKey");
        cerrMessageBeforeThrow(thisFunc,"Public key inverse in Zq[x]/x^N-1 finding failed");
        throw std::runtime_error("Exception in public key inverse creation");
    }*/
    this->publicKey = convolutionRq(this->randomTernary(), this->publicKey).timesThree(); // -Multiplication by the g polynomial.
    this->publicKey.mods_q();
}

// ____________________________________________________________________ Encryption keys ___________________________________________________________________________

RqPolynomial Encryption::encrypt(const char bytes[], size_t size) const{
    RpPolynomial msg = RpPolynomialFromBytes(bytes, size, false);
    RqPolynomial encryptedMsg = this->encrypt(msg);
    return encryptedMsg;
}

RqPolynomial Encryption::encrypt(const RpPolynomial& msg) const{
    RqPolynomial encryption;
    int*  randTernary = new int[NTRU_N];                                           // -Will represent the random polynomial needed for encryption
    const int d = NTRU_N/3;
    int _d_ = d, i, j, k;
    for(i = 0; i < NTRU_N; i++) randTernary[i] = 0;
    while(_d_ > 0) {
        i = randomIntegersN();                                                  // -Filling with threes. It represent the random polynomial multiplied by p
        if(randTernary[i] == 0) {randTernary[i] =  1; _d_--;}                   //  ...
    }
    _d_ = d;
    while(_d_ > 0) {
        i = randomIntegersN();                                                  // -Filling with negative threes
        if(randTernary[i] == 0) {randTernary[i] = -1; _d_--;}                   //  ...
    }
	for(i = 0; i < NTRU_N; i++) {                                                  // -Convolution process
	    k = NTRU_N - i;
	    if(randTernary[i] != 0) {
	        if(randTernary[i] == 1) {
	            for(j = 0; j < k; j++)                                          // -Ensuring we do not get out of the polynomial
                    encryption.coefficients[i+j] += this->publicKey[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        encryption.coefficients[k] += this->publicKey[j];           // Notice i+j = i + (k+NTRU_N-i), so i+j is congruent with k mod NTRU_N
	        }
	        if(randTernary[i] == -1) {
	            for(j = 0; j < k; j++)                                          // Ensuring we do not get out of the polynomial
                    encryption.coefficients[i+j] -= this->publicKey[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        encryption.coefficients[k] -= this->publicKey[j];           // Notice i+j = i + (k+NTRU_N-i), so i+j is congruent with k mod NTRU_N
	        }
	    }
	}
	for(i = 0; i < NTRU_N; i++) {                                                  // -Adding this polynomial (adding message)
	    if(msg.coefficients[i] == RpPolynomial::_1_) encryption.coefficients[i]++;
	    if(msg.coefficients[i] == RpPolynomial::_2_) encryption.coefficients[i]--;// -Number 2 is interpreted as -1.
	}
	encryption.mods_q();                                                        // -Obtaining center modulus for each coefficient
	delete[] randTernary;
	return encryption;
}

RqPolynomial Encryption::encryptFile(const char fileName[]) const{              // -Reads file content, writes in array, append a 0x80 at the end and encrypts.
    const std::string thisFunc = "Encryption::encryptFile(const char[])";
    const size_t plainTextMaxSize = inputPlainTextMaxSizeInBytes()-1;           // -Minus one because we will use one byte to mark the end of the file content.
    size_t fileInputArrSize = 0;
    char* fileInput = NULL;
    std::streampos fileSize;
    std::ifstream inputFile;
    NTRU::RqPolynomial enc_msg;
    std::string fileName_ = fileName == NULL ? "" :  fileName;                  // -If fileName is NULL, empty string will trow an exception at the moment of trying to open a file
    // -Start of: Getting file size.
    inputFile.open(fileName_, std::ios::binary | std::ios::ate);                // -Bynary mode. Positioning at the end of the file.
    if(inputFile.is_open()) fileSize = inputFile.tellg();                       // -Getting current position of the "get" pointer.
    else{
        throw FileIOException("In " + thisFunc + ": I could not open " + fileName_);
    }
    if((size_t)fileSize > plainTextMaxSize) {
        std::stringstream ss;
        ss << "File size exceeds the limit size (" << plainTextMaxSize << " bytes)\n";
        throw FileIOException(ss.str());
        //ss << "I will proceed to truncate the file\n";
        //fileSize = (std::streamoff)plainTextMaxSize;                          // -From here, input data has plainTextMaxSize bytes.
    }
    // -End of: Getting file size.
    inputFile.seekg(0, std::ios::beg);                                          // -Move back to the beginning
    // -Start of: Reading input file.
    fileInputArrSize = (size_t)fileSize;
    fileInput = new char[fileInputArrSize];
    inputFile.read(fileInput, fileSize);
    //displayByteArrayChar(fileInput, fileInputArrSize);std::cout << '\n';      // -Debugging purposes
    //displayByteArrayChar(fileInput, fileInputArrSize);std::cout << '\n';      // -Debuggin purposes
    // -End of: Reading input file.
    enc_msg = this->encrypt(fileInput, fileInputArrSize);                       // -Encrypting the bytes from the file and its size
    delete[] fileInput;
    return enc_msg;
}

RpPolynomial Encryption::decrypt(const RqPolynomial& e_msg) const{
    if(!this->validPrivateKey) {
        throw InvalidPrivateKey("In Encryption::decrypt(const RqPolynomial&) const: Object has no valid private key, it only has encryption capabilities.");
    }
    RqPolynomial msg_ = productByPrivatKey(e_msg);
    msg_.mods_q();
    RpPolynomial msg = mods_p(msg_);
    return msg;
}

RpPolynomial Encryption::decrypt(const char bytes[], size_t size) const{
    RpPolynomial msg;
    try{
        msg = this->decrypt(RqPolynomial(bytes, size));
    } catch(InvalidPrivateKey& exp){
        exp.add_trace_point("Caught in Encryption::decrypt(const char[], size_t) const");
    }
    return msg;
}
