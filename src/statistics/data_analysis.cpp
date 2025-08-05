#include<iostream>
#include"../../include/ntru/parameters_constants.hpp"
#include"../../include/ntru/encryption.hpp"

static void printHex(const char a[], size_t size, const char front[] = "", const char back[] = ""){
    size_t sz = size > 0 ? size - 1: 0;
    size_t i;
    unsigned char ua;
    std::cout << front;
    printf("[");
    for(i = 0; i < sz; i++){
        ua = (unsigned char)a[i];
        if(ua < 0x10) printf("0");
        printf("%X", ua);
    }
    ua = (unsigned char)a[i];
    if(ua < 0x10) printf("0");
    printf("%X", ua);
    printf("]");
    std::cout << back;
}

StatisticalMeasures::DataRandomness NTRU::Encryption::encryptedData(const NTRU::Encryption& e){
    size_t blockSize = e.inputPlainTextMaxSizeInBytes();                   // cipherTextSize/plainTextSize = 5*log2(q)/8
    const size_t cipherBlockSize= e.cipherTextSizeInBytes();               // cipherTextSize = plainTextSize*5*log2(q)/8
    const size_t numberOfRounds = 8*3*512*512/(5*blockSize*(size_t)log2q);      // cipherTextSize*numberOfRounds = 512*512*3 (size of a 256x256 pixel image)
    const size_t dummyEncLen = cipherBlockSize*(numberOfRounds);                // (plainTextSize*5*log2(q)/8)*numberOfRounds = 512*512*3
                                                                                // dummyLen = plainTextSize*numberOfRounds = 512*512*3*8/[5*log2(q)]
    blockSize++;                                                                // numberOfRounds = dummyLen/plainTextSize
    char dummy[NTRU_N];                                                         // -A byte more for blocks to avoid <<out of range>> error.
    char dummy_dec[NTRU_N];
    char* dummy_enc = new char[dummyEncLen];
    size_t i, j, k, l, r;
    int    a = 0;
    RqPolynomial enc;
    RpPolynomial dec;

    std::cout << "Encryption::Statistics::Data::encryption::Parameters: N = "<< NTRU_N << ", q = " << NTRU_Q << " ---------------------------------------" << std::endl;

    for(i = 0; i < blockSize; i++)   dummy[i]     = 0;
    for(i = 0; i < dummyEncLen; i++) dummy_enc[i] = 0;
    for(i = 0; i < blockSize; i++)   dummy_dec[i] = 0;

    blockSize--;                                                                // -Comming back to original size.
    std::cout << "Source/NTRUencryption.cpp, Encryption::Statistics::Data::encryption(): "
                 "Input length = " << blockSize*numberOfRounds << ". Encrypted input length = " << dummyEncLen << ". Block size = " << blockSize << '\n';

    for(j = 0, k = 0, r = 0; k < numberOfRounds; j += cipherBlockSize, k++) {
        if((k&7)==0) std::cout << "Encryption::Statistics::Data::encryption(): Round " << k << std::endl;
        enc = e.encrypt(dummy, blockSize);
        enc.toBytes(dummy_enc + j);
        enc = RqPolynomial(dummy_enc + j, cipherBlockSize);
        dec = e.decrypt(enc);
        Encryption::RpPolynomialtoBytes(dec, dummy_dec, false);                 // -Second parameter isPlainText = true
        for(l = 0; l < blockSize; l++){
            if(dummy[l] != dummy_dec[l]) {
                a = int(l<<1) - 4;
                std::cout << "At block " << k << ": Decryption failure in byte number " << "l = " << l << std::endl; // -Showing firs decryption failure
                if(l < 8) {
                    printHex(dummy,    16, "Block[0,16]           = ", "\n");
                    printHex(dummy_dec,16, "Dec(Enc(Block))[0,16] = ", "\n");
                    if(a > 0) for(; a > 0; a--) std::cout << ' ';
                    std::cout <<           " First occurrence here ~~^" << std::endl;
                } else if(blockSize - l < 8) {
                    printHex(dummy+(blockSize-16),    16, "Block[SZ-16,SZ-1]           = ", "\n");
                    printHex(dummy_dec+(blockSize-16),16, "Dec(Enc(Block))[SZ-16,SZ-1] = ", "\n");
                    if(a > 0) for(; a > 0; a--) std::cout << ' ';
                    std::cout <<                      "       First occurrence here ~~^" << std::endl;
                } else{
                    printHex(dummy+(l-8),    16, "Block[l-8,l+8]           = ", "\n");
                    printHex(dummy_dec+(l-8),16, "Dec(Enc(Block))[l-8,l+8] = ", "\n");
                    for(a = 0; a <16; a++) std::cout << ' ';
                    std::cout <<                 "    First occurrence here ~~^" << std::endl;
                }
                r++;
                break;
            }
        }
        if(dummy[i] == (char)255) i++;
        else dummy[i]++;
    }
    std::cout << "Total amount of rounds: " <<  numberOfRounds << '\n';
    std::cout << "Total amount of decryption failures: " << r;
    Data stats(dummy_enc, dummyEncLen);
    delete[] dummy_enc;
    return stats;
}
