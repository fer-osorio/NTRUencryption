#include"../Source/NTRUencryption.hpp"

#ifndef SETTINGS_HPP
#define SETTINGS_HPP
#define NAME_MAX_LEN 256

void showParameters();
void setEncryptionObjectFromFile(const char _fileName_[]);
void encryptFile(const char fileName[]);
void decryptFile(const char fileName[]);
void runEncryptionProgram();
void runDecryptionProgram();

#endif