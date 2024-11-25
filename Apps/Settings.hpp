#include"../Source/NTRUencryption.hpp"

#ifndef SETTINGS_HPP
#define SETTINGS_HPP
#define NAME_MAX_LEN 256

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