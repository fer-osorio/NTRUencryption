#include<fstream>
#include<cstring>
#include<exception>
#include<limits>
#include"Settings.hpp"

#define BYTES_FOR_FILE_SIZE 2                                                   // -Size of the information we will append to the data we intend to encrypt. Now
#define BUFFER_SIZE 1025
#define UPPER_BOUND 4097                                                        // -Intended for indices that run through strings. This selection has a reason; it
                                                                                //  is the upper limit for strings representing directories path

/************************************************************************* Attributes *****************************************************************************/

static NTRU::Encryption NTRUencryption;                                         // -Declaring encryption object. Should not be used jet

/************************************************************************* Attributes *****************************************************************************/


static void cerrMessageBeforeThrow(const char callerFunction[], const char message[]) {
    std::cerr << "In file Source/NTRUencryption.cpp, function " << callerFunction << ": " << message << '\n';
}

static void cerrMessageBeforeReThrow(const char callerFunction[], const char message[] = "") {
    std::cerr << "Called from: File Source/NTRUencryption.cpp, function " << callerFunction << " ..."<< message << '\n';
}

union ushort_to_char {                                                          // -Casting from short to an array of two chars
    unsigned short ushort;
    char         chars[2];
};

const NTRU_N NTRU_N_values[]    = {    _509_,     _677_ ,    _701_,     _821_,     _1087_,     _1171_,     _1499_};// -All the possible values for the N
const char NTRU_N_valuesList[]  = "(0) _509_, (1) _677_, (2) _701_, (3) _821_, (4) _1087_, (5) _1171_, (6) _1499_";// -Useful for CLI
const size_t NTRU_N_amount      = sizeof(NTRU_N_values)/sizeof(NTRU_N_values[0]);

const NTRU_q NTRU_q_values[]    = {    _2048_,     _4096_ ,     _8192_};        // -All the possible values for the q
const char NTRU_q_valuesList[]  = "(0) _2048_, (1) _4096_ , (2) _8192_";        // -Useful for CLI
const size_t NTRU_q_amount      = sizeof(NTRU_q_values)/sizeof(NTRU_q_values[0]);

const NTRU_p NTRU_p_values[]    = {_3_};

void showParameters(){
  printf("\t----------------------------------------\n");
  printf("\t| NTRU parmeters: N = %d, q = %d\t|\n", NTRU::get_N(), NTRU::get_q());
  printf("\t----------------------------------------\n");
}

/*
Valid file name or path grammar

'l' will denote letters in English alphabet, either lower case or upper case.
	For convenience, we will admit '_' and '-' as letters
'd' for digits 0, 1,..., 9
Sld	string of letters and digits that always starts with a letter
FN  File Name
PT  Path
Sled string of letters, digits and spaces. It never starts or ends with a space
FNWE File Name With Spaces

Sld	->	l·Sld	| l·d·Sld	|	l	|	l·d	                                    // -Concatenation of letters and digits that always start with a letter
Sled->	l·SPACES·Sled	| l·d·SPACES·Sled	|	l	|	l·d	                    // -Concatenation of letters and digits that always start with a letter

FN	->	Sld·FN	|	l                                                           // -File Name can not start with a digit; a single letter can be a File Name
FN	->	.Sld·FN |	FN.Sld·FN	|	.Sld                                        // -Can not finish with a point nor have two consecutive points

FN  ->  "FNWE"                                                                  // -If double quotes are presented at the beginning of the string, the grammar
FNWE->	Sled·FN	|	l                                                           //  accepts spaces in the middle of the string until the next double quote is found
FNWE->	.Sled·FN|	FN.Sled·FN	|	.Sled

FN  ->  FN/·Sld·FN   |   ../FN	|	/·Sld·FN                                    // -Considering file paths (Absolute Paths and Relative Paths) as file names

Note: SPACES can be represented by single spaces, or a concatenation of spaces
*/

enum CharType {zero, letter, digit, dot, underscore, hyphen, slash, space, singleQuote ,doubleQuote, notAllowed};

CharType characterType(const char c) {                                          // -Just checks if the character may be used in a File Name
    if((c > 64 && c < 91) || (c > 96 && c < 123)) return letter;                // -Letters
    if(c > 47 && c < 58) return digit;                                          // -Decimal digits
    if(c == '.') return dot;                                                    // -And finally special symbols
    if(c == '_') return underscore;                                             //  ...
    if(c == '-') return hyphen;                                                 //  ...
    if(c == '/') return slash;                                                  //  ...
    if(c == ' ') return space;                                                  //  ...
    if(c == '\'')return singleQuote;                                            //  ...
    if(c == '"') return doubleQuote;                                            //  ...
    if(c == 0)   return zero;

    return notAllowed;                                                          // -Anything else is a not allowed symbol
}

static bool isLetter(const char c) {
    return (c > 64 && c < 91) ||                                                // -Upper case letters
           (c > 96 && c < 123)||                                                // -Lower case letters
            c == '_' || c == '-';                                               // -Seeking simplicity, '-' and '_' will be consider as letters
}

static bool isDigit(const char c) {
    return (c > 47 && c < 58);
}

