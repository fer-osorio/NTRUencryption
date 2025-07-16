#include<iostream>
#include<cstdint>
#include<gmpxx.h>

#ifndef _NTRUENCRYPTION_HPP_
#define _NTRUENCRYPTION_HPP_

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821, _1087_ = 1087, _1171_ = 1171, _1499_ = 1499 };	// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };			// All the possible values for the q
enum NTRU_p {_3_= 3 };

namespace NTRU {

struct ZpPolynomial;
struct ZqPolynomial;
struct Z2Polynomial;
class  Encryption;

ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&);
ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);
ZpPolynomial mods_p(ZqPolynomial);

int get_N();
int get_q();

struct ZpPolynomial {								// -Representation of the polynomials in Zp[x]/(x^N-1)
	enum Z3{_0_ = 0, _1_ = 1, _2_ = 2};					// -ZpCenterPolynomial are polynomials with coefficients in {-1, 0, 1}
	private:
	Z3* coefficients = NULL;
	friend Encryption;

	public:
	ZpPolynomial();								// -Default constructor; initializes the polynomial with zeros
	ZpPolynomial(const ZpPolynomial& P);
	~ZpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
	}
	static ZpPolynomial randomTernary();

	private: void interchangeZeroFor(Z3 t);					// -Randomly selects a coefficient with value 0 and a coefficient with value t and interchanges them.
	private: void changeZeroForOne();

	public:
	static ZpPolynomial getPosiblePrivateKey();				// -Setting d = N/3, returns a polynomial with d -1's and d+1 1's

	ZpPolynomial& operator = (const ZpPolynomial& P);			// Assignment
	int operator[](int i) const;
	int degree() const;							// Returns degree of polynomial

	friend ZpPolynomial mods_p(ZqPolynomial P);
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
	friend ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);

	mpz_class toNumber() const;						// -Interprests the coefficientes as a bese 3 number.
	void print(const char* name = "", bool centered = true, const char* tail = "") const;
	void println(const char* name = "", bool centered = true) const;
};

struct Z2Polynomial {								// Representation of the polynomials in Z2[x]/(x^N-1)
	enum Z2 {_0_ = 0, _1_ = 1};						// Integers modulo 2 (binary numbers)
	friend Z2 operator + (Z2 a,Z2 b) {					// Addition modulus 2
		if(a!=b) return _1_;
		return _0_;
	}
	friend Z2 operator - (Z2 a,Z2 b) {					// Addition and subtraction coincide in Z2. This is just for evade problems
		if(a!=b) return _1_;						// with notation
		return _0_;
	}
	friend Z2 operator * (Z2 a, Z2 b) {					// Multiplication modulus 2
		if(a==0) return _0_;
		return  b ;
	}
	friend void operator += (Z2& a, Z2 b) {
		if(a != b) a = _1_;
		else a = _0_;
	}
	friend void operator -= (Z2& a, Z2 b) {
		if(a != b) a = _1_;
		else a = _0_;
	}

	private:
	Z2* coefficients = NULL;
	friend Encryption;

	public:
	Z2Polynomial();
	Z2Polynomial(const Z2Polynomial& P);
	Z2Polynomial(const ZpPolynomial& P);
	~Z2Polynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}

	Z2Polynomial& operator = (const Z2Polynomial& P);
	Z2Polynomial& operator = (const ZpPolynomial& P);
	Z2Polynomial& operator = (Z2 t);
	Z2Polynomial  operator + (const Z2Polynomial&) const;			// In Z2, addition (+) coincide with subtraction (-)
	Z2Polynomial  operator - (const Z2Polynomial&) const;			// In Z2, addition (+) coincide with subtraction (-)
	Z2Polynomial  operator * (const Z2Polynomial&) const;
	void division(const Z2Polynomial& P,Z2Polynomial res[2]) const;		// Division between this and P, result[2] will save the res[2]
	Z2Polynomial gcdXNmns1(Z2Polynomial& thisBezout) const;			// Greatest common between this and x^N-1 polynomial. Writing
										// gcd = u·(x^N - 1) + v·this, thisBezout == v
	bool operator == (int t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	bool operator == (const Z2Polynomial& P) const;
	bool operator != (int t) const {
		return this->degree() != 0 || this->coefficients[0] != t;
	}
	Z2 operator [] (int i) const;

	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&);

	int  degree() const;
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;

	private:
	void negFirstCoeff() {
		if(this->coefficients[0] == 0) this->coefficients[0] = _1_;
		else this->coefficients[0] = _0_;
	}
};

