#include"../Source/NTRUencryption.hpp"

int main(int argc, const char* argv[]) {
    NTRU::Encryption ntru;                      // Create an encryption instance (generates keys automatically)
    const char* plainText = "Hello, quantum-resistant world!";                  // Encrypt a message
    std::cout << "Plain text: " << plainText << '\n';
    NTRU::ZqPolynomial ciphertext = ntru.encrypt(plainText, strlen(plainText)); //  ...
    ciphertext.println("Cipher Text (vector form)");
    NTRU::ZpPolynomial decrypted = ntru.decrypt(ciphertext);                    // Decrypt the message
    decrypted.println("Decrypted message (vector form)")
    char decryptedBytes[1024];
    decrypted.toBytes(decryptedBytes, true);    // Convert back to bytes for your application
    std::cout << "Decrypted message: " << decryptedBytes << '\n';
    return 0;
}