static bool isLetterOrDigit(const char c) {
    return (c > 47 && c < 58) ||                                                // -Decimal digits
           (c > 64 && c < 91) ||                                                // -Upper case letters
           (c > 96 && c < 123)||                                                // -Lower case letters
            c == '_' || c == '-';                                               // -Seeking simplicity, '-' and '_' will be consider as letters
}

struct AnalyzeStringAsFileName {
    private:                                                                    // -Attributes
    const char* str = NULL;
    size_t      size = 0;
    unsigned    currentIndex = 0;
    private:                                                                    // -Functions for syntax analysis
    void cerrSyntaxErrMsg(const char[]);
    bool Sld();                                                                 // -The returned bool flags the founding of zero byte or the characters '\'' or '"')
    bool FN ();

    AnalyzeStringAsFileName(const char str_[]);

    public:
    static bool isValidFileName(const char str[]);
};

void AnalyzeStringAsFileName::cerrSyntaxErrMsg(const char msg[]) {
    const char SinErr[] = "Syntax Error: ";
    size_t sz = strlen(SinErr) + this->currentIndex;
    unsigned i;
    std::cerr << SinErr;
    std::cerr << this->str << std::endl;
    for(i = 0; i < sz; i++) std::cerr << ' ';
    std::cerr << "^~~~ " << msg << std::endl;
    if(isDigit(this->str[this->currentIndex]) || this->str[this->currentIndex] == ' ') {
        if(isDigit(this->str[this->currentIndex]))
            while(isDigit(this->str[this->currentIndex])) this->currentIndex++;
        else
            while(this->str[this->currentIndex] == ' ') this->currentIndex++;
    } else
        this->currentIndex++;
    if(this->str[this->currentIndex] != 0 && this->currentIndex < this->size) {
        this->FN();
    }
}

bool AnalyzeStringAsFileName::Sld() {
    if(!isLetter(this->str[this->currentIndex])) {                              // -This ensures we have a letter at the beginning of the string
        if(isDigit(this->str[this->currentIndex])) {
            this->cerrSyntaxErrMsg("File name can not start with a digit.");
            return false;
        }
        this->cerrSyntaxErrMsg("Expected a character from English alphabet.");
        return false;
    }
    unsigned i = 0;
    for(i = ++this->currentIndex; isLetterOrDigit(this->str[i]) || this->str[i] == ' '; i++) {} // -Running trough letters, digits and spaces
    this->currentIndex = i;

    if(this->str[this->currentIndex] == 0) {
        if(this->str[this->currentIndex-1] == ' ') {
            this->cerrSyntaxErrMsg("File Name/Path can not finish with spaces.");
            return false;
        }
        return true;
    }
    return this->FN();
}

bool AnalyzeStringAsFileName::FN() {
    CharType ct = characterType(this->str[this->currentIndex]);
    switch(ct) {
        case slash:                                                             // -Allowing slash for file paths
            this->currentIndex++;
            return this->Sld();
        case letter:
        case underscore:
        case hyphen:
            return this->Sld();                                // -Read (always starting with a letter) letters, digits and (if allowed) spaces;
            break;                                                              //  when a proper ending character is found (0,'\'','"',' ') then return
        case dot:                                                               // -Cases for dot
            if(this->str[++this->currentIndex] == '.') {                        // -The string "../" is allowed as a sub-string so we can use relative paths
                if(this->str[++this->currentIndex] == '/') {
                    this->currentIndex++;
                    return this->FN();
                } else {
                    this->cerrSyntaxErrMsg("Syntax Error: Expected '/' character.");
                    return false;
                }
            } else {
                return this->Sld();                            // -This lines can be interpreted as: Read (always starting with a letter) letters,
            }                                                                   //  digits and (if allowed) spaces; when a proper ending character is found
        case digit:
            this->cerrSyntaxErrMsg("File name can not start with a digit.");
            return false;
        case space:
            this->cerrSyntaxErrMsg("File name can not start with a space.");
            return false;
        case singleQuote:
            this->cerrSyntaxErrMsg("Not Expecting a single quote here.");
            return false;
        case doubleQuote:
            this->cerrSyntaxErrMsg("Not expecting a double quote here.");
            return false;
        case notAllowed:
            this->cerrSyntaxErrMsg("Unexpected character/symbol.");
            return false;
        case zero:
            this->cerrSyntaxErrMsg("Unexpected end of string.");
            return false;
    }
    return false;
}

AnalyzeStringAsFileName::AnalyzeStringAsFileName(const char _str_[]): str(_str_) {
    if(this->str != NULL) while(this->str[this->size] != 0) this->size++;
}

bool AnalyzeStringAsFileName::isValidFileName(const char str[]) {                 // -Validating string as a file name
    if(str == NULL) {
        std::cerr <<
        "In Apps/Settings.cpp, function bool AnalyzeStringAsFileName::isValidFileName(const char str[]): Passing a NULL pointer as argument.\n"
        "Returning false.\n";
        return false;
    }
    if(str[0] == 0) return false;
    AnalyzeStringAsFileName s(str);
    return s.FN();
}

