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

static void displayUsage(){
    std::cout << "NTRUencryption <command> [args...]\n";
    std::cout << "Commands:\n";
    std::cout << "\tencrypt <public_key> <file>     -Encrypts <file> using <public_key>.\n";
    std::cout << "\t                                -If <public_key> is equal to \"-\", then public key is build atomatically.\n";
    std::cout << "\tdecrypt <private_key> <file>    -Decrypts <file> using <private_key>.\n";
    std::cout << "\tbuild_keys                      -Builds and saves public-private key pair." << std::endl;
}

static void showParameters(){
  printf("\t----------------------------------------\n");
  printf("\t| NTRU parmeters: N = %d, q = %d\t|\n", NTRU::get_N(), NTRU::get_q());
  printf("\t----------------------------------------\n");
}

int main(int argc, char* argv[]) {
    if(argc < 2){                                                               // -Here, calling the program with no arguments is equivalent to asking for help
        displayUsage();
        return EXIT_SUCCESS;
    }
    showParameters();
    NTRU::Encryption* ptr_e = NULL;
    std::string command(argv[1]);
    if(command == "encrypt"){
        try{
            ptr_e = new NTRU::Encryption(argv[2]);
        } catch(const std::runtime_error& exp){
            std::cout << "I could not create NTRU::Encryption object.\n" << exp.what() << '\n';
            return EXIT_FAILURE;
        }
        NTRU::ZpPolynomial plainText(argv[3]);
        return EXIT_SUCCESS;
    }
    if(command == "decrypt"){
        try{
            ptr_e = new NTRU::Encryption(argv[2]);
        } catch(const std::runtime_error& exp){
            std::cout << "I could not create NTRU::Encryption object.\n" << exp.what() << '\n';
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if(command == "build_keys"){
        NTRU::Encryption e;
        e.saveKeys();
        return EXIT_SUCCESS;
    }
    if(ptr_e != NULL) delete ptr_e;
    return EXIT_SUCCESS;
}

/*
// Assume you have this class in a file like "Calculator.h"
class Calculator {
public:
    int add(int a, int b) { return a + b; }
    int subtract(int a, int b) { return a - b; }
};

void printUsage() {
    std::cerr << "Usage: my_app <command> [args...]" << std::endl;
    std::cerr << "Commands:" << std::endl;
    std::cerr << "  add <int> <int>      - Adds two integers." << std::endl;
    std::cerr << "  subtract <int> <int> - Subtracts two integers." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    // Instantiate your class
    Calculator calculator;

    try {
        if (command == "add") {
            if (args.size() < 2) {
                std::cerr << "Error: 'add' requires two integer arguments." << std::endl;
                return 1;
            }
            int num1 = std::stoi(args[0]); // std::stoi converts string to int
            int num2 = std::stoi(args[1]);
            int result = calculator.add(num1, num2);
            std::cout << "Result: " << result << std::endl;

        } else if (command == "subtract") {
            if (args.size() < 2) {
                std::cerr << "Error: 'subtract' requires two integer arguments." << std::endl;
                return 1;
            }
            int num1 = std::stoi(args[0]);
            int num2 = std::stoi(args[1]);
            int result = calculator.subtract(num1, num2);
            std::cout << "Result: " << result << std::endl;

        } else {
            std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
            printUsage();
            return 1;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid number provided." << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Number is out of range." << std::endl;
        return 1;
    }

    return 0;
}
*/
