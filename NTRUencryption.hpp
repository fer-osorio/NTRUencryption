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
	NTRU_N N;
	NTRU_p p = _3_;
	Z3*  coefficients = NULL;
	int* permutation  = NULL;													// This could be enhanced by making this attributes static
	Z3*  coeffCopy 	  = NULL;													// Copy of the coefficients. Useful at the moment of permute the coefficients

	public:
	ZpPolynomial(const ZpPolynomial& P);
	ZpPolynomial(NTRU_N _N_, int ones=0, int twos=0, NTRU_p _p_=_3_);			// Ones and twos will dictate the amount of 1 and 2 respectively
	ZpPolynomial(NTRU_N _N_, NTRU_p _p_, const char data[], int length);		// Initializing with string of bytes
	~ZpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
		if(this->permutation  != NULL) delete [] this->permutation;
		if(this->coeffCopy 	  != NULL) delete [] this->coeffCopy;
	}
	//******************************* Arithmetic *******************************
	ZpPolynomial  operator + (const ZpPolynomial&) const;						// Addition element by element
	ZpPolynomial  operator - (const ZpPolynomial&) const;						// Subtraction element by element
	ZpPolynomial  operator - () const;
	ZpPolynomial  operator * (const ZpPolynomial&) const;						// Multiplication will coincide with convolution (compare with classical)
	ZpPolynomial& operator-= (const ZpPolynomial&);

	void division(const ZpPolynomial& P,ZpPolynomial rslt[2]) const;			// The quotient and the remainder are saved in rslt
	ZpPolynomial gcdXNmns1(ZpPolynomial& thisBezout) const;

	inline bool operator == (int t) const {										// Comparison with a single integer
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	inline bool operator != (int t) const {										// Comparison with a single integer
		return this->degree() != 0 || this->coefficients[0] != t;
	}
	bool operator == (const ZpPolynomial& P) const;
	bool operator != (const ZpPolynomial& P) const;

	ZpPolynomial& operator = (const ZpPolynomial& P);							// Assignment
	ZpPolynomial& operator = (int t);											// Assignment with single integer

	inline int operator[](int i) const{
		if(i < 0) i = -i;
		if(i > this->N) i %= this->N;
		return this->coefficients[i];
	}
	inline NTRU_N get_N() const{ return this->N; }
	inline NTRU_p get_p() const{ return this->p; }

	void setPermutation();
	void permute();

	inline int degree() const{													// Returns degree of polynomial
		int deg = this->N;
		while(this->coefficients[--deg] == 0 && deg > 0) {}
		return deg;
	}

	ZqPolynomial encrypt(ZqPolynomial publicKey) const;							// -Encrypts the polynomial represented by this and return a ZqPolynomial
	friend ZpPolynomial mods_p(ZqPolynomial P);

	void toBytes(char dest[]) const;
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
};

ZqPolynomial operator - (int64_t, const ZqPolynomial&);							// The intention is to make this function a friend of ZqPolynomial

struct ZqPolynomial {															// Representation of the polynomials in Zq[x]/(x^N-1)
	class Zq {																	// Representation of the group of integers modulus q: Zq
		NTRU_q 	q;
		int64_t q_1;															// Will hold q-1, this will help with mod q operation
		int64_t negq_1;															// This will help in the centering process. Set as ~q_1
		int64_t q_div_2;

		public: Zq(NTRU_q _q_): q(_q_), q_1((int64_t)_q_-1), negq_1(~q_1), q_div_2(q>>1) {}
		public: NTRU_q get_q() const{ return q; }

		int64_t mod_q(int64_t t) const{											// operation t % q
    		if(t >= 0)	return t & this->q_1;									// Equivalent to t % q since q is a power of 2
    		else 		return (t | this->negq_1);								// Computing q - (-t%q) because -t = -(Q-1)q + (q-r) 0 <= r < q
		}
		int64_t mods_q(int64_t a) const{
    		int64_t r;
    		if(a >= 0) r = a & this->q_1;										// Equivalent to a % q since q is a power of 2
    		else r = (a | this->negq_1) & this->q_1;							// Computing q - (-a%q) because -t = -(Q-1)q + (q-r) 0 <= r < q
    		if(r < this->q_div_2) return r;										// At this point we know 0 <= r < q
    		else return r | this->negq_1;										// This is equivalent to r - this->q when r < q
		}
		int64_t add(int64_t a, int64_t b) const{
			return this->mod_q(a+b);
		}
		void addEqual(int64_t& a, int64_t b) const{
			a = this->mod_q(a+b);
		}
		int64_t subtract(int64_t a, int64_t b) const{
			return this->mod_q(a-b);
		}
		int64_t product(int64_t a, int64_t b) const{
			return this->mod_q(a*b);
		}
	};

