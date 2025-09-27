#include"../../include/ntru/ntru.hpp"
#include<cstring>
#include<iostream>

int main(int argc, const char* argv[]) {
    if(argc > 1){
        std::cout << "Executable arguments not supported. I will ignore:\n";
        for(int i = 1; i < argc-1; i++) std::cout << argv[i] << ", ";
        std::cout << argv[argc-1] << ".\n";
    }
    NTRU::Encryption ntru;                                                      // Create an encryption instance (generates keys automatically)
    ntru.printKeys();
    ntru.saveKeys();
    const char* plaintext00 = "Wir müssen wissen, wir werden wissen. Die Fakten sind: Wir wissen es nicht, und manchmal werden wir es nie wissen";
    const char* plaintext01 = "Wenn Mathematik konsistent ist, kann sie ihre eigene Konsistenz nicht beweisen. Hier, werden wir es nie wissen... ";
    std::cout << "Plain text: " << plaintext00 << "\n\n";
    std::cout << "Plain text: " << plaintext01 << "\n\n";

    // Create a RpPolynomial instance. Using RpPolynomial(const char data[], int dataLength, bool isPlainText)
    NTRU::RpPolynomial ZpPolyPlaintext00 = NTRU::Encryption::RpPolynomialFromBytes(plaintext00, strlen(plaintext00) + 1, false);
    NTRU::RpPolynomial ZpPolyPlaintext01 = NTRU::Encryption::RpPolynomialFromBytes(plaintext01, strlen(plaintext01) + 1, false);
    ZpPolyPlaintext00.println("Plain Text 1 (vector form)");
    ZpPolyPlaintext01.println("Plain Text 2 (vector form)");

    // Starting with encryption-decryption phase.
    NTRU::RqPolynomial ciphertext00 = ntru.encrypt(ZpPolyPlaintext00);              // Using encrypt(const RqPolynomial&)
    NTRU::RqPolynomial ciphertext01 = ntru.encrypt(ZpPolyPlaintext01);
    std::cout << '\n';
    ciphertext00.println("\nCipher Text 1 (vector form)");
    ciphertext01.println("Cipher Text 2 (vector form)");

    NTRU::RpPolynomial decrypted00 = ntru.decrypt(ciphertext00);                    // Decrypt the message. Using decrypt(const RqPolynomial&)
    NTRU::RpPolynomial decrypted01 = ntru.decrypt(ciphertext01);
    std::cout << '\n';
    decrypted00.println("Decrypted message 1 (vector form)");
    decrypted01.println("Decrypted message 2 (vector form)");
    // Finishing with encryption-decryption phase.

    char decryptedBytes00[1024];
    char decryptedBytes01[1024];
    NTRU::Encryption::RpPolynomialtoBytes(decrypted00, decryptedBytes00, false);    // Convert back to bytes for your application
    NTRU::Encryption::RpPolynomialtoBytes(decrypted01, decryptedBytes01, false);
    decryptedBytes00[strlen(plaintext00)] = 0;
    decryptedBytes01[strlen(plaintext01)] = 0;
    std::cout << "\nDecrypted message 1: " << decryptedBytes00 << '\n';
    std::cout << "\nDecrypted message 2: " << decryptedBytes01 << '\n';

    /*mpz_class num_r = decrypted00.toNumber();                                     // Showing the representation of RpPolynomial through a number
    std::cout << "\nNumber associated with (polynomial) decrypted00 message:\n" << num_r << '\n' << std::endl;
    return 0;*/
}
