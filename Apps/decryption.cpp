#include"Settings.hpp"

int main(int argc, char *argv[]) {
    if(argc > 1) {                                                              // -Handling arguments from console
        try {
            setEncryptionObjectFromFile(argv[1]);
        } catch(std::runtime_error& exp) {
            std::cout << "Could not create AES::Cipher object.\n" << exp.what() << '\n';
            return EXIT_FAILURE;
        }
        for(int i = 2; i < argc; i++) decryptFile(argv[i]);
        return EXIT_SUCCESS;
    }
    runDecryptionProgram();
    return EXIT_SUCCESS;
}
