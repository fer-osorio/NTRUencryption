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

int main(int argc, char* argv[])
{
    if(argc > 1) std::cout << '\n' << argv[0] << " does not support command "
    "line arguments\n";

    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;

    begin = std::chrono::steady_clock::now();
    NTRU::Encryption e(_821_, _8192_, _821_/3 + 1);
    end = std::chrono::steady_clock::now();

    std::cout << "\nNTRUencryption e(_821_,_8192_,_821_/3+1) execution time = "
    << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()
    << "[µs]\n" << std::endl;

    NTRUPolynomial::Message msg(e.get_N(), 1, 1);
    msg.println("Original message");
    std::cout << '\n';
    NTRUPolynomial::ZqCenterPolynomial e_msg = e.encrypt(msg);
    e_msg.println("Encrypted message");
    std::cout << '\n';
    NTRUPolynomial::Message d_msg = e.decrypt(e_msg);
    d_msg.println("decrypted message");
    if(msg == d_msg) std::cout << "\nSuccessful decryption\n";
    else std::cout << "\nUnsuccessful decryption\n";

    return 0;
}
