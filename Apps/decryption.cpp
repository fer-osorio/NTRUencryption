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
    std::cout <<
    "Hello! Thanks for executing me! I am really good at undoing the mess that evil encrypter sis can do. Feel free to use me to decrypt any\n"
    ".txt or .bin file you desire. At any moment you can stop me by pressing the keys 'CTRL+C'. Before anything...\n\n";
    try{
        runDecryptionProgram();
    } catch(std::runtime_error& exp) {
        std::cout << exp.what() << std::endl;
    }
    return EXIT_SUCCESS;
}
