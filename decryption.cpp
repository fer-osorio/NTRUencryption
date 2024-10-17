#include"NTRUencryption.hpp"
#include<fstream>

static int stringLength(const char str[]) {
    int l = -1;
    while(str[++l] != 0) {}
    return l;
}

static bool compareStrings(const char* str1, const char* str2) {
    int i = 0;
    if(str1 == NULL || str2 == NULL) return false;
    while(str1[i] == str2[i] && str1[i] != 0) { i++; }
    return str1[i] == str2[i];
}

int main(int argc, char* argv[]) {
    if(argc == 3) {                                                             // -Takes a private key (argv[1]) and a encrypted message (argv[2]) and proceeds
        NTRU::Encryption e(argv[1]);                                            //  with decryption. The decrypted message is saved in a binary file.
        NTRU::ZqPolynomial enc_msg;
        std::ifstream ifile;
        std::ofstream ofile;
        const char ntruq[] = "NTRUq";
        char* buff = NULL;
        int   aux = stringLength(ntruq);
        int   lenInBytes = 0;
        NTRU_N N = _509_;
        NTRU_q q = _2048_;
        ifile.open(argv[2]);
        if(ifile.is_open()) {
            buff = new char[aux + 1];
            ifile.read(buff, aux);
            buff[aux] = 0;
            if(compareStrings(buff, ntruq)) {
                ifile.read((char*)&N, 2);                                        // -A short int for the degree of the polynomial
                ifile.read((char*)&q, 2);                                        // -A short int for the q value
                lenInBytes = N*NTRU::ZqPolynomial::log2(q)/8 + 1;
                delete[] buff;
                buff = new char[lenInBytes];
                ifile.read(buff, lenInBytes);
                enc_msg = NTRU::ZqPolynomial(buff, lenInBytes);
                delete[] buff;
                ifile.close();
                NTRU::ZpPolynomial msg = e.decrypt(enc_msg);
                lenInBytes = e.plainTextMaxSizeInBytes();
                buff = new char[lenInBytes];
                msg.toBytes(buff);
                std::cout << "DecryptedMessage: " << buff << '\n';
                /*ofile.open("DecryptedMessage");
                if(ofile.is_open()) {
                    ofile.write(buff, lenInBytes);
                } else {
                    if(buff != NULL) delete[] buff;
                    std::cerr << "In decryption.cpp, function int main(int argc, char* argv[]): Could not write decrypted file.\n";
                    return EXIT_FAILURE;
                }*/
            } else {
                if(buff != NULL) delete[] buff;
                std::cerr << "In decryption.cpp, function int main(int argc, char* argv[]): Not a ntruq file.\n";
                return EXIT_FAILURE;
            }
        } else {
            if(buff != NULL) delete[] buff;
            std::cerr <<  "In decryption.cpp, function int main(int argc, char* argv[]): Could not open file.\n";
            return EXIT_FAILURE;
        }
        if(buff != NULL) delete[] buff;
        return EXIT_SUCCESS;
    }
    return EXIT_SUCCESS;
}
