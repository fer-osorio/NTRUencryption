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

#include"../Source/NTRUencryption.hpp"

static void displayUsage() {                                                    // -Executable with no arguments is equivalent to asking for help.
    std::cout << "Usage: NTRUencryption <command> [args...]\n";
    std::cout << "Commands:\n";
    std::cout << "\tencrypt <public_key> <file>                     - Encrypts <file> using <public_key>.\n";
    std::cout << "\tdecrypt <private_key> <file>                    - Decrypts <file> using <private_key>.\n";
    std::cout << "\tbuild_keys <public_key_name> <private_key_name> - Build public-private key, saves them using the provided names." << std::endl;
}

static void showParameters(){
  printf("\t----------------------------------------\n");
  printf("\t| NTRU parmeters: N = %d, q = %d\t|\n", NTRU::get_N(), NTRU::get_q());
  printf("\t| Plain text maximum size = %lu \t|\n", NTRU::inputPlainTextMaxSizeBytes());
  printf("\t| Cipher text size = %lu \t\t|\n",      NTRU::cipherTextSizeBytes());
  printf("\t----------------------------------------\n");
}

int main(int argc, char* argv[]) {
    if(argc < 2) {                                                              // -No argument provided. Displaying usage and returning.
        displayUsage();                                                         //  ...
        return 1;                                                               //  ...
    }
    showParameters(); std::cout << '\n';
    NTRU::Encryption* ptr_e = NULL;
    std::string command(argv[1]);
    char aux[1024];
    if(command == "encrypt" || command == "decrypt"){                           // -In either case we need to build an encryption object using the file provided as
        if(argc != 4){
            std::cerr << "Command >>" << command << "<< must be provided of two parameters.\n";
            return EXIT_FAILURE;
        }
        std::cout << "Executing: " << command << ' ' << argv[2] << ' ' << argv[3] << std::endl;
        try {                                                                   //  second argument.
            ptr_e = new NTRU::Encryption(argv[2]);
        } catch(std::runtime_error& exp) {
            std::cerr << "I could not create NTRU::Encryption object.\n" << exp.what() << '\n';
            return EXIT_FAILURE;
        }
        ptr_e->printKeys();
        if(command == "encrypt"){
            NTRU::ZqPolynomial encMsg;
            try{
                encMsg = ptr_e->encryptFile(argv[3]);        // -Encrypts file
            }catch(const std::runtime_error& exp){
                delete ptr_e;
                std::cerr << "I could not encrypt " << argv[3] << " file.\n" << exp.what() << '\n';
                return EXIT_FAILURE;
            }
            encMsg.println("Encrypted message (vector form)");
            try{
                std::string encMsgFname = "enc_" + std::string(argv[3]);
                encMsg.save(encMsgFname.c_str());
            } catch(const std::runtime_error& exp){
                delete ptr_e;
                std::cerr << "I could not save encrypted message.\n" << exp.what() << '\n';
                return EXIT_FAILURE;
            }
        } else{                                                                 // -Decryption process
            NTRU::ZqPolynomial encMsg;
            try{
                encMsg = NTRU::ZqPolynomial::fromFile(argv[3]);                 // -Creates encrypted message from file
            }catch(const std::runtime_error& exp){
                delete ptr_e;
                std::cerr << "I could not create NTRU::ZqPolynomial object.\n" << exp.what() << '\n';
                return EXIT_FAILURE;
            }
            encMsg.println("Encrypted message");
            NTRU::ZpPolynomial msg = ptr_e->decrypt(encMsg);
            msg.println();
            msg.toBytes(aux, NTRU::inputPlainTextMaxSizeBytes());
            printByteArrayChar(aux, NTRU::inputPlainTextMaxSizeBytes());std::cout << '\n';
            try{
                std::string decMsgFname = "dec_" + std::string(argv[3]);
                msg.writeFile(decMsgFname.c_str(), false);                      // -Decrypting, writing as binary file.
            } catch(const std::runtime_error& exp){
                delete ptr_e;
                std::cerr << "I could not save decrypted message.\n" << exp.what() << '\n';
                return EXIT_FAILURE;
            }
        }
    } else if(command == "build_keys"){
        if(argc != 4){
            std::cerr << "Command >>" << command << "<< must be provided with the names of the keys.\n";
            return EXIT_FAILURE;
        }
        std::cout << "Executing: " << command << ' ' << argv[2] << ' ' << argv[3] << std::endl;
        try {
            ptr_e = new NTRU::Encryption();                                     // -Generating keys automatically.
        } catch(std::runtime_error& exp) {
            std::cerr << "I could not create NTRU::Encryption object.\n" << exp.what() << '\n';
            return EXIT_FAILURE;
        }
        try{
            ptr_e->saveKeys(argv[2],argv[3]);
        } catch(const std::runtime_error& exp){
            delete ptr_e;
            std::cerr << "I could not save public-private key pair.\n" << exp.what() << '\n';
            return EXIT_FAILURE;
        }
    } else{
        std::cerr << "Command not supported." << std::endl;
        return EXIT_FAILURE;
    }
    if(ptr_e!=NULL) delete ptr_e;
    std::cout << "Fertig! Alles in Ordnung." << std::endl;
    return EXIT_SUCCESS;
}