namespace Extension{
    enum IOfile{ none, bin, txt, unrecognized};
    enum EncryptedFile{ enc_bin,                                                // -Encryption binary
                        enc_txt };                                              // -Encryption text
    enum DecryptedFile{ dec_bin,                                                // -Decryption binary
                        dec_txt};                                               // -Decryption text

    static IOfile strToIOfileExt(const char str[]);
    static IOfile getExtension(const char fileName[]);
    static void   appendEncryptedFileExtension(EncryptedFile cf, char destination[]);
    static void   appendDecryptedFileExtension(DecryptedFile df, char destination[]);
    static void   NTRUsaveWith_enc_ext(const NTRU::ZqPolynomial& p, const char* name, EncryptedFile enc_ext);
    static void   NTRUsaveWith_dec_ext(const NTRU::ZpPolynomial& p, const char* name, DecryptedFile dec_ext);
};

namespace FileHandling{
    namespace NTRUPolynomials{
        static void formatedSaving(const NTRU::ZpPolynomial&, const char[], Extension::IOfile);
        static NTRU::ZqPolynomial formatDataEncryption(const char data[], size_t size);
    };
    struct InputFile{
        private:
        std::ifstream      file = std::ifstream();
        std::streampos     size = -1;
        Extension::IOfile  ext  = Extension::none;
        InputFile();
        InputFile(const InputFile&);
        InputFile& operator = (const InputFile&);
        public:
        InputFile(const char fname[]);
        ~InputFile()   { this->file.close(); }

        std::streampos getSize() const{ return this->size; }
        Extension::IOfile retreaveExtension() const{ return this->ext; }
        bool is_open() const{ return this->file.is_open();}
        std::istream& read(char* s, std::streamsize sz) { return this->file.read(s,sz); }
    };
    static void encryptf(const char fname[]);
    static void decryptf(const char fname[]);
};

void FileHandling::NTRUPolynomials::formatedSaving(const NTRU::ZpPolynomial& p, const char fname[], Extension::IOfile io_ext){
    const char thisFunc[] = "void FileHandling::NTRUPolynomials::formatedSaving(const NTRU::ZpPolynomial&, const char[])";
    if(!AnalyzeStringAsFileName::isValidFileName(fname)){
        cerrMessageBeforeThrow(thisFunc, "Trying to create a file with a in-valid file name. I refuse to proceed...");
        std::cout << "File name rejected: " << fname << '\n';
        throw std::runtime_error("Not a valid file name");
    }
    char* bytes = new char[p.sizeInBytes(true)];                                // -Plaintext mode
    ushort_to_char fsize;
    std::ofstream ofile;
    p.toBytes(bytes,true);                                                      // -Plaintext mode
    memcpy(fsize.chars, bytes, BYTES_FOR_FILE_SIZE);                            // -The format it refers is: the first two bytes will be interpreted as the file
    if(io_ext == Extension::txt){                                               //  size, and the rest of the bytes will contain the information of the file.
        ofile.open(fname);
    } else {
        ofile.open(fname, std::ios::binary);
    }
    if(ofile.is_open()){
        ofile.write(&bytes[BYTES_FOR_FILE_SIZE], fsize.ushort);
    } else{
        cerrMessageBeforeThrow(thisFunc, "I could not create output file...");
        throw std::runtime_error("Unable to write file");
    }
}

NTRU::ZqPolynomial FileHandling::NTRUPolynomials::formatDataEncryption(const char data[], size_t size){
    size_t szdata_size = size + BYTES_FOR_FILE_SIZE;                            // -Will use the two first bytes for data size and then it will encrypt.
    char* szdata = new char[szdata_size];
    ushort_to_char encf_size = {0};
    encf_size.ushort = size;
    memcpy(szdata, encf_size.chars, BYTES_FOR_FILE_SIZE);
    memcpy((char*)&szdata[BYTES_FOR_FILE_SIZE], data, size);
    NTRU::ZqPolynomial enc_msg = NTRUencryption.encrypt(szdata, szdata_size);
    delete[] szdata;
    return enc_msg;
}

FileHandling::InputFile::InputFile(const char fname[]): ext(Extension::getExtension(fname)) {
    bool is_bin = false;
    switch(ext){                                                                // -File extension will determine the way we will tread the file
        case Extension::txt:
            this->file.open(fname, std::ios::ate);
            break;
        case Extension::bin:
            is_bin = true;
            break;
        case Extension::unrecognized:
            std::cout << "Could not recognize the extension; I will proceed using binary mode." << std::endl;
            is_bin = true;
            break;
        case Extension::none:
            std::cout << "No extension found; I will proceed using binary mode." << std::endl;
            is_bin = true;
            break;
    }
    if(is_bin) this->file.open(fname, std::ios::binary | std::ios::ate);
    if(this->file.is_open()) this->size = this->file.tellg();
    this->file.seekg(0, std::ios::beg);                                         // -Move back to the beginning
}

