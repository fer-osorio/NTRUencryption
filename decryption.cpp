#include"NTRUencryption.hpp"
#include<fstream>
#include<cstring>

#define BYTES_FOR_FILE_SIZE 2                                                   // -Size of the information we will append to the data we intend to encrypt. Now
#define FILE_TYPE_ID_SIZE   3                                                   //  it correspond to 3 bytes for the type of file, and tow bytes for the file size

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

union ushort_to_char {                                                          // -Casting from short to an array of two chars
    unsigned short ushort;
    char         chars[2];
};

static const char* fileType[] = {"bin", "txt"};                                 // -We will use this to register the type of file we are encrypting

int main(int argc, char* argv[]) {
    NTRU::Encryption e;
    if(argc == 3) {                                                             // -Takes a private key (argv[1]) and a encrypted message (argv[2]) and proceeds
        try{ e = NTRU::Encryption(argv[1]); }                                   //  with decryption. The decrypted message is saved in a binary file.
        catch(const char* exp) { std::cerr << exp; }
        NTRU::ZqPolynomial enc_msg;
        std::ifstream ifile;
        std::ofstream ofile;
        const char ntruq[] = "NTRUq";
        char  ftype[FILE_TYPE_ID_SIZE + 1];
        char* buff = NULL;
        int   encrypHeaderSize = FILE_TYPE_ID_SIZE + BYTES_FOR_FILE_SIZE;
        int   aux = stringLength(ntruq);
        int   lenInBytes = 0;
        ushort_to_char fileSize = {0};                                          // -So we can convert two chars into a short
        NTRU_N N = _509_;
        NTRU_q q = _2048_;
        ifile.open(argv[2], std::ios::binary);
        if(ifile.is_open()) {
            buff = new char[aux + 1];
            ifile.read(buff, aux);
            buff[aux] = 0;
            if(compareStrings(buff, ntruq)) {
                ifile.read((char*)&N, 2);                                       // -A short int for the degree of the polynomial
                ifile.read((char*)&q, 2);                                       // -A short int for the q value
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
                memcpy(ftype, buff, FILE_TYPE_ID_SIZE); ftype[FILE_TYPE_ID_SIZE] = 0;
                memcpy(fileSize.chars, (char*)&buff[FILE_TYPE_ID_SIZE], BYTES_FOR_FILE_SIZE);
                std::cout << "DecryptedMessage: " << (char*)&buff[encrypHeaderSize] << '\n';
                if(compareStrings(ftype, fileType[0])) {                        // -Writing binary file
                    ofile.open("DecryptedMessage.bin", std::ios::binary);
                    if(ofile.is_open()) {
                        ofile.write((char*)&buff[encrypHeaderSize], fileSize.ushort);
                    } else {
                        if(buff != NULL) delete[] buff;
                        std::cerr << "In decryption.cpp, function int main(int argc, char* argv[]): Could not write decrypted file.\n";
                        return EXIT_FAILURE;
                    }
                }
                if(compareStrings(ftype, fileType[1])) {                        // -Writing txt file
                    ofile.open("DecryptedMessage.txt");
                    if(ofile.is_open()) {
                        ofile.write((char*)&buff[encrypHeaderSize], fileSize.ushort);
                    } else {
                        if(buff != NULL) delete[] buff;
                        std::cerr << "In decryption.cpp, function int main(int argc, char* argv[]): Could not write decrypted file.\n";
                        return EXIT_FAILURE;
                    }
                }
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
