#include"polynomials.hpp"
#include"statistical_measures.hpp"

#ifndef _ENCRYPTION_HPP_
#define _ENCRYPTION_HPP_

namespace NTRU {

int get_N();
int get_q();

class Encryption {
private:
	// -Initializing with zeros
	ZqPolynomial publicKey	= ZqPolynomial();
	ZpPolynomial privatKey	= ZpPolynomial();				// -Private key has the form p·F0+1, we are saving just F0
	bool validPrivateKey	= false;					// -Flag; tells us if current object can only encrypt (only have a valid publickKey)
										// -or also is capable of decryption (has a valid private key).
public:
	Encryption();								// -Building keys automatically. Trows: const std::runtime_error&
	Encryption(const char* NTRUkeyFile);					// -Building from a NTRU key file. Trows: NTRU::MathException, NTRU::FileIOException, NTRU::ParameterMismatchException

	ZqPolynomial encrypt(const char bytes[] ,size_t size) const;		// -Encryption of char array
	ZqPolynomial encrypt(const ZpPolynomial&) const;			// -Encrypts ZpPolynomial
	ZqPolynomial encryptFile(const char fileName[]) const;			// -Reads file content, writes in array, append a 0x80 at the end and encrypts. Throws NTRU::FileIOException
	ZpPolynomial decrypt(const ZqPolynomial&) const;			// -Decryption of ZqPolynomial
	ZpPolynomial decrypt(const char bytes[] ,size_t size) const;		// -Decryption of char array

	bool validPrivateKeyAvailable() const{ return this->validPrivateKey; }

	static size_t inputPlainTextMaxSizeInBytes();
	static size_t outputPlainTextMaxSizeInBytes();
	static size_t cipherTextSizeInBytes();
	static size_t privateKeySizeInBytes();
	static size_t publicKeySizeInBytes();
	/*
		Remark: ZpPolynomial structure has no method to read or write files, neither to build an object from a byte array nor to write a
		byte array from an object; this is delegated to the encryption class because the building procedure depends on the answer to the
		following question: The data from the byte array represents an arbitrary message waiting to be encrypted (plain text), or it
		represents a private encryption key (private key)? The answer to these questions determines the reading and writing procedures.
	*/
	static ZpPolynomial ZpPolynomialFromBytes(const char bytes[], size_t size, bool isPrivateKey); // -Builds a ZpPolynomial from an array of bytes. The building procesure is determined by the bool argument 'isPrivateKey'. Throws: std::runtime_error&
	static ZpPolynomial ZpPolynomialPlainTextFromFile(const char* fileName);// -Building a ZpPolynomial from file. Throws FileIOException
	static void ZpPolynomialtoBytes(const ZpPolynomial& org, char dest[], bool isPrivateKey);// -From ZpPolynomial to bytes. If isPrivateKey is true, polynomial is interpreted as private key
	static void ZpPolynomialPlainTextSave(const ZpPolynomial& org, const char* name = NULL, bool saveAsText = false);// -Saving ZpPolynomial in a file with format. It assumes the data containded inside it has no format. Throws NTRU::FileIOException
	static void ZpPolynomialWriteFile(const ZpPolynomial& org, const char* fileName, bool writeAsText);// -Writes a file using the data contained in the polynomial. In this case, it looks for the "end-of-content" mark, which is 1000,0000. Throws NTRU::FileIOException
	/*	There is no "build private key from file" method because it is already implemented in the constructor Encryption(const char* NTRUkeyFile)	*/

	void saveKeys(const char publicKeyName[] = NULL, const char privateKeyName[] = NULL) const; // -Saves keys using the names passed as arguments. Throws NTRU::FileIOException
	void printKeys(const char publicKeyName[] = NULL, const char privateKeyName[] = NULL) const;

	// -Performance measurement
	static StatisticalMeasures::Dispersion<uint32_t> keyGenerationTime();
	static StatisticalMeasures::Dispersion<uint32_t> ciphering(const NTRU::Encryption* e, const NTRU::ZpPolynomial* msg);// Taking average of encryption time
	static StatisticalMeasures::Dispersion<uint32_t> deciphering(const NTRU::Encryption* e, const NTRU::ZqPolynomial* emsg);// Taking average of decryption time
	static StatisticalMeasures::DataRandomness encryptedData(const NTRU::Encryption* ptr_e);

private:
	ZqPolynomial productByPrivatKey(const ZqPolynomial& P) const;
	ZqPolynomial productByPrivatKey(const Z2Polynomial& P) const;
	void setKeys();								// -Creation of the keys. Throws NTRU::MathException
	void setKeysFromPrivKey();						// -Creates private key inverse and public key from private key. Intended for the
										//  creation of Encryption object from file. Throws NTRU::MathException
};
}

// -Helper functions, debugging.
void displayByteArrayBin(const char byteArray[], size_t size);					// -Prints bytes using bynari format
void displayByteArrayChar(const char byteArray[], size_t size);					// -Prints each byte as a character, if not printable, it prints its hexadecimal value, if it is a white space, it prints its hexadecimal value
#endif