void FileHandling::encryptf(const char fname[]) {
    const char      thisFunc[] = "static void cipherbinFile(const char fname[])";
    std::streampos  fileSize;
    const size_t    plainTextMaxSize = NTRUencryption.plainTextMaxSizeInBytes();
    unsigned short  fileSzPlusBytesForSz;
    char*           fileInput = NULL;
    NTRU::ZqPolynomial  enc_msg;
    FileHandling::InputFile in_f(fname);
    Extension::IOfile ext = in_f.retreaveExtension();

    if (!in_f.is_open()) {
        cerrMessageBeforeThrow(thisFunc, "Error opening file for encryption");
        throw std::runtime_error("File opening failed");
    }
    fileSize = in_f.getSize();                                                  // -Get the file size
    if((size_t)fileSize > plainTextMaxSize - BYTES_FOR_FILE_SIZE) {
        cerrMessageBeforeThrow(thisFunc, "");
        std::cerr << "File size exceeds the limit size (" << plainTextMaxSize - BYTES_FOR_FILE_SIZE << " bytes)\n";
        throw std::runtime_error("Upper bound for file size exceeded");
    }
    fileSzPlusBytesForSz = (unsigned)fileSize + (unsigned)BYTES_FOR_FILE_SIZE;  // -This will allow us two write the size in the input array for encryption.
    fileInput = new char[fileSzPlusBytesForSz];
    memcpy(fileInput, (char*)&fileSize, BYTES_FOR_FILE_SIZE);                   // -Writing file size
    in_f.read((char*)&fileInput[BYTES_FOR_FILE_SIZE], fileSize);
    enc_msg = NTRUencryption.encrypt(fileInput, fileSzPlusBytesForSz);          // -Encrypting the bytes from the file and its size
    enc_msg.println("Encrypted message:");
    delete[] fileInput;
    try {
        if(ext == Extension::txt) Extension::NTRUsaveWith_enc_ext(enc_msg, fname, Extension::EncryptedFile::enc_txt);
        else Extension::NTRUsaveWith_enc_ext(enc_msg, fname, Extension::EncryptedFile::enc_bin);
    }
    catch(const std::runtime_error&) {
        cerrMessageBeforeReThrow(thisFunc);
        throw;
    }
}

void FileHandling::decryptf(const char fname[]){
    const char      thisFunc[] = "void decryptFileBinMode(const char fname[])";
    char*           buff     = NULL;
    const char      ntruq[]  = "NTRUq";
    const size_t    ntruqlen = strlen(ntruq);
    size_t          lenInBytes = 0;
    NTRU_N N = _509_;
    NTRU_q q = _2048_;
    FileHandling::InputFile in_f(fname);
    Extension::IOfile ext = in_f.retreaveExtension();

    if (!in_f.is_open()) {
        cerrMessageBeforeThrow(thisFunc, "Error opening file for decryption");
        throw std::runtime_error("File opening failed");
    }
    buff = new char[ntruqlen + 1];
    in_f.read(buff, ntruqlen);
    buff[ntruqlen] = 0;
    if(strcmp(buff, ntruq) == 0) {
        in_f.read((char*)&N, 2);                                             // -A short int for the degree of the polynomial
        in_f.read((char*)&q, 2);                                             // -A short int for the q value
        lenInBytes = size_t(N*NTRU::ZqPolynomial::log2(q)/8 + 1);
        delete[] buff;
        buff = new char[lenInBytes];
        in_f.read(buff, (std::streamsize)lenInBytes);
        NTRU::ZpPolynomial msg = NTRUencryption.decrypt(buff, lenInBytes);
        msg.println("Decrypted Message");
        delete[] buff;
        try{
            if(ext == Extension::txt) Extension::NTRUsaveWith_dec_ext(msg, fname, Extension::dec_txt);
            else Extension::NTRUsaveWith_dec_ext(msg, fname, Extension::dec_bin);
        } catch(const std::runtime_error&){
            cerrMessageBeforeReThrow(thisFunc);
            throw;
        }
    } else {
        if(buff != NULL) delete[] buff;
        cerrMessageBeforeThrow(thisFunc, "Not a ntruq file");
        throw std::runtime_error("Not valid input file");
    }
}

Extension::IOfile Extension::strToIOfileExt(const char str[]) {
    if(str == NULL) {
        std::cerr << "In file Source/NTRUencryption.cpp, function static Extension::IOfile strToFileExtension(const char str[])";
        return Extension::unrecognized;
    }
    if(strcmp(str, "bin") == 0) return Extension::bin;
    if(strcmp(str, "txt") == 0) return Extension::txt;
    if(str[0] == 0)             return Extension::none;
    return Extension::unrecognized;
}

Extension::IOfile Extension::getExtension(const char fileName[]) {
    if(fileName == NULL) {
        std::cerr << "In file Source/NTRUencryption.cpp, function static FileExtensions getExtension(const char fileName[]). filename == NULL...\n";
        return Extension::unrecognized;
    }
    int i = -1;
    while(fileName[++i] != 0) {}                                                // -Looking for end of string
    while(fileName[i] != '.' && i >= 0) {i--;}                                  // -Looking for last point

    if(i >= 0)  return Extension::strToIOfileExt(&fileName[++i]);
    return Extension::none;
}


void Extension::appendEncryptedFileExtension(Extension::EncryptedFile cf, char destination[]) {
    int i = -1;
    while(destination[++i] != 0) {}                                             // -Looking for end of string
    while(destination[i] != '.' && i >= 0) {i--;}                               // -Looking for last point
    if(i >= 0) destination[i] = 0;
    switch(cf) {                                                                // -Concatenating the extension at the end of destination
        case enc_bin:
            strcat(destination, "_enc.bin");
            break;
        case enc_txt:
            strcat(destination, "_enc.txt");
            break;
    }
}

