#include<iostream>
#include"../../../include/ntru/parameters_constants.hpp"
#include"../../../include/ntru/encryption.hpp"
#include"../../../include/metrics-analysis/statistical_measures.hpp"
#include"../../../include/print_debug/debug_helpers.hpp"

StatisticalMeasures::DataRandomness NTRU::Encryption::encryptedDataRandomness(const NTRU::Encryption& e, const std::vector<std::byte>& plain_data){
    const size_t plain_blk_sz = e.inputPlainTextMaxSizeInBytes();               // Plain data block size
    const size_t cipher_blk_sz = e.cipherTextSizeInBytes();                     // Cipher block size
    const size_t number_of_rounds = plain_data.size() > 0 ? plain_data.size() / plain_blk_sz: 0;
    const size_t enc_data_sz = cipher_blk_sz*(number_of_rounds + 1);            // Encrypted data size
    // Containers of plain and encypted data
    std::vector<char> plain_blk(plain_blk_sz);
    std::vector<char> cipher_blk(cipher_blk_sz);
    std::vector<char> decrypted_blk(plain_blk_sz);
    std::vector<std::byte> enc_data(enc_data_sz, std::byte{0});
    size_t i, j, k, l, n;
    RqPolynomial enc;
    RpPolynomial dec;
    StatisticalMeasures::DataRandomness dr;

    std::cout << "Encryption::encryptedData: Parameters: N = "<< NTRU_N << ", q = " << NTRU_Q << " ---------------------------------------" << std::endl;
    std::cout << "Input length = " << plain_data.size() << ". Encrypted input length = " << enc_data_sz << ". Block size = " << plain_blk_sz << '\n';

    for(i = k = n = 0, j = plain_blk_sz; j < plain_data.size(); i += plain_blk_sz, j += plain_blk_sz, k += cipher_blk_sz, n++){
        for(l = 0; l < plain_blk_sz; l++) plain_blk[l] = static_cast<char>(plain_data[i+l]); // Copying and casting
        enc = e.encrypt(plain_blk.data(), plain_blk.size());
        enc.toBytes(cipher_blk.data());
        for(l = 0; l < cipher_blk_sz; l++) enc_data[k+l] = static_cast<std::byte>(cipher_blk[l]); // Save encrypted data block
        enc = RqPolynomial(cipher_blk.data(), cipher_blk.size());
        dec = e.decrypt(enc);
        Encryption::RpPolynomialtoBytes(dec, decrypted_blk.data(), false);      // -Second parameter isPrivateKey = false
        if(decrypted_blk != plain_blk){
            std::string dec_fail_msg = "Decryption failure at block " + std::to_string(n) + ".";
            print_first_difference(plain_blk, decrypted_blk, dec_fail_msg, "Plain: ", "Decrypted: ", 16);
            return dr;
        }
    }
    if(j > plain_data.size()){
        j = j - plain_data.size();
        for(l = 0; l < j; l++) plain_blk[l] = static_cast<char>(plain_data[i+l]); // Copying and casting the rest of the data
        enc = e.encrypt(plain_blk.data(), plain_blk.size());
        enc.toBytes(cipher_blk.data());
        for(l = 0; l < cipher_blk_sz; l++) enc_data[k+l] = static_cast<std::byte>(cipher_blk[l]); // Save encrypted data block
        enc = RqPolynomial(cipher_blk.data(), cipher_blk.size());
        dec = e.decrypt(enc);
        Encryption::RpPolynomialtoBytes(dec, decrypted_blk.data(), false);      // -Second parameter isPrivateKey = false
        if(decrypted_blk != plain_blk){
            std::string dec_fail_msg = "Decryption failure at block " + std::to_string(n) + ".";
            print_first_difference(plain_blk, decrypted_blk, dec_fail_msg, "Plain: ", "Decrypted: ", 16);
            return dr;
        }
    }
    dr = StatisticalMeasures::DataRandomness(enc_data);
    return dr;
}
