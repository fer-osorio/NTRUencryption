#include<fstream>
#include<cstring>
#include<exception>
#include"Settings.hpp"

#define BYTES_FOR_FILE_SIZE 2                                                   // -Size of the information we will append to the data we intend to encrypt. Now
#define FILE_TYPE_ID_SIZE   3                                                   //  it correspond to 3 bytes for the type of file, and tow bytes for the file size
#define BUFFER_SIZE 1025
#define UPPER_BOUND 4097                                                        // -Intended for indices that run through strings. This selection has a reason; it
                                                                                //  is the upper limit for strings representing directories path
enum    FileExtensions              { bin,   txt, none, unrecognized };
static const char* supportedExt[] = {"bin", "txt"};                             // -We will use this to register the type of file we are encrypting
static FileExtensions getExtension(const char fileName[]) {
    if(fileName == NULL) {
        std::cerr << "In file Source/NTRUencryption.cpp, function static FileExtensions getExtension(const char fileName[]). filename == NULL...\n";
        return unrecognized;
    }
    int i = -1;
    while(fileName[++i] != 0) {}                                                // -Looking for end of string
    while(fileName[i] != '.' && i > 0) {i--;}                                   // -Looking for last point

    if(i >= 0) {
        i++;                                                                    // -Moving one place ahead the point
        if(strcmp(&fileName[i], "bin") == 0) return bin;
        if(strcmp(&fileName[i], "txt") == 0) return txt;
        return unrecognized;
    }
    return none;                                                                // -Could not find the point
}
static FileExtensions strToFileExtension(const char str[]) {
    if(str == NULL || str[0] == 0) return none;
    if(strcmp(str, "bin") == 0) return bin;
    if(strcmp(str, "txt") == 0) return txt;
    return unrecognized;
}

const NTRU_N NTRU_N_values[]    = {    _509_,     _677_ ,    _701_,     _821_,     _1087_,     _1171_,     _1499_};// -All the possible values for the N
const char NTRU_N_valuesList[]  = "(0) _509_, (1) _677_, (2) _701_, (3) _821_, (4) _1087_, (5) _1171_, (6) _1499_";// -Useful for CLI
const size_t NTRU_N_amount      = sizeof(NTRU_N_values)/sizeof(NTRU_N_values[0]);

const NTRU_q NTRU_q_values[]    = {    _2048_,     _4096_ ,     _8192_};        // -All the possible values for the q
const char NTRU_q_valuesList[]  = "(0) _2048_, (1) _4096_ , (2) _8192_";        // -Useful for CLI
const size_t NTRU_q_amount      = sizeof(NTRU_q_values)/sizeof(NTRU_q_values[0]);

const NTRU_p NTRU_p_values[]    = {_3_};

static void cerrMessageBeforeThrow(const char callerFunction[], const char message[]) {
    std::cerr << "In file Source/NTRUencryption.cpp, function " << callerFunction << ": " << message << '\n';
}