void Extension::appendDecryptedFileExtension(Extension::DecryptedFile df, char destination[]) {
    int i = -1;
    while(destination[++i] != 0) {}                                             // -Looking for end of string
    while(destination[i] != '.' && i >= 0) {i--;}                               // -Looking for last point
    if(i >= 0) destination[i] = 0;
    switch(df) {                                                                // -Concatenating the extension at the end of destination
        case dec_bin:
            strcat(destination, "_dec.bin");
            break;
        case dec_txt:
            strcat(destination, "_dec.txt");
            break;
    }
}

void Extension::NTRUsaveWith_enc_ext(const NTRU::ZqPolynomial& p, const char* name, EncryptedFile enc_ext){
    char new_name[UPPER_BOUND];
    strcpy(new_name, name);
    if(enc_ext == Extension::enc_bin) appendEncryptedFileExtension(enc_bin, new_name);
    if(enc_ext == Extension::enc_txt) appendEncryptedFileExtension(enc_txt, new_name);
    try {
        if(enc_ext == Extension::enc_bin) p.save(new_name);
        if(enc_ext == Extension::enc_txt) p.save(new_name, true);
    }
    catch(const std::runtime_error&) {
        cerrMessageBeforeReThrow("static void NTRUsaveWith_enc_ext(const NTRU::ZqPolynomial&, const char*)");
        throw;
    }
}

void Extension::NTRUsaveWith_dec_ext(const NTRU::ZpPolynomial& p, const char* name, DecryptedFile dec_ext){
    char new_name[UPPER_BOUND];
    strcpy(new_name, name);
    if(dec_ext == Extension::dec_bin) appendDecryptedFileExtension(dec_bin, new_name);
    if(dec_ext == Extension::dec_txt) appendDecryptedFileExtension(dec_txt, new_name);
    try {
        if(dec_ext == Extension::dec_bin) FileHandling::NTRUPolynomials::formatedSaving(p, new_name, Extension::bin);
        if(dec_ext == Extension::dec_txt) FileHandling::NTRUPolynomials::formatedSaving(p, new_name, Extension::txt);
    }
    catch(const std::runtime_error&) {
        cerrMessageBeforeReThrow("static void NTRUsaveWith_dec_ext(const NTRU::ZpPolynomial&, const char*, EncryptedFile dec_ext)");
        throw;
    }
}

namespace Options {                                                             // -Name space destined for naming the main process executed by the program
	enum Key_retreaving {                                                       // -We can say each enumeration list all the possibilities for the refereed action
		RetrieveFromFile,
		CreateNew
	};
	static bool isKeyRetreavingValue(int t) {
		return
		t == RetrieveFromFile ||
		t == CreateNew;
	}

    enum Cipher_object {                                                        // -Actions a cipher object can do over a file
        Ciphering,
        Deciphering
    };

	enum Encryption_main_menu {                                                 // -For encryption program, these are the first options are shown to the user
	    encryptFiles,
	    encryptTextFromCLI,
	    saveKey
	};
	static bool isEncryptionMainMenuValue(int t) {
		return
		t == encryptFiles ||
		t == encryptTextFromCLI ||
		t == saveKey;
	}
	static bool isEncryptionMainMenuValueNoSaveKey(int t) {
		return
		t == encryptFiles ||
		t == encryptTextFromCLI;
	}
};

namespace CLI {                                                                 // -Functions that interact with user through CIL
    static int  validInput_int();                                               // -Checks valid integer input from CLI. If valid, returns the input
    static void getLine(const std::string message, char destination[]);         // -Get input string from console. Finish with a line ending '\n'
    static int  retreaveValidOption(std::string optionsString, bool (validOptionCriteria)(int)); // -Force the user to select a valid option
    static void retreaveValidFileName(const char message[], char name_dest[]);  // -Obtain a valid file input from CLI.
    static void getLineAndRetreaveKeyFromFile(const char appendMsg[] = "key");  // -Retrieve line, interprets it as a file and tries to build key from that file
    static void retreaveKey(Options::Key_retreaving, const char addedMsg[]=""); // -Retrieve key accordingly to its argument
    static void getFilesAndCipherAct(Options::Cipher_object);                   // -Get line, interpret it as a sequence of files and try to encrypt/decrypt them
    static void retreaveTexAndEncrypt();                                        // -Gets input from CIL, encrypt and saves it int a text file
};

static int  subStringDelimitedBySpacesOrQuotation(const char* source, const int start, char* destination);
static void setEncryptionObject();
void setEncryptionObjectFromFile(const char fname[]);
static void cipherObjectOverFile(const Options::Cipher_object, const char fname[]);// -Encrypts or decrypts file of second argument accordingly to the first argument
static void runProgram(const Options::Cipher_object op);

void CLI::getLine(const std::string message, char destination[]) {
    std::cout << message << " The maximum amount of characters allowed is " << NAME_MAX_LEN << ":\n";
    std::cin.getline(destination, NAME_MAX_LEN - 1, '\n');

}

