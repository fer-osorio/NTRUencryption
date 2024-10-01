#include<iostream>
#include<cstdint>

#ifndef _NTRUENCRYPTION_HPP_
#define _NTRUENCRYPTION_HPP_

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821, _1087_ = 1087, _1171_ = 1171, _1499_ = 1499 };	// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };						// All the possible values for the q
enum NTRU_p {_3_	= 3 };

namespace NTRU {

struct ZqPolynomial;
struct ZpPolynomial;
class  Encryption;

ZpPolynomial mods_p(ZqPolynomial);

struct ZpPolynomial {															// Representation of the polynomials in Zp[x]/(x^N-1)
	enum Z3{_0_ = 0, _1_ = 1, _2_ = 2};											// ZpCenterPolynomial are polynomials with coefficients in {-1, 0, 1}
	private:
	static const Z3 Z3add [3][3];												// Addition in Z3
	static const Z3 Z3subt[3][3];												// Subtraction in Z3
	static const Z3 Z3prod[3][3];												// Product in Z3
	static const Z3 Z3neg [3];													// Negative in Z3
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
	/*NTRU_N N;
	NTRU_p p = _3_;*/
	Z3*  coefficients = NULL;
	int* permutation  = NULL;													// This could be enhanced by making this attributes static
	Z3*  coeffCopy 	  = NULL;													// Copy of the coefficients. Useful at the moment of permute the coefficients

	public:
	ZpPolynomial();																// -Default constructor; initializes the polynomial with zeros
	ZpPolynomial(const ZpPolynomial& P);
	//ZpPolynomial(NTRU_N _N_, int ones=0, int twos=0, NTRU_p _p_=_3_);			// Ones and twos will dictate the amount of 1 and 2 respectively
	ZpPolynomial(const char data[], int dataLength);							// Initializing with string of bytes
	~ZpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
		if(this->permutation  != NULL) delete [] this->permutation;
		if(this->coeffCopy 	  != NULL) delete [] this->coeffCopy;
	}
	static ZpPolynomial randomTernary();
	void changeAzeroForAone();
	static ZpPolynomial getPosiblePrivateKey();									// -Setting d = N/3, returns a polynomial with d -1's and d+1 1's
	//******************************* Arithmetic *******************************
	ZpPolynomial  operator + (const ZpPolynomial&) const;						// Addition element by element
	ZpPolynomial  operator - (const ZpPolynomial&) const;						// Subtraction element by element
	ZpPolynomial  operator - () const;
	ZpPolynomial  operator * (const ZpPolynomial&) const;						// Multiplication will coincide with convolution (compare with classical)
	ZpPolynomial& operator-= (const ZpPolynomial&);

	void division(const ZpPolynomial& P,ZpPolynomial rslt[2]) const;			// The quotient and the remainder are saved in rslt
	ZpPolynomial gcdXNmns1(ZpPolynomial& thisBezout) const;

