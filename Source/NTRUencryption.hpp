#include<iostream>
#include<cstdint>

#ifndef _NTRUENCRYPTION_HPP_
#define _NTRUENCRYPTION_HPP_

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821, _1087_ = 1087, _1171_ = 1171, _1499_ = 1499 };	// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };			// All the possible values for the q
enum NTRU_p {_3_	= 3 };

namespace NTRU {

struct ZpPolynomial;
struct ZqPolynomial;
struct Z2Polynomial;
class  Encryption;

ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&);
ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);
ZpPolynomial mods_p(ZqPolynomial);

struct ZpPolynomial {								// -Representation of the polynomials in Zp[x]/(x^N-1)
	enum Z3{_0_ = 0, _1_ = 1, _2_ = 2};					// -ZpCenterPolynomial are polynomials with coefficients in {-1, 0, 1}
	private:
	static const Z3 Z3add [3][3];						// -Addition in Z3
	static const Z3 Z3subt[3][3];						// -Subtraction in Z3
	static const Z3 Z3prod[3][3];						// -Product in Z3
	static const Z3 Z3neg [3];						// -Negative in Z3
	friend Z3 operator + (Z3 a, Z3 b) {
    	return Z3add[a][b];
	}
	friend void operator += (Z3& a, Z3 b) {
    	a = Z3add[a][b];
	}
	friend Z3 operator - (Z3 a, Z3 b) {
    	return Z3subt[a][b];
	}
	friend Z3 operator - (Z3 a) {
		return Z3neg[a];
	}
	friend void operator -= (Z3& a, Z3 b) {
    	a = Z3subt[a][b];
	}
	friend Z3 operator * (Z3 a, Z3 b) {
    	return Z3prod[a][b];
	}
	private:
	Z3*  coefficients = NULL;
	Z3*  coeffCopy 	  = NULL;						// Copy of the coefficients. Useful at the moment of permute the coefficients
	static int* permutation;						// This could be enhanced by making this attributes static

	public:
	ZpPolynomial();								// -Default constructor; initializes the polynomial with zeros
	ZpPolynomial(const ZpPolynomial& P);
	ZpPolynomial(const char data[], int dataLength);			// -Initializing with string of bytes
	~ZpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
		if(this->coeffCopy    != NULL) delete [] this->coeffCopy;
	}
	static ZpPolynomial randomTernary();
	void interchangeZeroFor(Z3);
	void changeZeroForOne();
	static ZpPolynomial getPosiblePrivateKey();				// -Setting d = N/3, returns a polynomial with d -1's and d+1 1's

	ZpPolynomial  operator + (const ZpPolynomial&) const;			// Addition element by element
	ZpPolynomial  operator - (const ZpPolynomial&) const;			// Subtraction element by element
	ZpPolynomial  operator - () const;
	ZpPolynomial  operator * (const ZpPolynomial&) const;			// Multiplication will coincide with convolution (compare with classical)
	ZpPolynomial& operator-= (const ZpPolynomial&);

	void division(const ZpPolynomial& P,ZpPolynomial rslt[2]) const;	// The quotient and the remainder are saved in rslt
	ZpPolynomial gcdXNmns1(ZpPolynomial& thisBezout) const;

	bool operator == (int t) const {					// Comparison with a single integer
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	bool operator != (int t) const {					// Comparison with a single integer
		return this->degree() != 0 || this->coefficients[0] != t;
	}
	bool operator == (const ZpPolynomial& P) const;
	bool operator != (const ZpPolynomial& P) const;
	ZpPolynomial& operator = (const ZpPolynomial& P);			// Assignment
	ZpPolynomial& operator = (int t);					// Assignment with single integer
	int operator[](int i) const;

	void setPermutation();
	void permute();
	int  degree() const;							// Returns degree of polynomial

	ZqPolynomial encrypt(ZqPolynomial publicKey, bool showEncryptionTime = false) const;// -Encrypts the polynomial represented by this and return a ZqPolynomial

	friend ZpPolynomial mods_p(ZqPolynomial P);
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
	friend ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);

	size_t sizeInBytes() const;
	void toBytes(char dest[]) const;
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;

	void save(const char* name = NULL, bool saveAsText = false) const;	// -Saving ZpPolynomial in a file.
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
	Z2Polynomial  operator *	(const Z2Polynomial&) const;
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
};