int subStringDelimitedBySpacesOrQuotation(const char* source, const int startAt, char* destination) { // -Writes on destination the fist sub-string delimited by
    const char thisFunc[] = "int subStringDelimitedBySpacesOrQuotation(const char* source, const int startAt, char* destination)"; //    spaces or quotations
    if(source == NULL) {
        cerrMessageBeforeThrow(thisFunc, "Source pointer is a NULL pointer ~~> source == NULL.");
        throw std::invalid_argument("Source pointer is a NULL pointer ~~> source == NULL.\n");
    }
    if(source == NULL) {
        cerrMessageBeforeThrow(thisFunc, "Destination pointer is a NULL pointer ~~> destination == NULL.");
        throw std::invalid_argument("Destination pointer is a NULL pointer ~~> destination == NULL.");
    }
    int  i = 0, j = 0;
    char endingMark = 0;
    for(i = startAt; source[i] == ' ' || source[i] == '\t'; i++) {              // -Ignoring spaces and tabs
        if(i >= UPPER_BOUND) {
            cerrMessageBeforeThrow(thisFunc, "The upper bound for the index looking for the starting token was overreached ~~> i >= UPPER_BOUND.");
            throw std::runtime_error("Upper bound reached.");
        }
        if(source[i] == 0) {
            cerrMessageBeforeThrow(thisFunc, "End of string reached, nothing but spaces found ~~> source[i] == 0.");
            throw std::runtime_error("End of string reached.");
        }
    }
    endingMark = source[i];
    if(endingMark == '\'' || endingMark == '"') {                               // -If quotation is found (single or double), then the writing on destination will
        for(i++; source[i] != endingMark; i++, j++) {                           //  stop when the same quotation is found
            if(i >= UPPER_BOUND) {
                cerrMessageBeforeThrow(thisFunc, "The upper bound for the index looking for the starting token was overreached ~~> i >= UPPER_BOUND.");
                throw std::runtime_error("Upper bound reached.");
            }
            if(source[i] == 0) {
                cerrMessageBeforeThrow(thisFunc, "End of string reached, ending token not found ~~> source[i] == 0.");
                throw std::runtime_error("End of string reached.");
            }
            destination[j] = source[i];
        }
    } else {
        for(; source[i] != ' ' && source[i] != '\t' && source[i] != 0; i++, j++) { // -After ignoring tabs and spaces, the first character found is not a quotation,
            if(i >= UPPER_BOUND) {                                              //  then the writing on destination will stop when a space is found
                cerrMessageBeforeThrow(thisFunc, "The upper bound for the index looking for the ending token was overreached ~~> i >= UPPER_BOUND.");
                throw std::runtime_error("Upper bound reached.");
            }
            destination[j] = source[i];
        }
    }
    destination[j] = 0;
    return i;
}

void setEncryptionObject() {
    try { NTRUencryption = NTRU::Encryption(); }
    catch(const std::runtime_error&) {
        cerrMessageBeforeReThrow("void setEncryptionObject(NTRU_N _N_, NTRU_q _q_)");
        throw;
    }
}

void setEncryptionObjectFromFile(const char fname[]) {
    try { NTRUencryption = NTRU::Encryption(fname); }
    catch(const std::runtime_error&) {
        cerrMessageBeforeReThrow("void setEncryptionObject(NTRU_N _N_, NTRU_q _q_)");
        throw;
    }
}

void cipherObjectOverFile(const Options::Cipher_object act, const char fname[]) {
    const char  thisFunc[] = "void cipherObjectOverFile(const Options::Cipher_object act,  const char fname[])";
    switch(act) {
        case Options::Cipher_object::Ciphering:
            try{
                FileHandling::encryptf(fname);
            }catch(const std::runtime_error&) {
                cerrMessageBeforeReThrow(thisFunc);
                throw;
            }
            break;
        case Options::Cipher_object::Deciphering:
            try{
                FileHandling::decryptf(fname);
            }catch(const std::runtime_error&){
                cerrMessageBeforeReThrow(thisFunc);
                throw;
            }
            break;
    }
}

static const char invalidInputMsg[] = "\nInvalid input. Try again.\n";

static const char encryptionMainMenu[] =
"\nPress:\n"
"(0) to encrypt files.\n"
"(1) to encrypt text retrieved from console.\n"
"(2) to save encryption keys.\n";

static const char encryptionMainMenuNoSaveKey[] =
"\nPress:\n"
"(0) to encrypt files.\n"
"(1) to encrypt text retrieved from console.\n";

static const char keyRetreavingOptions[] =
"Would you like to:\n"
"(0) Retrieve encryption keys from file.\n"
"(1) Let this program generate the encryption keys.\n";

static std::string selectNTRU_N =
std::string("Select NTRU_N parameter:\n") + NTRU_N_valuesList + "\n";

static std::string selectNTRU_q =
std::string("Select NTRU_q parameter:\n") + NTRU_q_valuesList + "\n";

int CLI::validInput_int(){
    int input;
    while(!(std::cin >> input)) {
        std::cin.clear();                                                       // -Clear bad input flag
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');     // -Discard input
        std::cout << "Invalid input; please re-enter.\n";
    }
    return input;
}