	bool operator == (int t) const {										// Comparison with a single integer
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	bool operator != (int t) const {										// Comparison with a single integer
		return this->degree() != 0 || this->coefficients[0] != t;
	}
	bool operator == (const ZpPolynomial& P) const;
	bool operator != (const ZpPolynomial& P) const;

	ZpPolynomial& operator = (const ZpPolynomial& P);							// Assignment
	ZpPolynomial& operator = (int t);											// Assignment with single integer

	int operator[](int i) const;

	/*inline NTRU_N get_N() const{ return this->N; }
	inline NTRU_p get_p() const{ return this->p; }*/

	void setPermutation();
	void permute();

	int degree() const;															// Returns degree of polynomial

	ZqPolynomial encrypt(ZqPolynomial publicKey) const;							// -Encrypts the polynomial represented by this and return a ZqPolynomial
	friend ZpPolynomial mods_p(ZqPolynomial P);

	void toBytes(char dest[]) const;
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;

	void save(const char* name = NULL) const;									// -Saving ZpPolynomial in a Binary file.
};

ZqPolynomial operator - (int64_t, const ZqPolynomial&);							// The intention is to make this function a friend of ZqPolynomial

struct ZqPolynomial {															// Representation of the polynomials in Zq[x]/(x^N-1)
	public: struct Z2Polynomial {												// Representation of the polynomials in Z2[x]/(x^N-1)
		enum Z2 {_0_ = 0, _1_ = 1};												// Integers modulo 2 (binary numbers)
		friend Z2 operator + (Z2 a,Z2 b) {										// Addition modulus 2
			if(a!=b) return _1_;
			return _0_;
		}
		friend Z2 operator - (Z2 a,Z2 b) {										// Addition and subtraction coincide in Z2. This is just for evade problems
			if(a!=b) return _1_;												// with notation
			return _0_;
		}
		friend Z2 operator * (Z2 a, Z2 b) {										// Multiplication modulus 2
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
		Z2Polynomial operator + (const Z2Polynomial&) const;					// In Z2, addition (+) coincide with subtraction (-)
		Z2Polynomial operator - (const Z2Polynomial&) const;					// In Z2, addition (+) coincide with subtraction (-)
		Z2Polynomial operator *	(const Z2Polynomial&) const;
		void division(const Z2Polynomial& P,Z2Polynomial res[2]) const;			// Division between this and P, result[2] will save the res[2]
		Z2Polynomial gcdXNmns1(Z2Polynomial& thisBezout) const;					// Greatest common between this and x^N-1 polynomial. Writing
																				// gcd = u·(x^N - 1) + v·this, thisBezout == v
		bool operator == (int t) const {
			return this->degree() == 0 && this->coefficients[0] == t;
		}
		bool operator == (const Z2Polynomial& P) const;
		bool operator != (int t) const {
			return this->degree() != 0 || this->coefficients[0] != t;
		}
		Z2 operator [] (int i) const;
		int degree() const;
		void print(const char* name = "", const char* tail = "") const;
		void println(const char* name = "") const;
	};

	friend ZqPolynomial ZpPolynomial::encrypt(ZqPolynomial publicKey) const;

	private:
	int64_t* coefficients = NULL;

	public:
	ZqPolynomial();
	ZqPolynomial(const ZpPolynomial& P);
	ZqPolynomial(const ZqPolynomial& P);
	ZqPolynomial(const Z2Polynomial& P);
	~ZqPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	ZqPolynomial& operator = (const ZqPolynomial& P);
	ZqPolynomial& operator = (const ZpPolynomial& P);

	int64_t operator[](int i) const;

	ZqPolynomial operator + (const ZqPolynomial& P) const;
	ZqPolynomial operator - (const ZqPolynomial& P) const;
	ZqPolynomial operator * (const ZqPolynomial& P) const;
	ZqPolynomial operator * (const ZpPolynomial& P) const;						// This function sees the ZpPolynomial as a ZqPolynomial
	friend ZqPolynomial operator - (int64_t, const ZqPolynomial&);

	bool operator == (const Z2Polynomial& P) const;
	bool operator == (int64_t t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	int degree() const;															// -Returns degree of polynomial
	void mod_q() const;
	void mods_q() const;

	ZqPolynomial getNTRUpublicKey();											// -Provided this object is the inverse in Z[x]/X^N-1 modulo q of the private key,
																				//	this function returns the public key
	void toBytes(char* dest) const;												// -Writes the coefficients into an array of bytes. If a certain coefficient is
																				//  negative, +=q is applied in order to write a positive number
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;

	void save(const char* name = NULL) const;									// -Saving ZqPolynomial in a Binary file.
};

class Encryption {
	public:																		// Attributes
	ZqPolynomial publicKey;
	ZpPolynomial privateKey;
	ZpPolynomial privateKeyInv_p;												// Private key inverse modulo p

	public:
	Encryption();																// -Sets the NTRU parameters to N = 1499 and q = 8192
	Encryption(NTRU_N N, NTRU_q  q);

	ZqPolynomial encrypt(const ZpPolynomial&);
	ZqPolynomial encrypt(const char fstring[]);									// Encryption of formatted string
	ZpPolynomial decrypt(const ZqPolynomial&);

	NTRU_N get_N() const;
	NTRU_q get_q() const;
	void setNTRUparameters(NTRU_N, NTRU_q) const;								// -Sets the value of NTRUencryption algorithm

	void savePrivateKey_txt();

	private:
	void setKeys();																// Creation of the keys
};
}
#endif