ZqPolynomial operator - (int64_t, const ZqPolynomial&);				// The intention is to make this function a friend of ZqPolynomial

struct ZqPolynomial {								// Representation of the polynomials in Zq[x]/(x^N-1)
	private:
	int64_t* coefficients = NULL;
	friend Encryption;

	public:
	ZqPolynomial();
	ZqPolynomial(const ZqPolynomial& P);
	ZqPolynomial(const char data[], int dataLength);			// -Initializing with string of bytes
	~ZqPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	ZqPolynomial& operator = (const ZqPolynomial& P);
	int64_t operator[](int i) const;
	ZqPolynomial operator + (const ZqPolynomial& P) const;
	ZqPolynomial operator * (const ZqPolynomial& P) const;
	friend ZqPolynomial operator - (int64_t, const ZqPolynomial&);

	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&);
	friend ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);

	int degree()  const;							// -Returns degree of polynomial
	bool equalsOne() const;
	void mod_q()  const;
	void mods_q() const;
	int lengthInBytes() const;
	static int log2(NTRU_q q);

	void toBytes(char dest[]) const;					// -Writes the coefficients into an array of bytes. If a certain coefficient is
										//  negative, +=q is applied in order to write a positive number
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
	void save(const char* name = NULL, bool saveAsText = false) const;	// -Saving ZqPolynomial in a Binary file. Throws std::runtime_error
	static ZqPolynomial fromFile(const char* fileName);			// -Building a ZqPolynomial from file. Throws std::runtime_error()

	private:
	ZqPolynomial& timesThree();						// -Gets a ZpPolynomial via the operations 3p + 1
};

class Encryption {
	private:								// -Initializing with zeros
	ZqPolynomial publicKey	= ZqPolynomial();
	ZpPolynomial privatKey	= ZpPolynomial();				// -Private key has the form p·F0+1, we are saving just F0
	bool validPrivateKey	= false;					// -Flag; tells us if current object can only encrypt (only have a valid publickKey)
										// -or also is capable of decryption (has a valid private key).
	public:
	Encryption();								// -Building keys automatically. Trows: const std::runtime_error&
	Encryption(const char* NTRUkeyFile);					// -Building from a NTRU key file. Trows: const std::runtime_error&

	ZqPolynomial encrypt(const char bytes[] ,size_t size) const;		// -Encryption of char array
	ZqPolynomial encrypt(const ZpPolynomial&) const;			// -Encrypts ZpPolynomial
	ZqPolynomial encryptFile(const char fileName[]) const;			// -Reads file content, writes in array, append a 0x80 at the end and encrypts.
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
	static ZpPolynomial ZpPolynomialPlainTextFromFile(const char* fileName);// -Building a ZpPolynomial from file. Throws std::runtime_error()
	static void ZpPolynomialtoBytes(const ZpPolynomial& org, char dest[], bool isPrivateKey);// -From ZpPolynomial to bytes. If isPrivateKey is true, polynomial is interpreted as private key
	static void ZpPolynomialPlainTextSave(const ZpPolynomial& org, const char* name = NULL, bool saveAsText = false);// -Saving ZpPolynomial in a file with format. It assumes the data containded inside it has no format
	static void ZpPolynomialWriteFile(const ZpPolynomial& org, const char* fileName, bool writeAsText);// -Writes a file using the data contained in the polynomial. In this case, it looks for the "end-of-content" mark, which is 1000,0000
	/*	There is no "build private key from file" method because it is already implemented in the constructor Encryption(const char* NTRUkeyFile)	*/

