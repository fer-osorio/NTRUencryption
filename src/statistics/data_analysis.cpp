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

static void print_differences(std::vector<char> v1, std::vector<char> v2, size_t numb_diff){
    if(v1 == v2) return;
    size_t s1 = v1.size(), s2 = v2.size();
    if(s1 != s2) {
        std::cout << "Vectors differ in size. First vector size: " << s1 << ", second vector size: " << s2 << "\n";
        return;
    }
    for(size_t i = 0; i < s1; i++){
        if(v1[i] != v2[i]){}
    }
}

// cipherTextSize/plainTextSize = 5*log2(q)/8
// cipherTextSize = plainTextSize*5*log2(q)/8
// cipherTextSize*numberOfRounds = 512*512*3 (size of a 256x256 pixel image)
// (plainTextSize*5*log2(q)/8)*numberOfRounds = 512*512*3
// dummyLen = plainTextSize*numberOfRounds = 512*512*3*8/[5*log2(q)]
// numberOfRounds = dummyLen/plainTextSize
// -A byte more for blocks to avoid <<out of range>> error.

StatisticalMeasures::DataRandomness NTRU::Encryption::encryptedData(const NTRU::Encryption& e, const std::vector<std::byte>& plain_data){
    const size_t plain_blk_sz = e.inputPlainTextMaxSizeInBytes();               // Plain data block size
    const size_t cipher_blk_sz = e.cipherTextSizeInBytes();                     // Cipher block size
    const size_t number_of_rounds = plain_data.size() > 0 ? plain_data.size() / plain_blk_sz + 1: 0;
    const size_t enc_data_sz = cipher_blk_sz*number_of_rounds;                  // Encrypted data size
    std::vector<char> plain_blk(plain_data.begin(), plain_data.begin() + plain_blk_sz); //plain_blk(plain_blk_sz);
    std::vector<char> cipher_blk(cipher_blk_sz);
    std::vector<char> decrypted_blk(plain_blk_sz);
    std::vector<std::byte> enc_data(enc_data_sz, std::byte{0});
    size_t i, j, k, l, r;
    int a = 0;
    RqPolynomial enc;
    RpPolynomial dec;
    StatisticalMeasures::DataRandomness dr;

    for(i = 0, j = plain_blk_sz, k = 0; j < plain_data.size(); i += plain_blk_sz, j += plain_blk_sz, k += cipher_blk_sz){
        plain_blk = std::vector<char>(plain_data.begin() + i, plain_data.begin() + j);
        enc = e.encrypt(plain_blk.data(), plain_blk.size());
        enc.toBytes(cipher_blk.data());
        for(l = 0; l < cipher_blk_sz; l++) enc_data[k+l] = static_cast<std::byte>(cipher_blk[l]); // Save encrypted data block
        enc = RqPolynomial(cipher_blk.data(), cipher_blk.size());
        dec = e.decrypt(enc);
        Encryption::RpPolynomialtoBytes(dec, decrypted_blk.data(), false);      // -Second parameter isPrivateKey = false
        if(decrypted_blk != plain_blk){}
    }

    std::cout << "Encryption::Statistics::Data::encryption::Parameters: N = "<< NTRU_N << ", q = " << NTRU_Q << " ---------------------------------------" << std::endl;
    std::cout << "Source/NTRUencryption.cpp, Encryption::Statistics::Data::encryption(): "
                 "Input length = " << plain_blk_sz*number_of_rounds << ". Encrypted input length = " << enc_data_sz << ". Block size = " << plain_blk_sz << '\n';

    for(j = 0, k = 0, r = 0; k < number_of_rounds; j += cipher_blk_sz, k++) {
        if((k&7)==0) std::cout << "Encryption::Statistics::Data::encryption(): Round " << k << std::endl;
        enc = e.encrypt(dummy, plain_blk_sz);
        enc.toBytes(dummy_enc + j);
        enc = RqPolynomial(dummy_enc + j, cipher_blk_sz);
        dec = e.decrypt(enc);
        Encryption::RpPolynomialtoBytes(dec, dummy_dec, false);                 // -Second parameter isPlainText = true
        for(l = 0; l < plain_blk_sz; l++){
            if(dummy[l] != dummy_dec[l]) {
                a = int(l<<1) - 4;
                std::cout << "At block " << k << ": Decryption failure in byte number " << "l = " << l << std::endl; // -Showing firs decryption failure
                if(l < 8) {
                    printHex(dummy,    16, "Block[0,16]           = ", "\n");
                    printHex(dummy_dec,16, "Dec(Enc(Block))[0,16] = ", "\n");
                    if(a > 0) for(; a > 0; a--) std::cout << ' ';
                    std::cout <<           " First occurrence here ~~^" << std::endl;
                } else if(plain_blk_sz - l < 8) {
                    printHex(dummy+(plain_blk_sz-16),    16, "Block[SZ-16,SZ-1]           = ", "\n");
                    printHex(dummy_dec+(plain_blk_sz-16),16, "Dec(Enc(Block))[SZ-16,SZ-1] = ", "\n");
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
    std::cout << "Total amount of rounds: " <<  number_of_rounds << '\n';
    std::cout << "Total amount of decryption failures: " << r;
    Data stats(dummy_enc, enc_data_sz);
    delete[] dummy_enc;
    return stats;
}