int CLI::retreaveValidOption(std::string optionsString, bool (validOptionCriteria)(int)) { // -Will ask the user for input from a set of options
    int option;
    std::cout << optionsString;
    option = CLI::validInput_int();
    getchar();                                                                  // -Will take the "\n" left behind at the moment of press enter
    while(!validOptionCriteria(option)) {                                       // -Validating the option using the criteria specified by 'validOptionCriteria'
        std::cout << invalidInputMsg;                                           //  function. If not valid, it will reaped the process
        std::cout << optionsString;
        option = CLI::validInput_int();
        getchar();                                                              // -Will take the "\n" left behind at the moment of press enter
    }
    return option;
}

void CLI::retreaveValidFileName(const char message[], char name_dest[]) {
    CLI::getLine(message, name_dest);
    while(AnalyzeStringAsFileName::isValidFileName(name_dest)){
        std::cerr << "Not a valid file name. Try again." << std::endl;
        CLI::getLine(message, name_dest);
    }
}

void CLI::getLineAndRetreaveKeyFromFile(const char appendMsg[]) {
    char buffer[NAME_MAX_LEN];
    bool notValidNTRUkeyFile = true;
    CLI::getLine(std::string("Write the name/path of ") + appendMsg, buffer);
    while(notValidNTRUkeyFile) {
        notValidNTRUkeyFile = false;
        try {                                                                   // -Tries to build a key from file, if fails because the file does not exist, tries
            setEncryptionObjectFromFile(buffer);                                //  again
        }
        catch(std::runtime_error& exp) {
            notValidNTRUkeyFile = true;
            std::cerr << exp.what() << " Try again.\n";
            CLI::getLine(std::string("Write the name/path of ") + appendMsg, buffer);
        }
    }
}

void CLI::getFilesAndCipherAct(Options::Cipher_object op) {
    char buffer[UPPER_BOUND];
    char fileName[NAME_MAX_LEN];
    bool subStringExp = false;
    size_t bufferSize = UPPER_BOUND - 1;
    size_t inputStrSize = 0;
    int i, j;
    const char enc[] = "encrypt", dec[] = "decrypt";
    const char* opStr;
    if(op == Options::Cipher_object::Ciphering)   opStr = enc;
    if(op == Options::Cipher_object::Deciphering) opStr = dec;
    std::cout <<
    "Write the names/paths of the files you desire to " << opStr << "separated with spaces. Once done, press enter (input must not have spaces and should be\n"
    "at most " << bufferSize << " characters long. File names/paths must have at most "<< NAME_MAX_LEN << " characters):\n\n";
    std::cin.getline(buffer, (std::streamsize)bufferSize, '\n');
    inputStrSize = strlen(buffer);
    if(buffer[0] == 0) return;
    for(i = 0, j = 0; i < (int)inputStrSize && j < (int)inputStrSize; ) {           // -'for' ends with the break statement on its end (equivalent to a do-while)
        try{
            j = subStringDelimitedBySpacesOrQuotation(buffer, i, fileName);
        } catch(std::runtime_error& exp) {
            std::cout << exp.what();
            subStringExp = true;
        }
        i += j;
        if(!subStringExp) {
            try {
                cipherObjectOverFile(op, fileName);
            } catch(const std::runtime_error&) {
                cerrMessageBeforeReThrow("void CLI::getFilesAndCipherAct(Options::Cipher_object op)");
                throw;
            }
        }
        while(buffer[i] == ' ' || buffer[i] == '\t') if(buffer[i++] == 0) break;// -Terminating 'for'
    }
}

void CLI::retreaveTexAndEncrypt() {
    char*  consoleInput = NULL;
    char*  aux = NULL;
    char   fileName[NAME_MAX_LEN];
    size_t stringSize = 0, k = 0;
    std::ofstream file;
    NTRU::ZqPolynomial enc_msg;
    std::cout <<
    "\nWrite the string you want to encrypt. To process the string sent the value 'EOF', which you can do by:\n\n"
    "- Pressing twice the keys CTRL+Z for Windows.\n"
    "- Pressing twice the keys CTRL+D for Unix and Linux.\n\n";

    consoleInput = new char[UPPER_BOUND];
    while(std::cin.get(consoleInput[stringSize++])) {                           // -Input from CLI.
        if(k == BUFFER_SIZE) {                                                  // -Buffer size exceeded, taking more memory space
            aux = new char[stringSize];
            std::memcpy(aux, consoleInput, stringSize);
            delete[] consoleInput;
            consoleInput = new char[stringSize + BUFFER_SIZE];
            std::memcpy(consoleInput, aux, stringSize);
            delete[] aux; aux = NULL;
            k = 0;
        } else { k++; }
    }
    CLI::retreaveValidFileName("Write the name for the .txt file that will contain the encryption.\n", fileName);
    file.open(fileName);
    if(file.is_open()) {
        enc_msg = FileHandling::NTRUPolynomials::formatDataEncryption(consoleInput, stringSize);
        enc_msg.save(fileName, true);
        file.close();
    } else {
        std::cout << "Could not create output file.\n";
    }
    if(consoleInput != NULL) delete[] consoleInput;
    if(aux != NULL)          delete[] aux;
}

