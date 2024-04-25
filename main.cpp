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

#include<chrono>
#include"NTRUencryption.hpp"

int main(int argc, char* argv[]) {
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;

    begin = std::chrono::steady_clock::now();
    NTRU::Encryption e(_1499_, _8192_, _1499_/3 + 1);
    end = std::chrono::steady_clock::now();
    std::cout << "\nPrivate and public keys generation took "<< std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()<<"[µs]\n"<<std::endl;

    if(argc > 1) {
        char str[301], enc_str[2260];
        str[300] = 0;
        enc_str[2259] = 0;
        begin = std::chrono::steady_clock::now();
        ZqCenterPolynomial e_msg = e.encrypt(argv[1]);
        end = std::chrono::steady_clock::now();
        std::cout << "\nMessage was encrypted in " << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "[µs]\n" << std::endl;
        e_msg.println("Encrypted message");

        e_msg.toBytes(enc_str);
        std::cout << "\n\nEncrypted message:\n";
        for(int i = 0; i < 2250; i++) std::cout << enc_str[i];
        std::cout << "\n\n";

        begin = std::chrono::steady_clock::now();
        NTRUPolynomial::ZpCenterPolynomial d_msg = e.decrypt(e_msg);
        end = std::chrono::steady_clock::now();
        std::cout << "\nMessage was decrypted in " << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "[µs]\n" << std::endl;
        d_msg.toByteArray(str);
        std::cout << "Decrypted message: " << str << '\n';

        return 0;
    }

    NTRUPolynomial::ZpCenterPolynomial msg(e.get_N(), e.get_p(), 1, 1);
    msg.println("Original message");
    std::cout << '\n';

    begin = std::chrono::steady_clock::now();
    NTRUPolynomial::ZqCenterPolynomial e_msg = e.encrypt(msg);
    end = std::chrono::steady_clock::now();
    std::cout << "\nMessage was encrypted in "
    << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "[µs]\n" << std::endl;

    e_msg.println("Encrypted message"); std::cout << '\n';

    begin = std::chrono::steady_clock::now();
    NTRUPolynomial::ZpCenterPolynomial d_msg = e.decrypt(e_msg);
    end = std::chrono::steady_clock::now();
    std::cout << "\nMessage was decrypted in "
    << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "[µs]\n" << std::endl;

    d_msg.println("decrypted message");
    if(msg == d_msg) std::cout << "\nSuccessful decryption\n";
    else std::cout << "\nUnsuccessful decryption\n";

    return 0;
}
