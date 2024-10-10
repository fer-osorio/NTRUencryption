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

#define NAME_MAX_LEN 50

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
            std::cout << errmsg << fname << '\n';
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
    NTRU::Encryption e(_1499_,_8192_);
    if(argc == 2) {
        TXT text, prvKeytxt;
        char enc_str[2437];

        try { text = TXT(argv[1]); }
        catch(const char* str) { std::cout << str; }
        enc_str[2436] = 0;


        NTRU::ZqPolynomial e_msg = e.encrypt(text.content, true);
        e_msg.println("Encrypted message");

        e_msg.toBytes(enc_str);
        std::cout << "\n\nEncrypted message:\n";
        for(int i = 0; i < 2250; i++) std::cout << enc_str[i];
        std::cout << "\n\n";

        text.overWrite(enc_str, 2436);

        try{ text.save(); }
        catch(const char* str) { std::cout << str; }

        return 0;
    }

    char* input = NULL;
    char  cipherText[2500];
    char* aux   = NULL;
    char  output[512];
    unsigned size = 0, i = 0;

    input = new char[1025];
    size = 0;
    std::cout << "\nWrite the string you want to encrypt. To process the string sent the value 'EOF', which you can do by:\n\n"
                 "- Pressing twice the keys CTRL+Z for Windows.\n"
                 "- Pressing twice the keys CTRL+D for Unix and Linux.\n\n";
    while(std::cin.get(input[size++])) {                                        // -Input from CLI.
        if(i == 1024) {
            aux = new char[size];
            std::memcpy(aux, input, size);
            delete[] input;
            input = new char[size + 1025];
            std::memcpy(input, aux, size);
            delete[] aux;
            i = 0;
        } else { i++; }
    }
    input[--size] = 0;
    std::cout << "\n\nInput: " << input << '\n';

    NTRU::ZqPolynomial e_msg = e.encrypt(input, true);

    e_msg.println("Encrypted message in polynomial form"); std::cout << '\n';
    e_msg.toBytes(cipherText);
    std::cout << "Encrypted message with ASCII codification\n\n";
    for(i = 0; i < 2435; i++) std::cout << cipherText[i];
    std::cout << "\n\n";

    NTRU::ZpPolynomial d_msg = e.decrypt(e_msg, true);

    d_msg.toBytes(output);
    std::cout << "Decrypted message: " << output << '\n';

    if(input != NULL) delete[] input;
    if(aux   != NULL) delete[] aux;
    return 0;
}
