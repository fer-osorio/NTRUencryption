#include"../Source/NTRUencryption.hpp"

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#define NAME_MAX_LEN 256
#define BYTES_FOR_FILE_SIZE 2                                                   // -Size of the information we will append to the data we intend to encrypt. Now
#define FILE_TYPE_ID_SIZE   3                                                   //  it correspond to 3 bytes for the type of file, and tow bytes for the file size

struct TXT {
	char	name[NAME_MAX_LEN] = {0};					// -Naming instance
	size_t	size = 0;
	char*	content = NULL;							// -Text file content.

	TXT(): name() {}                                                        // -Just for type declaration.
	TXT(const char* fname);							// -Initializing from file
	TXT(const TXT& t);
	~TXT();

	TXT& operator = (const TXT& t);
	void overwrite(const char data[], size_t dataSize);			// -Overwrite content of object
	void save(const char* fname = NULL) const;
};

void setEncryptionObjectFromFile(const char _fileName_[]);
void encryptFile(const char fileName[]);
void decryptFile(const char fileName[]);
void runEncryptionProgram();
void runDecryptionProgram();

#endif