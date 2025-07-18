#include"../Source/NTRUencryption.hpp"

int main(int argc, const char* argv[]) {
    if(argc > 1){
        std::cout << "Executable arguments not supported. I will ignore:\n";
        for(int i = 1; i < argc-1; i++) std::cout << argv[i] << ", ";
        std::cout << argv[argc-1] << ".\n";
    }
    NTRU::Encryption ntru;                                                      // Create an encryption instance (generates keys automatically)
    const char* plainText = "Hello, quantum-resistant world!";                  // Encrypt a message
    std::cout << "Plain text: " << plainText << "\n\n";
    // Create a ZpPolynomial instance. Using ZpPolynomial(const char data[], int dataLength, bool isPlainText)
    NTRU::ZpPolynomial ZpPolyPlainText = NTRU::Encryption::ZpPolynomialFromBytes(plainText, strlen(plainText)+1, false);
    ZpPolyPlainText.println("Plain Text (vector form)");

    // Starting with encryption-decryption phase.
    NTRU::ZqPolynomial ciphertext = ntru.encrypt(ZpPolyPlainText);              // Using encrypt(const ZqPolynomial&)
    std::cout << '\n';
    ciphertext.println("Cipher Text (vector form)");
    NTRU::ZpPolynomial decrypted = ntru.decrypt(ciphertext);                    // Decrypt the message. Using decrypt(const ZqPolynomial&)
    std::cout << '\n';
    decrypted.println("Decrypted message (vector form)");
    // Finishing with encryption-decryption phase.

    char decryptedBytes[1024];
    NTRU::Encryption::ZpPolynomialtoBytes(decrypted, decryptedBytes, false);    // Convert back to bytes for your application
    std::cout << "\nDecrypted message: " << decryptedBytes << '\n';

    mpz_class num_r = decrypted.toNumber();                                     // Showing the representation of ZpPolynomial through a number
    std::cout << "\nNumber associated with (polynomial) decrypted message:\n" << num_r << '\n' << std::endl;
    return 0;
}