void CLI::retreaveKey(Options::Key_retreaving Kr, const char addedMsg[]) {      // -Retrieving key accordingly to the user's input
    switch(Kr) {
        case Options::RetrieveFromFile:
            CLI::getLineAndRetreaveKeyFromFile(addedMsg);
            break;
        case Options::CreateNew:
            setEncryptionObject();
        break;
    }
}

void runProgram(const Options::Cipher_object op) {
    char publicKeyName[NAME_MAX_LEN];
    char privatKeyName[NAME_MAX_LEN];
    bool validName = false;                                                     // -Flags is the given name for the encryption key given by the user is valid
    Options::Encryption_main_menu encMainMen = Options::Encryption_main_menu::saveKey;
    Options::Key_retreaving Kr = Options::Key_retreaving::RetrieveFromFile;            // -First action, setting the encryption key
    if(op == Options::Cipher_object::Deciphering) {                             // -If we want to decrypt, I am assuming we already have a private key
        CLI::retreaveKey(Kr, "private key.");
    } else {
        Kr = (Options::Key_retreaving)CLI::retreaveValidOption(keyRetreavingOptions, Options::isKeyRetreavingValue);
        CLI::retreaveKey(Kr, "public key.");
    }
    switch(op) {
        case Options::Cipher_object::Ciphering:
            if(Kr == Options::Key_retreaving::CreateNew)                        // -If the program generates the key, we need to save it
                encMainMen = (Options::Encryption_main_menu)CLI::retreaveValidOption(encryptionMainMenu, Options::isEncryptionMainMenuValue);
            if(Kr == Options::Key_retreaving::RetrieveFromFile)                 // -If the key is retrieved from file, there is no need to save it
                encMainMen = (Options::Encryption_main_menu)CLI::retreaveValidOption(encryptionMainMenuNoSaveKey, Options::isEncryptionMainMenuValueNoSaveKey);
            switch(encMainMen) {
                case Options::Encryption_main_menu::encryptFiles:
                    try {
                        CLI::getFilesAndCipherAct(Options::Cipher_object::Ciphering);// -Encrypting files passed in a line from CLI
                    } catch(std::runtime_error& exp) {
                        cerrMessageBeforeReThrow("void runProgram(const Options::Cipher_object op)");
                        std::cerr << exp.what();
                        return;
                    }
                    break;
                case Options::Encryption_main_menu::encryptTextFromCLI:
                    CLI::retreaveTexAndEncrypt();                               // -Retrieve text from CLI, encrypts and saves the result in a text file
                    break;
                case Options::Encryption_main_menu::saveKey:
                    break;
            }
            if(Kr == Options::Key_retreaving::CreateNew) {
                CLI::getLine("Assign a name to the public key file.", publicKeyName);
                validName = AnalyzeStringAsFileName::isValidFileName(publicKeyName);
                while(!validName) {                                             // -Validating the name for the key files
                    CLI::getLine("Try again. Assign a name to public key file.", publicKeyName);
                    validName = AnalyzeStringAsFileName::isValidFileName(publicKeyName);
                }
                CLI::getLine("Assign a name to the private key file.", privatKeyName);
                validName = AnalyzeStringAsFileName::isValidFileName(publicKeyName);
                while(!validName) {                                             // -Validating the name for the key files
                    CLI::getLine("Try again. Assign a name to the private key file.", privatKeyName);
                    validName = AnalyzeStringAsFileName::isValidFileName(publicKeyName);
                }
                NTRUencryption.saveKeys(publicKeyName, privatKeyName);
            }
            break;
        case Options::Cipher_object::Deciphering:
            if(!NTRUencryption.validPrivateKeyAvailable()) {
                cerrMessageBeforeThrow("void runProgram(const Options::Cipher_object op)", "Not valid private key found.");
                throw std::runtime_error("Private key not found.");
            }
            try {
                CLI::getFilesAndCipherAct(Options::Cipher_object::Deciphering);
            } catch(std::runtime_error& exp) {
                cerrMessageBeforeReThrow("void runProgram(const Options::Cipher_object op)");
                std::cerr << exp.what();
            }
            break;
    }
}

void encryptFile(const char fileName[]) {
    try {
        cipherObjectOverFile(Options::Cipher_object::Ciphering, fileName);
    } catch(const std::runtime_error& exp) {
        cerrMessageBeforeReThrow("void encryptFile(const char fileName[])");
        std::cerr << exp.what() << std::endl;
    }
}

void decryptFile(const char fileName[]) {
    try {
        cipherObjectOverFile(Options::Cipher_object::Deciphering, fileName);
    } catch(const std::runtime_error& exp) {
        cerrMessageBeforeReThrow("void decryptFile(const char fileName[])");
        std::cerr << exp.what() << std::endl;
    }
}

void runEncryptionProgram() {
    runProgram(Options::Cipher_object::Ciphering);
}

void runDecryptionProgram() {
    try{
        runProgram(Options::Cipher_object::Deciphering);
    } catch(const std::runtime_error& exp){
        cerrMessageBeforeReThrow("void runDecryptionProgram()");
        throw;
    }
}
