#include<fstream>
#include<cstring>
#include"Settings.hpp"

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

static const char* supportedExt[] = {"bin", "txt"};                             // -We will use this to register the type of file we are encrypting

enum CharType {letter, digit, dot, underscore, hyphen, slash, space, singleQuote ,doubleQuote, notAllowed, zero};

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
    private: // Attributes
    const char* str = NULL;
    size_t      size = 0;
    unsigned    currentIndex = 0;
    bool        allowSpaces  = false;
    bool        beginsSingleQuote = false;
    bool        beginsDoubleQuote = false;
    private: // Functions for syntax analysis
    void cerrSyntaxErrMsg(const char[]);
    bool Sld();                                                                 // -The returned bool flags the founding of zero byte or the characters '\'' or '"')
    bool FN ();

    StringFileNameAnalize(const char str[], const bool allowSpaces_ = false);

    public:
    static bool isValidFileName(const char str[]);
};

void StringFileNameAnalize::cerrSyntaxErrMsg(const char msg[]) {
    const char SinErr[] = "Syntax Error: ";
    size_t sz = strlen(SinErr) + this->currentIndex;
    unsigned i;
    std::cerr << SinErr;
    for(i = 0; i <= this->currentIndex; i++) std::cerr << this->str[i];
    std::cerr << '\n';
    for(i = 1; i < sz; i++) std::cerr << ' ';
    std::cerr << "^~~~ " << msg << std::endl;
}

bool StringFileNameAnalize::Sld() {
    if(!isLetter(this->str[this->currentIndex])) {                              // -This ensures we have a letter at the beginning of the string
        if(isDigit(this->str[this->currentIndex])) {
            StringFileNameAnalize::cerrSyntaxErrMsg("File name can not start with a digit.");
            return false;
        }
        StringFileNameAnalize::cerrSyntaxErrMsg("Expected a character from English alphabet.");
        return false;
    }
    int i = 0;
    if(this->allowSpaces){                                                      // -If file name starts with single/double quote or the constructor sets as so
        for(i = ++this->currentIndex; isLetterOrDigit(this->str[i]) || this->str[i] == ' '; i++) {} // -Running trough letters, digits and spaces
    } else
        for(i = ++this->currentIndex; isLetterOrDigit(this->str[i]); i++) {}    // -Running trough letters and digits

    this->currentIndex = i;

    if(this->str[this->currentIndex] == 0) {
        if(this->beginsSingleQuote) {
            StringFileNameAnalize::cerrSyntaxErrMsg("String started with single quote, then it should finish with single quotes.");
            return false;
        }
        if(this->beginsDoubleQuote) {
            StringFileNameAnalize::cerrSyntaxErrMsg("String started with double quote, then it should finish with double quotes.");
            return false;
        }
        return true;
    }
    if(this->beginsSingleQuote && this->str[this->currentIndex] == '\'') {      // -Starting with single quote, finishing with single quote
        if(this->str[this->currentIndex-1] == ' ') {
            StringFileNameAnalize::cerrSyntaxErrMsg("File Name/Path can not finish with spaces.");
            return false;
        }
        return true;
    }
    if(this->beginsDoubleQuote && this->str[this->currentIndex] == '"' ) {      // -Starting with double quote, finishing with double quote
        if(this->str[this->currentIndex-1] == ' ') {
            StringFileNameAnalize::cerrSyntaxErrMsg("File Name/Path can not finish with spaces.");
            return false;
        }
        return true;
    }
    if(this->str[this->currentIndex] == ' ') return true;                       // -At this point, a space is a proper ending for the input string
    return false;
}

bool StringFileNameAnalize::FN() {
    CharType ct = characterType(this->str[this->currentIndex]);
    switch(ct) {
        case slash:                                                             // -Allowing slash for file paths
            this->currentIndex++;
            return StringFileNameAnalize::Sld();
        case letter:
        case underscore:
        case hyphen:
            return StringFileNameAnalize::Sld();                                // -Read (always starting with a letter) letters, digits and (if allowed) spaces;
            break;                                                              //  when a proper ending character is found (0,'\'','"',' ') then return
        case dot:                                                               // -Cases for dot
            if(this->str[++this->currentIndex] == '.') {                        // -The string "../" is allowed as a sub-string so we can use relative paths
                if(this->str[++this->currentIndex] == '/') {
                    this->currentIndex++;
                    return StringFileNameAnalize::FN();
                } else {
                    StringFileNameAnalize::cerrSyntaxErrMsg("Syntax Error: Expected '/' character.");
                    return false;
                }
            } else {
                return StringFileNameAnalize::Sld();                            // -This lines can be interpreted as: Read (always starting with a letter) letters,
            }                                                                   //  digits and (if allowed) spaces; when a proper ending character is found
        break;
        case digit:
            StringFileNameAnalize::cerrSyntaxErrMsg("File name can not start with a digit.");
            return false;
        case space:
            StringFileNameAnalize::cerrSyntaxErrMsg("Not Expecting a space here.");
            return false;
        case singleQuote:
            StringFileNameAnalize::cerrSyntaxErrMsg("Not Expecting a single quote here.");
            return false;
        case doubleQuote:
            StringFileNameAnalize::cerrSyntaxErrMsg("Not expecting a double quote here.");
            return false;
        case notAllowed:
            StringFileNameAnalize::cerrSyntaxErrMsg("Unexpected character/symbol.");
            return false;
        case zero:
            StringFileNameAnalize::cerrSyntaxErrMsg("Unexpected end of string.");
            return false;
    }
}

StringFileNameAnalize::StringFileNameAnalize(const char str[], const bool allowSpaces_): allowSpaces(allowSpaces_) {
    if(str != NULL) while(str[this->size] != 0) this->size++;
    if((this->beginsSingleQuote = (str[0] == '\''))) this->allowSpaces = true;  // -If starting with single quote, allow spaces is overwritten
    if((this->beginsDoubleQuote = (str[0] == '"' ))) this->allowSpaces = true;  // -If starting with double quote, allow spaces is overwritten
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
        this->size = fileSize;
        file.seekg(0, std::ios::beg);                                           // -Go to the beginning
        this->content = new char[fileSize];                                     // -Allocate memory and copy file content
        file.read(this->content, fileSize);                                     // ...
        file.close();
    } else {
        char errmsg[] = "\nIn TXT.cpp file, TXT::TXT(const char* fname): Could not open file ";
        std::cerr << errmsg << fname << '\n';
        throw errmsg;
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
        file.write(this->content, this->size);
        file.close();
    } else {
        throw "File could not be written.";
    }
}