	void saveKeys(const char publicKeyName[] = NULL, const char privateKeyName[] = NULL) const;
	void printKeys(const char publicKeyName[] = NULL, const char privateKeyName[] = NULL) const;

	private:

	ZqPolynomial productByPrivatKey(const ZqPolynomial& P) const;
	ZqPolynomial productByPrivatKey(const Z2Polynomial& P) const;
	void setKeys();								// -Creation of the keys
	void setKeysFromPrivKey();						// -Creates private key inverse and public key from private key. Intended for the
										//  creation of Encryption object from file
	public: struct Statistics{
		struct Time{
			private:
			double Maximum  =  0.0;
			double Minimum  =  0.0;
			double Average  = -1.0;
			double Variance =  0.0;
			double AvrAbsDev=  0.0;

			double maximum( const uint64_t time_data[], size_t size) const;
			double minimum( const uint64_t time_data[], size_t size) const;
			double average( const uint64_t time_data[], size_t size) const;
			double variance(const uint64_t time_data[], size_t size) const;
			double avrAbsDev( const uint64_t time_data[], size_t size) const;

			Time(const uint64_t time_data[], size_t size);

			public:
			Time(){}
			double getMaximum() const{ return this->Maximum; }
			double getMinimum() const{ return this->Minimum; }
			double getAverage() const{ return this->Average; }
			double getVariance()const{ return this->Variance;}
			double getAAD()     const{ return this->AvrAbsDev; }	// -Average absolute deviation

			static Time keyGeneration();
			static Time ciphering(const NTRU::Encryption* e, const NTRU::ZpPolynomial* msg);// Taking average of encryption time
			static Time deciphering(const NTRU::Encryption* e, const NTRU::ZqPolynomial* emsg);// Taking average of decryption time
		};
		struct Data{
			private:
			double Entropy = 0.0;
			double XiSquare = 0.0;
			double Correlation = 10.0;

			uint32_t byteValueFrequence[256] = {0};
			bool byteValueFrequenceStablisched = false;
			void setbyteValueFrequence(const char data[], size_t size);

			double entropy(const char data[], size_t size);
			double xiSquare(const char data[], size_t size);
			double correlation(const char data[], size_t size, size_t offset);

			Data(const char data[], size_t size);

			public:
			Data(){}
			Data(const Data& d){
				this->Entropy = d.Entropy;
				this->XiSquare = d.XiSquare;
				this->Correlation = d.Correlation;
				for(int i = 0; i < 256; i++) this->byteValueFrequence[i] = d.byteValueFrequence[i];
				this->byteValueFrequenceStablisched = d.byteValueFrequenceStablisched;
			}
			Data& operator = (const Data& d){
				if(this!=&d){
					this->Entropy = d.Entropy;
					this->XiSquare = d.XiSquare;
					this->Correlation = d.Correlation;
					for(int i = 0; i < 256; i++) this->byteValueFrequence[i] = d.byteValueFrequence[i];
					this->byteValueFrequenceStablisched = d.byteValueFrequenceStablisched;
				}
				return *this;
			}
			double getEntropy() const{ return this->Entropy; }
			double getCorrelation() const { return this->Correlation; }
			double getXiSquare() const{ return this->XiSquare; }
			static Data encryption(const NTRU::Encryption* ptr_e);
		};
	};
};
}

// -Helper functions, debugging.
void displayByteArrayBin(const char byteArray[], size_t size);					// -Prints bytes using bynari format
void displayByteArrayChar(const char byteArray[], size_t size);					// -Prints each byte as a character, if not printable, it prints its hexadecimal value, if it is a white space, it prints its hexadecimal value
#endif