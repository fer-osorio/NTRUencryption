/* main.cpp
 *
 * Copyright 2024 Fernando Osorio
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include<fstream>
#include<cstring>
#include"NTRUencryption.hpp"

#define NAME_MAX_LEN 256

void copyStr(const char source[], char dest[]) {                                // Supposing source is a formatted string and dest has enough space
    for(int i = 0; source[i] != 0; i++) dest[i] = source[i];
}

struct TXT {
	char name[NAME_MAX_LEN] = {0};
    unsigned size = 0;
    char* content = NULL; // text file content.

    TXT(): name() {}                                                            // -Just for type declaration.

    TXT(const char* fname) {                                                    // -Building from file.
        std::ifstream file;
        int i = 0, nameMaxLen = NAME_MAX_LEN - 1;
        for( ;fname[i] != 0; i++) {                                             // -Assigning file name to name object
            name[i] = fname[i];
            if(i == nameMaxLen) {                                               // -If maximum length reached, truncate the file name.
                name[i] = 0;                                                    // -Indicate the end of the string
                break;                                                          // -End for loop
            }
        }
        file.open(fname);
        if(file.is_open()) {
            file.seekg(0, std::ios::end);                                       // -Look for the end of the file
            std::streampos fileSize = file.tellg();
            this->size = fileSize;
            file.seekg(0, std::ios::beg);                                       // -Go to the beginning
            this->content = new char[fileSize];                                 // -Allocate memory and copy file content
            file.read(this->content, fileSize);                                 // ...
            file.close();
        } else {
            char errmsg[] = "\nIn TXT.cpp file, TXT::TXT(const char* fname): "
                            "Could not open file ";
            std::cerr << errmsg << fname << '\n';
            throw errmsg;
        }
    }
    TXT(const TXT& t): size(t.size){                                            // -Copy constructor
        copyStr(t.name, this->name);
        this->content = new char[t.size];
        for(unsigned i = 0; i < t.size; i++) this->content[i] = t.content[i];
    }

    ~TXT() {
        if(this->content != NULL) delete[] this->content;
        this->content = NULL;
    }

	TXT& operator = (const TXT& t) {
	     if(this != &t) {                                                       // -Guarding against self assignment
            if(this->content != NULL) delete[] this->content;                   // -Deleting old content
            copyStr(t.name, this->name);
            this->size = t.size;
            this->content = new char[t.size];
            for(unsigned i = 0; i < t.size; i++) this->content[i] = t.content[i];
        }
        return *this;
	}

	void overWrite(const char data[], unsigned dataLen) {                       // -Rewrites TXT files
	    if(this->content != NULL) delete[] this->content;                       // -Deleting old content
	    this->size = dataLen;
	    this->content = new char[dataLen];
	    for(unsigned i = 0; i < dataLen; i++) this->content[i] = data[i];
	}

    void save(const char* fname = NULL) const{                                  // -Saving the content in a .txt file
        std::ofstream file;
        char _fname[NAME_MAX_LEN];
        if(fname == NULL) {                                                     // -If a file name is not provided, then we assign the name of the object
            copyStr(this->name, _fname);
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
};

int main(int argc, char* argv[]) {
    NTRU_N N = _1499_;
    NTRU_q q = _8192_;
    TXT   text;
    char* enc_str = NULL;
    char* consoleInput = NULL;
    char* fileInput = NULL;
    NTRU::ZqPolynomial e_msg;
    char  fileName[NAME_MAX_LEN];
    int   size = 0, option = 0;
    std::ifstream file;
    std::streampos fileSize;
    if(argc == 3) {
        NTRU::Encryption e(argv[1]);
        file.open(argv[2], std::ios::binary | std::ios::ate);                   // Open the file in binary mode and move to the end
        if (!file.is_open()) {
            std::cerr << "Error opening file" << std::endl;
            return -1;
        }
        fileSize = file.tellg();                                                // Get the file size
        file.seekg(0, std::ios::beg);                                           // Move back to the beginning
        fileInput = new char[fileSize];
        file.read(fileInput, fileSize);
        if (!file) {
            std::cerr << "Error reading file" << std::endl;
            delete[] fileInput;
            file.close();
            return -1;
        }
        file.close();
        e_msg = e.encrypt(fileInput, fileSize);
        try { e_msg.save(); }
        catch(const char* str) { std::cerr << str; }
        delete[] fileInput;

        return 0;
    }

    std::cout << "\nPress:\n"                                                   // -Getting input from console
        "(1) to encrypt a binary file.\n"
        "(2) to encrypt a text file.\n"
        "(3) to encrypt text retrieved from console.\n"
        "(4) to generate pairs of keys.\n"
        "Important note: All the encryption options will require a public key file to work.\n";
    std::cin >> option;
    getchar();

    while(option < 1 || option > 4) {
        std::cout << "\nInvalid input. Try again.\n";
        std::cout << "\nPress:\n"
            "(1) to encrypt a binary file.\n"
            "(2) to encrypt a text file.\n"
            "(3) to encrypt text retrieved from console.\n"
            "(4) to generate pairs of keys.\n";
        std::cin >> option;
        getchar();
    }

    if(option != 4) {
        std::cout << "Write the name/path of the public key we will use for encryption. The maximum amount of characters allowed for this is "<<NAME_MAX_LEN<<":\n";
        std::cin.getline(fileName, NAME_MAX_LEN - 1, '\n');
        NTRU::Encryption e(fileName);
        std::cout << "NTRU public key parameters: N = " << e.get_N() << ", q = " << e.get_q() << ".\n";
        const int cipherTextSize   = e.cipherTextSizeInBytes();
        const int plainTextMaxSize = e.plainTextMaxSizeInBytes();
        switch(option) {
            case 1:
                std::cout << "Write the name/path of the file. The maximum amount of characters allowed for this is  " << NAME_MAX_LEN << " :\n";
                std::cin.getline(fileName, NAME_MAX_LEN - 1, '\n');
                file.open(fileName, std::ios::binary | std::ios::ate);          // Open the file in binary mode and move to the end
                if (!file.is_open()) {
                    std::cerr << "Error opening file" << std::endl;
                    return -1;
                }
                fileSize = file.tellg();                                        // Get the file size
                file.seekg(0, std::ios::beg);                                   // Move back to the beginning
                fileInput = new char[fileSize];
                file.read(fileInput, fileSize);
                if (!file) {
                    std::cerr << "Error reading file" << std::endl;
                    delete[] fileInput;
                    file.close();
                    return -1;
                }
                e_msg = e.encrypt(fileInput, fileSize);
                try { e_msg.save(); }
                catch(const char* str) { std::cerr << str; }
                delete[] fileInput; fileInput = NULL;
                break;
            case 2:                                                             // Working on encrypting a text file.
                std::cout << "Write the name/path of the text. The maximum amount of characters allowed for this is  " << NAME_MAX_LEN << " :\n";
                std::cin.getline(fileName, NAME_MAX_LEN - 1, '\n');
                try { text = TXT(fileName); }
                catch(const char* str) { std::cerr << str; }
                enc_str = new char[(unsigned)cipherTextSize];
                e_msg = e.encrypt(text.content, (int)text.size);
                e_msg.println("\nEncrypted message");
                e_msg.toBytes(enc_str);
                text.overWrite(enc_str, (unsigned)cipherTextSize);
                try{ text.save(); }
                catch(const char* str) { std::cerr << str; }
                try { e_msg.save(); }
                catch(const char* str) { std::cerr << str; }
                delete[] enc_str; enc_str = NULL;
                break;
            case 3:
                std::cout << "\nWrite the string you want to encrypt. To process the string sent the value 'EOF', which you can do by:\n\n"
                "- Pressing twice the keys CTRL+Z for Windows.\n"
                "- Pressing twice the keys CTRL+D for Unix and Linux.\n\n"
                "Maximum size for the input: " << plainTextMaxSize << " characters.\n";
                consoleInput = new char[(unsigned)plainTextMaxSize + 1];
                for(size = 0; size < plainTextMaxSize && std::cin.get(consoleInput[size]); size++) {} // -Input from CLI.
                consoleInput[size] = 0;
                e_msg = e.encrypt(consoleInput, size);
                e_msg.println("\nEncrypted message");
                try { e_msg.save("encryptedMessage.ntru"); }
                catch(const char* str) { std::cerr << str; }
                delete[] consoleInput; consoleInput = NULL;
                break;
        }
    } else {
        std::cout << "\nSelect the parameters for the keys seeing them as Polynomials:\n"
        "N = \n"
        "    (1) 509,    (2) 677,    (3) 701,    (4) 821,    (5) 1087,    (6) 1171,    (7) 1499\n";
        std::cin >> option;
        getchar();
        while(option < 1 || option > 7) {
            std::cout << "\nInvalid input. Try again.\n"
            "N = \n"
            "    (1) 509,    (2) 677,    (3) 701,    (4) 821,    (5) 1087,    (6) 1171,    (7) 1499\n";
            std::cin >> option;
            getchar();
        }
        NTRU_N Noptions[] = {_509_,  _677_,  _701_,  _821_, _1087_, _1171_, _1499_};
        N = Noptions[option - 1];
        std::cout << "Selected N = " << N <<"\n\n"
        "q = \n"
        "    (1) 2048,    (2) 4096,    (3) 8192\n";
        std::cin >> option;
        getchar();
        while(option < 1 || option > 3) {
            std::cout << "\nInvalid input. Try again.\n"
            "q = \n"
            "    (1) 2048,    (2) 4096,    (3) 8192\n";
            std::cin >> option;
            getchar();
        }
        NTRU_q qoptions[] = {_2048_, _4096_, _8192_};
        q = qoptions[option - 1];
        std::cout << "Selected q = " << q <<"\n\n";
        NTRU::Encryption keys(N, q);
        std::cout << "Saving Public and private key. Parameters: N = " << N << ", q = " << q <<".\n";
        keys.saveKeys();
    }

    if(consoleInput != NULL) delete[] consoleInput;
    if(fileInput    != NULL) delete[] fileInput;
    if(enc_str      != NULL) delete[] enc_str;

    return EXIT_SUCCESS;
}