static void cerrMessageBeforeReThrow(const char callerFunction[], const char message[] = "") {
    std::cerr << "Called from: File Source/NTRUencryption.cpp, function " << callerFunction << " ..."<< message << '\n';
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

struct StringFileNameAnalize {
    private:                                                                    // -Attributes
    const char* str = NULL;
    size_t      size = 0;
    unsigned    currentIndex = 0;
    /*bool        allowSpaces  = false;
    bool        beginsSingleQuote = false;
    bool        beginsDoubleQuote = false;*/
    private:                                                                    // -Functions for syntax analysis
    void cerrSyntaxErrMsg(const char[]);
    bool Sld();                                                                 // -The returned bool flags the founding of zero byte or the characters '\'' or '"')
    bool FN ();

    StringFileNameAnalize(const char str_[]);

    public:
    static bool isValidFileName(const char str[]);
};

void StringFileNameAnalize::cerrSyntaxErrMsg(const char msg[]) {
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

bool StringFileNameAnalize::Sld() {
    if(!isLetter(this->str[this->currentIndex])) {                              // -This ensures we have a letter at the beginning of the string
        if(isDigit(this->str[this->currentIndex])) {
            this->cerrSyntaxErrMsg("File name can not start with a digit.");
            return false;
        }
        this->cerrSyntaxErrMsg("Expected a character from English alphabet.");
        return false;
    }
    unsigned i = 0;
    //if(this->allowSpaces){                                                      // -If file name starts with single/double quote or the constructor sets as so
        for(i = ++this->currentIndex; isLetterOrDigit(this->str[i]) || this->str[i] == ' '; i++) {} // -Running trough letters, digits and spaces
    //} else {
        //for(i = ++this->currentIndex; isLetterOrDigit(this->str[i]); i++) {}    // -Running trough letters and digits
    //}
    this->currentIndex = i;

    if(this->str[this->currentIndex] == 0) {
        /*if(this->beginsSingleQuote) {
            this->cerrSyntaxErrMsg("String started with single quote, then it should finish with single quotes.");
            return false;
        }
        if(this->beginsDoubleQuote) {
            this->cerrSyntaxErrMsg("String started with double quote, then it should finish with double quotes.");
            return false;
        }*/
        if(this->str[this->currentIndex-1] == ' ') {
            this->cerrSyntaxErrMsg("File Name/Path can not finish with spaces.");
            return false;
        }
        return true;
    }
    /*if(this->beginsSingleQuote && this->str[this->currentIndex] == '\'') {      // -Starting with single quote, finishing with single quote
        if(this->str[this->currentIndex-1] == ' ') {
            this->cerrSyntaxErrMsg("File Name/Path can not finish with spaces.");
            return false;
        }
        return true;
    }*/
    /*if(this->beginsDoubleQuote && this->str[this->currentIndex] == '"' ) {      // -Starting with double quote, finishing with double quote
        if(this->str[this->currentIndex-1] == ' ') {
            this->cerrSyntaxErrMsg("File Name/Path can not finish with spaces.");
            return false;
        }
        return true;
    }*/
    /*if(this->str[this->currentIndex] == ' ') {                                  // -At this point, a space is a proper ending for the input string
        this->cerrSyntaxErrMsg("To allow spaces, put the string inside single or double quotes");
        return false;
    }*/
    return this->FN();
}

bool StringFileNameAnalize::FN() {
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

StringFileNameAnalize::StringFileNameAnalize(const char _str_[]): str(_str_) {
    if(this->str != NULL) while(this->str[this->size] != 0) this->size++;
}

bool StringFileNameAnalize::isValidFileName(const char str[]) {                 // -Validating string as a file name
    StringFileNameAnalize s(str);
    return s.FN();
}

TXT::TXT(const char fname[]) {                                                  // -Building from file.
    std::ifstream file;
    int i = 0, nameMaxLen = NAME_MAX_LEN - 1;
    for( ;fname[i] != 0; i++) {                                                 // -Assigning file name to name object
        name[i] = fname[i];
        if(i == nameMaxLen) {                                                   // -If maximum length reached, truncate the file name.
            name[i] = 0;                                                        // -Indicate the end of the string
            break;                                                              // -End for loop
        }
    }
    file.open(fname);
    if(file.is_open()) {
        file.seekg(0, std::ios::end);                                           // -Look for the end of the file
        std::streampos fileSize = file.tellg();
        this->size = (size_t)fileSize;
        file.seekg(0, std::ios::beg);                                           // -Go to the beginning
        this->content = new char[fileSize];                                     // -Allocate memory and copy file content
        file.read(this->content, fileSize);                                     // ...
        file.close();
    } else {
        cerrMessageBeforeThrow("TXT::TXT(const char fname[])", "Could not open file");
        throw std::runtime_error("File opening failed");
    }
}

TXT::TXT(const TXT& t): size(t.size){                                           // -Copy constructor
    strcpy(this->name, t.name);
    this->content = new char[t.size];
    for(unsigned i = 0; i < t.size; i++) this->content[i] = t.content[i];
}

TXT::~TXT(){
    if(this->content != NULL) delete[] this->content;
    this->content = NULL;
}

TXT& TXT::operator=(const TXT &t) {
    if(this != &t) {                                                            // -Guarding against self assignment
        if(this->content != NULL) delete[] this->content;                       // -Deleting old content
        strcpy(this->name, t.name);
        this->size = t.size;
        this->content = new char[t.size];
        for(unsigned i = 0; i < t.size; i++) this->content[i] = t.content[i];
    }
    return *this;
}

void TXT::overwrite(const char data[], size_t dataSize){                        // -Rewrites TXT files
	if(this->content != NULL) delete[] this->content;                           // -Deleting old content
    this->size = dataSize;
    this->content = new char[this->size];
    for(unsigned i = 0; i < dataSize; i++) this->content[i] = data[i];
}

void TXT::save(const char fname[]) const{                                       // -Saving the content in a .txt file
    std::ofstream file;
    char _fname[NAME_MAX_LEN];
    if(fname == NULL) {                                                         // -If a file name is not provided, then we assign the name of the object
        strcpy(_fname, this->name);
        file.open(_fname);
    } else {
        file.open(fname);
    }
    if(file.is_open()) {
        file.write(this->content, (std::streamsize)this->size);
        file.close();
    } else {
        cerrMessageBeforeThrow("void TXT::save(const char fname[]) const)", "Could not save file");
        throw std::runtime_error("File writing failed");
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
	static bool isEncryptionMainMenu(int t) {
		return
		t == encryptFiles ||
		t == encryptTextFromCLI ||
		t == saveKey;
	}
};

union ushort_to_char {                                                          // -Casting from short to an array of two chars
    unsigned short ushort;
    char         chars[2];
};

namespace CLI {                                                                 // -Functions that interact with user through CIL
    static void getLine(const std::string message, char destination[]);         // -Get input string from console. Finish with a line ending '\n'
    static int  retreaveValidOption(std::string optionsString, bool (validOptionCriteria)(int)); // -Force the user to select a valid option
    static void getLineAndRetreaveKeyFromFile(const char appendMsg[] = "key");  // -Retrieve line, interprets it as a file and tries to build key from that file
    static void retreaveKey(Options::Key_retreaving, const char addedMsg[]=""); // -Retrieve key accordingly to its argument
    static void getFilesAndCipherAct(Options::Cipher_object);                   // -Get line, interpret it as a sequence of files and try to encrypt/decrypt them
    static void retreaveTexAndEncrypt();                                        // -Gets input from CIL, encrypt and saves it int a text file
};

static int  subStringDelimitedBySpacesOrQuotation(const char* source, const int start, char* destination);
static void setEncryptionObject(NTRU_N _N_, NTRU_q _q_);
static void encryptFileBinMode(const char fname[]);                             // -Opens file in binary mode and encrypt
static void encryptTextFile(const char fname[]);                                // -Opens file as text and encrypt
static void decryptFileBinMode(const char fname[]);
static void cipherObjectOverFile(const Options::Cipher_object, const char fname[]);// -Encrypts or decrypts file of second argument accordingly to the first argument


/************************************************************************* Attributes *****************************************************************************/

static NTRU::Encryption NTRUencryption;                                         // -Declaring encryption object. Should not be used jet

/************************************************************************* Attributes *****************************************************************************/


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

void setEncryptionObject(NTRU_N _N_, NTRU_q _q_) {
    try { NTRUencryption = NTRU::Encryption(_N_, _q_); }
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

static const int encrypFileHeaderSize = BYTES_FOR_FILE_SIZE + FILE_TYPE_ID_SIZE;

void encryptFileBinMode(const char fname[]) {
    const char thisFunc[] = "static void cipherbinFile(const char fname[])";
    std::ifstream   file;
    std::streampos  fileSize;
    const size_t    plainTextMaxSize = NTRUencryption.plainTextMaxSizeInBytes();
    unsigned short  fileSizePlusHeaderSize;
    char*           fileInput = NULL;
    NTRU::ZqPolynomial enc_msg;
    file.open(fname, std::ios::binary | std::ios::ate);                         // -Open the ifile in binary mode and move to the end
    if (!file.is_open()) {
        cerrMessageBeforeThrow(thisFunc, "Error opening file for encryption");
        throw std::runtime_error("File opening failed");
    }
    fileSize = file.tellg();                                            // -Get the file size
    if((size_t)fileSize > plainTextMaxSize - encrypFileHeaderSize) {
        file.close();
        cerrMessageBeforeThrow(thisFunc, "");
        std::cerr << "File size exceeds the limit size (" << plainTextMaxSize - encrypFileHeaderSize << " bytes)";
        throw std::runtime_error("Upper bound for file size exceeded");
    }
    file.seekg(0, std::ios::beg);                                   // -Move back to the beginning
    fileSizePlusHeaderSize = (unsigned)fileSize + (unsigned)encrypFileHeaderSize;   // -This will allow us two write the size in the input array for
    fileInput = new char[fileSizePlusHeaderSize];                   //  encryption.
    memcpy(fileInput, supportedExt[bin], FILE_TYPE_ID_SIZE);              // -Assigning bin file type
    memcpy((char*)&fileInput[FILE_TYPE_ID_SIZE], (char*)&fileSize, BYTES_FOR_FILE_SIZE); // -Writing file size
    file.read((char*)&fileInput[encrypFileHeaderSize], fileSize);
    file.close();
    enc_msg = NTRUencryption.encrypt(fileInput, fileSizePlusHeaderSize);         // -Encrypting the bytes from the file and its size
    delete[] fileInput;
    try {
        enc_msg.save();
    }
    catch(const char* str) {
        cerrMessageBeforeReThrow(thisFunc);
        throw;
    }
}

void encryptTextFile(const char fname[]) {
    const char      thisFunc[] = "void encryptTextFile(const char fname[])";
    char*           fileInput = NULL;
    char*           enc_text  = NULL;
    TXT             text;
    const size_t    cipherTextSize   = NTRUencryption.cipherTextSizeInBytes();
    const size_t    plainTextMaxSize = NTRUencryption.plainTextMaxSizeInBytes();
    unsigned short  fileSizePlusHeaderSize = text.size + (unsigned)encrypFileHeaderSize;
    NTRU::ZqPolynomial enc_msg;
    try { text = TXT(fname); }
    catch(const char* str) {
        cerrMessageBeforeReThrow(thisFunc);
        throw;
    }
    if(text.size > plainTextMaxSize - encrypFileHeaderSize) {
        cerrMessageBeforeThrow(thisFunc, "");
        std::cerr << "File size exceeds the limit size (" << plainTextMaxSize - encrypFileHeaderSize << " bytes)";
        throw std::runtime_error("Upper bound for file size exceeded");
    }
    fileSizePlusHeaderSize = text.size + (unsigned)encrypFileHeaderSize;// -This will allow us two write the size in the input array for encryption.
    fileInput = new char[fileSizePlusHeaderSize];
    memcpy(fileInput, supportedExt[txt], FILE_TYPE_ID_SIZE);              // -Assigning txt file type
    memcpy((char*)&fileInput[FILE_TYPE_ID_SIZE], (char*)&text.size, BYTES_FOR_FILE_SIZE);
    memcpy((char*)&fileInput[encrypFileHeaderSize], text.content, text.size);
    enc_msg = NTRUencryption.encrypt(fileInput, fileSizePlusHeaderSize, true);
    delete[] fileInput;
    enc_msg.println("\nEncrypted message");
    enc_text = new char[(unsigned)cipherTextSize];
    enc_msg.toBytes(enc_text);
    text.overwrite(enc_text, (unsigned)cipherTextSize);
    try{ text.save(); }
    catch(const char* str) { std::cerr << str; }
    try { enc_msg.save("encryptedMessage.ntruq"); }
    catch(const char* str) {
        cerrMessageBeforeReThrow(thisFunc);
        throw;
    }
    if(enc_text != NULL) { delete[] enc_text; enc_text = NULL; }
}

void decryptFileBinMode(const char fname[]) {
    const char thisFunc[] = "void decryptFileBinMode(const char fname[])";
    char*           buff     = NULL;
    const char      ntruq[]  = "NTRUq";
    char            ftype[FILE_TYPE_ID_SIZE + 1];
    const size_t    ntruqlen = strlen(ntruq);
    size_t          lenInBytes = 0;
    ushort_to_char  fileSize_u= {0};                                              // -So we can convert two chars into a short
    NTRU_N N = _509_;
    NTRU_q q = _2048_;
    FileExtensions  ext;
    std::ifstream   ifile;
    std::ofstream   ofile;

    buff = new char[ntruqlen + 1];
    ifile.open(fname, std::ios::binary);                                        // -Open the ifile in binary mode and move to the end
    if (!ifile.is_open()) {
        cerrMessageBeforeThrow(thisFunc, "Error opening file for encryption");
        throw std::runtime_error("File opening failed");
    }
    ifile.read(buff, ntruqlen);
    buff[ntruqlen] = 0;
    if(strcmp(buff, ntruq) == 0) {
        ifile.read((char*)&N, 2);                                       // -A short int for the degree of the polynomial
        ifile.read((char*)&q, 2);                                       // -A short int for the q value
        lenInBytes = size_t(N*NTRU::ZqPolynomial::log2(q)/8 + 1);
        delete[] buff;
        buff = new char[lenInBytes];
        ifile.read(buff, (std::streamsize)lenInBytes);
        ifile.close();
        NTRU::ZpPolynomial msg = NTRUencryption.decrypt(buff, lenInBytes);
        delete[] buff;
        lenInBytes = NTRUencryption.plainTextMaxSizeInBytes();
        buff = new char[lenInBytes];
        msg.toBytes(buff);
        memcpy(ftype, buff, FILE_TYPE_ID_SIZE);
        ftype[FILE_TYPE_ID_SIZE] = 0;
        memcpy(fileSize_u.chars, (char*)&buff[FILE_TYPE_ID_SIZE], BYTES_FOR_FILE_SIZE);
        ext = strToFileExtension(ftype);
        switch(ext) {
            case bin:
                std::cout << "DecryptedMessage:\n" << (char*)&buff[encrypFileHeaderSize] << '\n';
                ofile.open("DecryptedMessage.bin", std::ios::binary);
                if(ofile.is_open()) {
                    ofile.write((char*)&buff[encrypFileHeaderSize], fileSize_u.ushort);
                } else {
                    if(buff != NULL) delete[] buff;
                    cerrMessageBeforeThrow(thisFunc, "Could not write decrypted file");
                    throw std::runtime_error("File writing failed");
                }
                break;
            case txt:
                ofile.open("DecryptedMessage.txt");
                if(ofile.is_open()) {
                    ofile.write((char*)&buff[encrypFileHeaderSize], fileSize_u.ushort);
                } else {
                    if(buff != NULL) delete[] buff;
                    cerrMessageBeforeThrow(thisFunc, "Could not write decrypted file");
                    throw std::runtime_error("File writing failed");
                }
                break;
            case none:
                break;
            case unrecognized:
                break;
        }
    } else {
        if(buff != NULL) delete[] buff;
        cerrMessageBeforeThrow(thisFunc, "Not a ntruq file");
        throw std::runtime_error("Not valid input file");
    }
}

void cipherObjectOverFile(const Options::Cipher_object act, const char fname[]) {
    const char  thisFunc[] = "void cipherObjectOverFile(const Options::Cipher_object act,  const char fname[])";
    FileExtensions ext = getExtension(fname);                                                  // -Recognizing extension.
    switch(ext) {
        case bin:
            switch(act) {
                case Options::Cipher_object::Ciphering:
                    try{
                        encryptFileBinMode(fname);
                    }catch(const std::runtime_error&) {
                        cerrMessageBeforeReThrow(thisFunc);
                        throw;
                    }
                    break;
                case Options::Cipher_object::Deciphering:
                    try{
                        decryptFileBinMode(fname);
                    }catch(const std::runtime_error&){
                        cerrMessageBeforeReThrow(thisFunc);
                        throw;
                    }
                    break;
            }
            break;
        case txt:                                                               // -Notice how a text file just can be encrypted
            switch(act) {
                case Options::Cipher_object::Ciphering:
                    try{
                        encryptTextFile(fname);
                    }catch(const std::runtime_error&) {
                        cerrMessageBeforeReThrow(thisFunc);
                        throw;
                    }
                    break;
                case Options::Cipher_object::Deciphering:
                    std::cerr << "Decryption of text files not currently supported.\n";
                    break;
            }
            break;
        case none:
        case unrecognized:
            std::cout << "Not a .txt or .bin extension found. Proceeding to treat the file as a binary file.\n";
            switch(act) {
                case Options::Cipher_object::Ciphering:
                    try{
                        encryptFileBinMode(fname);
                    }catch(const std::runtime_error&) {
                        cerrMessageBeforeReThrow(thisFunc);
                        throw;
                    }
                    break;
                case Options::Cipher_object::Deciphering:
                    try{
                        decryptFileBinMode(fname);
                    }catch(const std::runtime_error&){
                        cerrMessageBeforeReThrow(thisFunc);
                        throw;
                    }
                    break;
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

static const char keyRetreavingOptions[] =
"Would you like to:\n"
"(0) Retrieve encryption keys from file.\n"
"(1) Let this program generate the encryption keys.\n";

static std::string selectNTRU_N =
std::string("Select NTRU_N parameter:\n") + NTRU_N_valuesList + "\n";

static std::string selectNTRU_q =
std::string("Select NTRU_q parameter:\n") + NTRU_q_valuesList + "\n";

int CLI::retreaveValidOption(std::string optionsString, bool (validOptionCriteria)(int)) { // -Will ask the user for input from a set of options
    int option;
    std::cout << optionsString;
    std::cin >> option;
    getchar();                                                                  // -Will take the "\n" left behind at the moment of press enter
    while(!validOptionCriteria(option)) {                                       // -Validating the option using the criteria specified by 'validOptionCriteria'
        std::cout << invalidInputMsg;                                           //  function. If not valid, it will reaped the process
        std::cout << optionsString;
        std::cin >> option;
        getchar();                                                              // -Will take the "\n" left behind at the moment of press enter
    }
    return option;
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
    for(i = 0, j = 0; i < (int)inputStrSize && j < (int)inputStrSize; i += j) {           // -'for' ends with the break statement on its end (equivalent to a do-while)
        try{
            j = subStringDelimitedBySpacesOrQuotation(&buffer[i], i, fileName);
        } catch(std::runtime_error& exp) {
            std::cout << exp.what();
            subStringExp = true;
        }
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
            delete[] aux;
            k = 0;
        } else { k++; }
    }
    CLI::getLine("Write the name for the .txt file that will contain the encryption.\n", fileName);
    file.open(fileName);
    if(file.is_open()) {
        enc_msg = NTRUencryption.encrypt(consoleInput, stringSize);
        file.write(consoleInput, (std::streamsize)stringSize);
        file.close();
    } else {
        std::cout << "Could not create output file.\n";
    }
    if(consoleInput != NULL) delete[] consoleInput;
    if(aux != NULL)          delete[] aux;
}

static bool NTRU_N_validOpt(int opt) { return opt >= 0 && opt < (int)NTRU_N_amount;}
static bool NTRU_q_validOpt(int opt) { return opt >= 0 && opt < (int)NTRU_q_amount;}

void CLI::retreaveKey(Options::Key_retreaving Kr, const char addedMsg[]) {      // -Retrieving key accordingly to the user's input
    int opt_N, opt_q;
    switch(Kr) {
        case Options::RetrieveFromFile:
            CLI::getLineAndRetreaveKeyFromFile(addedMsg);
            break;
        case Options::CreateNew:
            opt_N = CLI::retreaveValidOption(selectNTRU_N, NTRU_N_validOpt);
            opt_q = CLI::retreaveValidOption(selectNTRU_q, NTRU_q_validOpt);
            setEncryptionObject(NTRU_N_values[opt_N], NTRU_q_values[opt_q]);
        break;
    }
}

void runProgram(const Options::Cipher_object op) {
    char publicKeyName[NAME_MAX_LEN];
    char privatKeyName[NAME_MAX_LEN];
    bool validName = false;                                                     // -Flags is the given name for the encryption key given by the user is valid
    Options::Encryption_main_menu encMainMen;
    Options::Key_retreaving  Kr;

    Kr = (Options::Key_retreaving)CLI::retreaveValidOption(keyRetreavingOptions, Options::isKeyRetreavingValue);    // -Before encryption, the key must obtain
    if(op == Options::Cipher_object::Deciphering)                                                                   //  the encryption key
        CLI::retreaveKey(Kr, "private key.");
    else
        CLI::retreaveKey(Kr, "public key.");

    switch(op) {
        case Options::Cipher_object::Ciphering:
            encMainMen = (Options::Encryption_main_menu)CLI::retreaveValidOption(encryptionMainMenu, Options::isEncryptionMainMenu);// -Asking what we will encrypt
            switch(encMainMen) {
                case Options::Encryption_main_menu::encryptFiles:
                    try {
                        CLI::getFilesAndCipherAct(Options::Cipher_object::Ciphering);// -Encrypting files passed in a line from CLI
                    } catch(std::runtime_error& exp) {
                        cerrMessageBeforeReThrow("void runProgram(const Options::Cipher_object op)");
                        std::cerr << exp.what();
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
                validName = StringFileNameAnalize::isValidFileName(publicKeyName);
                while(!validName) {                                                 // -Validating the name for the key files
                    CLI::getLine("Try again. Assign a name to public key file.", publicKeyName);
                    validName = StringFileNameAnalize::isValidFileName(publicKeyName);
                }
                CLI::getLine("Assign a name to the private key file.", privatKeyName);
                validName = StringFileNameAnalize::isValidFileName(publicKeyName);
                while(!validName) {                                                 // -Validating the name for the key files
                    CLI::getLine("Try again. Assign a name to the private key file.", privatKeyName);
                    validName = StringFileNameAnalize::isValidFileName(publicKeyName);
                }
                NTRUencryption.saveKeys(publicKeyName, privatKeyName);
            }
            break;
        case Options::Cipher_object::Deciphering:
            CLI::getFilesAndCipherAct(Options::Cipher_object::Deciphering);
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
        cerrMessageBeforeReThrow("void encryptFile(const char fileName[])");
        std::cerr << exp.what() << std::endl;
    }
}

void runEncryptionProgram() {
    runProgram(Options::Cipher_object::Ciphering);
}

void runDecryptionProgram() {
    if(!NTRUencryption.validPrivateKeyAvailable()) {
        cerrMessageBeforeThrow("void runDecryptionProgram()", "Not valid private key found.");
        throw std::runtime_error("Private key not found.");
    }
    runProgram(Options::Cipher_object::Deciphering);
}
