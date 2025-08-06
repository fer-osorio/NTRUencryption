#include"polynomials.hpp"
#include"statistical_measures.hpp"

#ifndef _ENCRYPTION_HPP_
#define _ENCRYPTION_HPP_

namespace NTRU {

class Encryption {
private:
	// -Initializing with zeros
	RqPolynomial publicKey	= RqPolynomial();
	RpPolynomial privatKey	= RpPolynomial();				// -Private key has the form p·F0+1, we are saving just F0
	bool validPrivateKey	= false;					// -Flag; tells us if current object can only encrypt (only have a valid publickKey)
										// -or also is capable of decryption (has a valid private key).
	template <typename T>
	friend class StatisticalMeasures::Dispersion;
	friend StatisticalMeasures::DataRandomness;
public:
	Encryption();								// -Building keys automatically. Trows: const std::runtime_error&
	Encryption(const char* NTRUkeyFile);					// -Building from a NTRU key file. Trows: NTRU::MathException, NTRU::FileIOException, NTRU::ParameterMismatchException

	RqPolynomial encrypt(const char bytes[] ,size_t size) const;		// -Encryption of char array
	RqPolynomial encrypt(const RpPolynomial&) const;			// -Encrypts RpPolynomial
	RqPolynomial encryptFile(const char fileName[]) const;			// -Reads file content, writes in array, append a 0x80 at the end and encrypts. Throws NTRU::FileIOException
	RpPolynomial decrypt(const RqPolynomial&) const;			// -Decryption of RqPolynomial
	RpPolynomial decrypt(const char bytes[] ,size_t size) const;		// -Decryption of char array

	bool validPrivateKeyAvailable() const{ return this->validPrivateKey; }

	static size_t inputPlainTextMaxSizeInBytes();
	static size_t outputPlainTextMaxSizeInBytes();
	static size_t cipherTextSizeInBytes();
	static size_t privateKeySizeInBytes();
	static size_t publicKeySizeInBytes();
	/*
		Remark: RpPolynomial structure has no method to read or write files, neither to build an object from a byte array nor to write a
		byte array from an object; this is delegated to the encryption class because the building procedure depends on the answer to the
		following question: The data from the byte array represents an arbitrary message waiting to be encrypted (plain text), or it
		represents a private encryption key (private key)? The answer to these questions determines the reading and writing procedures.
	*/
	static RpPolynomial RpPolynomialFromBytes(const char bytes[], size_t size, bool isPrivateKey); // -Builds a RpPolynomial from an array of bytes. The building procesure is determined by the bool argument 'isPrivateKey'. Throws: std::runtime_error&
	static RpPolynomial RpPolynomialPlainTextFromFile(const char* fileName);// -Building a RpPolynomial from file. Throws FileIOException
	static void RpPolynomialtoBytes(const RpPolynomial& org, char dest[], bool isPrivateKey);// -From RpPolynomial to bytes. If isPrivateKey is true, polynomial is interpreted as private key
	static void RpPolynomialPlainTextSave(const RpPolynomial& org, const char* name, bool saveAsText = false);// -Saving RpPolynomial in a file with format. It assumes the data containded inside it has no format. Throws NTRU::FileIOException
	static void RpPolynomialWriteFile(const RpPolynomial& org, const char* fileName, bool writeAsText);// -Writes a file using the data contained in the polynomial. In this case, it looks for the "end-of-content" mark, which is 1000,0000. Throws NTRU::FileIOException
	static void RqPolynomialSave(const RqPolynomial& pl, const char* name, bool saveAsText = false);// -Saving RqPolynomial in a Binary file. Throws NTRU::FileIOException
	static RqPolynomial RqPolynomialFromFile(const char* fileName);		// -Building a RqPolynomial from file. Throws NTRU::FileIOException, NTRU::ParameterMismatchException
	/*	There is no "build private key from file" method because it is already implemented in the constructor Encryption(const char* NTRUkeyFile)	*/

	void saveKeys(const char publicKeyName[] = NULL, const char privateKeyName[] = NULL) const; // -Saves keys using the names passed as arguments. Throws NTRU::FileIOException
	void printKeys(const char publicKeyName[] = NULL, const char privateKeyName[] = NULL) const;

	static RpPolynomial randomTernary();					// -Random Rp polynomial generation
private:
	// -Runs key generation, ciphering or deciphering test depending on the parameter const TestID&
	static StatisticalMeasures::Dispersion<uint32_t> encryption_test(const TestID&,const NTRU::Encryption&,const NTRU::RpPolynomial&,const NTRU::RqPolynomial&);
public:
	// -Performance measurement
	static StatisticalMeasures::Dispersion<uint32_t> keyGenerationTime();
	static StatisticalMeasures::Dispersion<uint32_t> ciphering(const NTRU::Encryption& e, const NTRU::RpPolynomial& msg);// Taking average of encryption time
	static StatisticalMeasures::Dispersion<uint32_t> deciphering(const NTRU::Encryption& e, const NTRU::RqPolynomial& emsg);// Taking average of decryption time
	static StatisticalMeasures::DataRandomness encryptedData(const NTRU::Encryption& e, const std::vector<std::byte>& plain_data);

private:
	static void interchangeZeroFor(RpPolynomial::Z3 t, RpPolynomial& pl);	// -Randomly selects a coefficient with value 0 and a coefficient with value t and interchanges them.
	RqPolynomial productByPrivatKey(const RqPolynomial& P) const;
	RqPolynomial productByPrivatKey(const R2Polynomial& P) const;
	void setKeys();								// -Creation of the keys. Throws NTRU::MathException
	void setKeysFromPrivKey();						// -Creates private key inverse and public key from private key. Intended for the
										//  creation of Encryption object from file. Throws NTRU::MathException
};
}
#endif