	public: struct Z2Polynomial {												// Representation of the polynomials in Z2[x]/(x^N-1)
		enum Z2 {_0_ = 0, _1_ = 1};												// Integers modulo 2 (binary numbers)
		inline friend Z2 operator + (Z2 a,Z2 b) {								// Addition modulus 2
			if(a!=b) return _1_;
			return _0_;
		}
		inline friend Z2 operator - (Z2 a,Z2 b) {								// Addition and subtraction coincide in Z2. This is just for evade problems
			if(a!=b) return _1_;												// with notation
			return _0_;
		}
		inline friend Z2 operator * (Z2 a, Z2 b) {								// Multiplication modulus 2
			if(a==0) return _0_;
			return  b ;
		}
		inline friend void operator += (Z2& a, Z2 b) {
			if(a != b) a = _1_;
			else a = _0_;
		}
		inline friend void operator -= (Z2& a, Z2 b) {
			if(a != b) a = _1_;
			else a = _0_;
		}

		private: NTRU_N N;
		Z2* coefficients = NULL;

		private: Z2Polynomial(NTRU_N _N_): N(_N_) {
			this->coefficients = new Z2[_N_];
			for(int i = 0; i < _N_; i++) this->coefficients[i] = _0_;
		}

		public:
		Z2Polynomial(const Z2Polynomial& P);
		Z2Polynomial(const ZpPolynomial& P);
		Z2Polynomial(NTRU_N _N_, int ones);
		~Z2Polynomial() {
			if(this->coefficients != NULL) delete[] this->coefficients;
		}

		inline NTRU_N get_N() const{ return this->N; }

		Z2Polynomial& operator = (const Z2Polynomial& P);
		Z2Polynomial& operator = (const ZpPolynomial& P);
		Z2Polynomial& operator = (Z2 t);
		Z2Polynomial operator + (const Z2Polynomial&) const;					// In Z2, addition (+) coincide with subtraction (-)
		Z2Polynomial operator - (const Z2Polynomial&) const;					// In Z2, addition (+) coincide with subtraction (-)
		Z2Polynomial operator *	(const Z2Polynomial&) const;
		void division(const Z2Polynomial& P,Z2Polynomial res[2]) const;			// Division between this and P, result[2] will save the res[2]
		Z2Polynomial gcdXNmns1(Z2Polynomial& thisBezout) const;					// Greatest common between this and x^N-1 polynomial. Writing
																				// gcd = u·(x^N - 1) + v·this, thisBezout == v
		inline bool operator == (int t) const {
			return this->degree() == 0 && this->coefficients[0] == t;
		}
		bool operator == (const Z2Polynomial& P) const;
		inline bool operator != (int t) const {
			return this->degree() != 0 || this->coefficients[0] != t;
		}
		inline Z2 operator [] (int i) const{
			if(i < 0) i = -i;
			if(i >= this->N) i %= N;
			return this->coefficients[i];
		}
		inline int degree() const{												// Returns degree of polynomial
			int deg = this->N;
			while(this->coefficients[--deg] == _0_ && deg > 0) {}
			return deg;
		}
		void print(const char* name = "", const char* tail = "") const;
		void println(const char* name = "") const;
	};

	friend ZqPolynomial ZpPolynomial::encrypt(ZqPolynomial publicKey) const;

	private: NTRU_N N;
	int64_t* coefficients = NULL;
	Zq zq;

	public:
	ZqPolynomial(NTRU_N, NTRU_q);
	ZqPolynomial(const ZpPolynomial& P,NTRU_q _q_);
	ZqPolynomial(const ZqPolynomial& P);
	ZqPolynomial(const Z2Polynomial& P,NTRU_q _q_);
	~ZqPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	ZqPolynomial& operator = (const ZqPolynomial& P);
	ZqPolynomial& operator = (const ZpPolynomial& P);

	inline int64_t operator[](int i) const{
		if(i < 0) i = -i;
		if(i > this->N) i %= this->N;
		return this->coefficients[i];
	}
	inline NTRU_N get_N() const{ return this->N; }
	inline NTRU_q get_q() const{ return this->zq.get_q(); }

	ZqPolynomial operator + (const ZqPolynomial& P) const;
	ZqPolynomial operator - (const ZqPolynomial& P) const;
	ZqPolynomial operator * (const ZqPolynomial& P) const;
	ZqPolynomial operator * (const ZpPolynomial& P) const;						// This function sees the ZpPolynomial as a ZqPolynomial
	friend ZqPolynomial operator - (int64_t, const ZqPolynomial&);

	bool operator == (const Z2Polynomial& P) const;
	inline bool operator == (int64_t t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}

	void mod_q() const{															// Computes mods q for every coefficient of the polynomial.
		for(int i = 0; i < this->N; i++)
        	this->coefficients[i] = this->zq.mod_q(this->coefficients[i]);
	}
	void mods_q() const{														// Computes mods q for every coefficient of the polynomial.
		for(int i = 0; i < this->N; i++)
        	this->coefficients[i] = this->zq.mods_q(this->coefficients[i]);
	}

	ZqPolynomial getNTRUpublicKey();											// -Provided this object is the inverse in Z[x]/X^N-1 modulo q of the private key,
																				//	this function returns the public key
	void toBytes(char* dest) const;												// -Writes the coefficients into an array of bytes. If a certain coefficient is
																				//  negative, +=q is applied in order to write a positive number
	inline int degree() const{													// -Returns degree of polynomial
		int deg = this->N;
		while(this->coefficients[--deg] == 0 && deg > 0) {}
		return deg;
	}
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
};

class Encryption {
	public:																		// Attributes
	const NTRU_N N;
	const NTRU_q q;
	const NTRU_p p;
	private: int d;

	ZqPolynomial publicKey;
	ZpPolynomial privateKey;
	ZpPolynomial privateKeyInv_p;												// Private key inverse modulo p

	public:
	Encryption(NTRU_N _N_, NTRU_q _q_, int _d_=0,NTRU_p _p_=_3_);

	ZqPolynomial encrypt(const ZpPolynomial&);
	ZqPolynomial encrypt(const char fstring[]);									// Encryption of formatted string
	ZpPolynomial decrypt(const ZqPolynomial&);

	NTRU_N get_N() const{ return this->N; }
	NTRU_p get_p() const{ return this->p; }
	void savePrivateKey_txt();

	private: void setKeys();													// Creation of the keys
};
}
#endif