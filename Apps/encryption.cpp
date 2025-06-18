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

#include"Settings.hpp"

int main(int argc, char* argv[]) {
    showParameters();
    if(argc > 1) {                                                              // -Handling arguments from console
        try {
            setEncryptionObjectFromFile(argv[1]);
        } catch(std::runtime_error& exp) {
            std::cout << "Could not create NTRU::Encryption object.\n" << exp.what() << '\n';
            return EXIT_FAILURE;
        }
        for(int i = 2; i < argc; i++) encryptFile(argv[i]);
        return EXIT_SUCCESS;
    }
    std::cout <<
    "\nHi! I am a program which is particularly good at encrypting binary and text files (as long as they are not too big!!). Feel\n"
    "free to use me to encrypt any .txt or .bin file you desire. At any moment you can stop me by pressing the keys 'CTRL+C'.\n"
    "Before anything...\n\n";
    runEncryptionProgram();
    return EXIT_SUCCESS;
}