ZqPolynomial operator - (int64_t, const ZqPolynomial&);				// The intention is to make this function a friend of ZqPolynomial

struct ZqPolynomial {								// Representation of the polynomials in Zq[x]/(x^N-1)
	private:
	int64_t* coefficients = NULL;

	public:
	ZqPolynomial();
	ZqPolynomial(const ZpPolynomial& P);
	ZqPolynomial(const ZqPolynomial& P);
	ZqPolynomial(const Z2Polynomial& P);
	ZqPolynomial(const char data[], int dataLength);			// -Initializing with string of bytes
	~ZqPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	ZqPolynomial& operator = (const ZqPolynomial& P);
	ZqPolynomial& operator = (const ZpPolynomial& P);

	int64_t operator[](int i) const;

	ZqPolynomial operator + (const ZqPolynomial& P) const;
	ZqPolynomial operator - (const ZqPolynomial& P) const;
	ZqPolynomial operator * (const ZqPolynomial& P) const;
	friend ZqPolynomial operator - (int64_t, const ZqPolynomial&);
	bool operator == (const Z2Polynomial& P) const;
	bool operator == (int64_t t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	bool operator != (int t) const {					// Comparison with a single integer
		return this->degree() != 0 || this->coefficients[0] != t;
	}

	friend ZqPolynomial ZpPolynomial::encrypt(ZqPolynomial publicKey, bool showEncryptionTime) const;
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&);
	friend ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);

	int degree()  const;							// -Returns degree of polynomial
	void mod_q()  const;
	void mods_q() const;
	int lengthInBytes() const;
	static int log2(NTRU_q q);

	ZqPolynomial getNTRUpublicKey();					// -Provided this object is the inverse in Z[x]/X^N-1 modulo q of the private key,
										//  this function returns the public key
	void toBytes(char dest[]) const;					// -Writes the coefficients into an array of bytes. If a certain coefficient is
										//  negative, +=q is applied in order to write a positive number
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;

	void save(const char* name = NULL, bool saveAsText = false) const;	// -Saving ZqPolynomial in a Binary file.
};

class Encryption {
	private:								// -Attributes
	ZqPolynomial publicKey		= ZqPolynomial();			// -Initializing each argument as the zero polynomial
	ZpPolynomial privateKey		= ZpPolynomial();
	ZpPolynomial privateKeyInv_p 	= ZpPolynomial();			// -Private key inverse modulo p
	bool validPrivateKey		= false;				// -Flag; tells us if current object can only encrypt (only have a valid publickKey)
										// -or also is capable of decryption (has a valid private key).
	NTRU_N N;
	NTRU_q q;

	public:
	Encryption();
	Encryption(NTRU_N, NTRU_q);
	Encryption(const char* NTRUkeyFile);					// -Building from a NTRU key file

	ZqPolynomial encrypt(const char bytes[] ,int size, bool showEncryptionTime = false) const;// -Encryption of char array
	ZqPolynomial encrypt(const ZpPolynomial&,bool showEncryptionTime = false) const;// -Encrypts ZpPolynomial
	ZpPolynomial decrypt(const ZqPolynomial&,bool showDecryptionTime = false) const;// -Decryption of ZqPolynomial
	ZpPolynomial decrypt(const char bytes[] ,int size, bool showEncryptionTime = false) const;// -Decryption of char array

	NTRU_N get_N() const { return this->N; }
	NTRU_q get_q() const { return this->q; }
	bool validPrivateKeyAvailable() const{ return this->validPrivateKey; }

	size_t plainTextMaxSizeInBytes() const;
	size_t cipherTextSizeInBytes()   const;
	size_t privateKeySizeInBytes()   const;
	size_t publicKeySizeInBytes()    const;

	void saveKeys(const char publicKeyName[] = NULL, const char privateKeyName[] = NULL) const;

	private:
	void setKeys(bool showKeyCreationTime = false);				// -Creation of the keys
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
			double getAAD()     const{ return this->AvrAbsDev; }

			static Time keyGeneration(NTRU_N,NTRU_q);
			static Time ciphering(NTRU_N,NTRU_q);
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
			double getEntropy() const{ return this->Entropy; }
			double getCorrelation() const { return this->Correlation; }
			double getXiSquare() const{ return this->XiSquare; }
			static Data encryption(NTRU_N,NTRU_q);
		};
	};
};
}
#endif