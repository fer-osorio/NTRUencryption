#include<iostream>

#ifndef _NTRUPOLYNOMIALS_HPP_
#define _NTRUPOLYNOMIALS_HPP_

#define DECIMAL_BASE 10

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821, _1087_ = 1087, _1171_ = 1171, _1499_ = 1499 };	// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };						// All the possible values for the q
enum NTRU_p {_3_	= 3 };

namespace NTRUPolynomial {

struct ZqPolynomial;
struct ZpCenterPolynomial;
struct ZqCenterPolynomial;
struct ZpPolynomial {															// Representation of the polynomials in Zp[x]/(x^N-1)
	private: enum Z3{_0_ = 0, _1_ = 1, _2_ = 2};								// ZpCenterPolynomial are polynomials with coefficients in {-1, 0, 1}
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
	ZpPolynomial(const ZpCenterPolynomial& P);
	ZpPolynomial(NTRU_N _N_, int ones=0, int twos=0, NTRU_p _p_=_3_);			// Ones and twos will dictate the amount of 1 and 2 respectively
	~ZpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
		if(this->permutation  != NULL) delete [] this->permutation;
		if(this->coeffCopy 	  != NULL) delete [] this->coeffCopy;
	}
	// Arithmetic
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
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
	void test(NTRU_N _N_, int d) const;
};

ZqPolynomial operator - (unsigned, const ZqPolynomial&);								// The intention is to make this function a friend of ZqPolynomial

struct ZqPolynomial {															// Representation of the polynomials in Zq[x]/(x^N-1)
	class Zq{																	// Representation of the group of integers modulus q: Zq
		NTRU_q q;
		unsigned  q_1;															// Will hold q-1, this will help with mod q operation

		public: Zq(NTRU_q _q_): q(_q_), q_1((unsigned)_q_-1) {}
		public: inline NTRU_q get_q() const{ return q; }

		unsigned mod_q(unsigned t) const{										// operation t % q
			return t & this->q_1;												// Since q is a power of two, t & (q-1) is equivalent to t%q, but much faster
		}
		unsigned add(unsigned a, unsigned b) const{
			return (a+b) & this->q_1;
		}
		void addEqual(unsigned& a, unsigned b) const{
			a = (a+b) & this->q_1;
		}
		unsigned subtract(unsigned a, unsigned b) const{
			return (a+this->negative(b)) & this->q_1;
		}
		unsigned product(unsigned a, unsigned b) const{
			return a*b & this->q_1;
		}
		unsigned negative(unsigned a) const{
			if(a == 0) return a;
			return -(a&this->q_1)&this->q_1;									// Since q is a power of two, this is equivalent to q - a%q
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
		void test(NTRU_N _N_, int d) const;
	};

	private: NTRU_N N;
	unsigned* coefficients = NULL;
	Zq _Zq_;

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

	inline unsigned operator[](int i) const{
		if(i < 0) i = -i;
		if(i > this->N) i %= this->N;
		return this->coefficients[i];
	}
	inline NTRU_N get_N() const{ return this->N; }
	inline NTRU_q get_q() const{ return this->_Zq_.get_q(); }

	ZqPolynomial operator - (const ZqPolynomial& P) const;
	ZqPolynomial operator * (const ZqPolynomial& P) const;
	friend ZqPolynomial operator - (unsigned, const ZqPolynomial&);

	bool operator == (const Z2Polynomial& P) const;
	inline bool operator == (unsigned t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}

	inline int degree() const{													// Returns degree of polynomial
		int deg = this->N;
		while(this->coefficients[--deg] == 0 && deg > 0) {}
		return deg;
	}
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
};

class ZpCenterPolynomial {
	enum Z3{_1_ = -1, _0 = 0, _1 = 1};											// ZpCenterPolynomial are polynomials with coefficients in {-1, 0, 1}
	static const Z3 Z3add [3][3];												// Centered addition in Z3
	static const Z3 Z3subs[3][3];												// Centered subtraction in Z3
	friend Z3 operator + (Z3 a, Z3 b) {
		int i = a, j = b;
		if(i == -1) i = 2;
    	if(j == -1) j = 2;
    	return ZpCenterPolynomial::Z3add[i][j];
	}
	friend void operator += (Z3& a, Z3 b) {
		int i = a, j = b;
		if(i == -1) i = 2;
    	if(j == -1) j = 2;
    	a = ZpCenterPolynomial::Z3add[i][j];
	}
	friend Z3 operator - (Z3 a, Z3 b) {
		int i = a, j = b;
		if(i == -1) i = 2;
    	if(j == -1) j = 2;
    	return ZpCenterPolynomial::Z3subs[i][j];
	}
	friend Z3 operator * (Z3 a, Z3 b) {
		if(a == _0) return  _0;
    	if(b == _0) return  _0;
		if(a == _1) return   b;
    	if(b == _1) return _1_;
    	return _1;
	}
	class ZpCentered {															// Handling elements in Z3 with centered elements (-1,0,1)
		NTRU_p p;

		public:
		ZpCentered(NTRU_p _p_): p(_p_) {}
		int addition(int a, int b)	const;										// Supposing a,b are in the set {-p/2 - 1,...,0,..,p/2 - 1}
		int subtraction(int a, int b)const;										// Supposing a,b are in the set {-p/2 - 1,...,0,..,p/2 - 1}
		int product(int a, int b) 	const;										// Supposing a,b are in the set {-p/2 - 1,...,0,..,p/2 - 1}

		NTRU_p get_p() const{return this->p;}
	};

	NTRU_N N;
	NTRU_p p;
	Z3* coefficients = NULL;

	public:
	ZpCenterPolynomial(const ZpCenterPolynomial&);
	ZpCenterPolynomial(const ZpPolynomial&);
	ZpCenterPolynomial(NTRU_N _N_, NTRU_p _p_, unsigned ones = 0, unsigned negOnes = 0);
	ZpCenterPolynomial(const ZqCenterPolynomial&, NTRU_p);						// It will copy all the ones and negative ones. Everything else will be zero
	ZpCenterPolynomial(NTRU_N _N_, NTRU_p _p_, const char data[], int length);	// Creating polynomial from array of bytes
	~ZpCenterPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}

	ZpCenterPolynomial& operator = (const ZpCenterPolynomial&);
	inline int operator[](int i) const{
		if(i < 0) i = -i;
		if(i > this->N) i %= this->N;
		return this->coefficients[i];
	}
	bool operator == (const ZpCenterPolynomial&) const;
	bool operator == (const ZpPolynomial&) const;
	inline bool operator == (int t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	void printTheDifferences(const ZpPolynomial&, const char* leftName, const char* righName) const;

	ZpCenterPolynomial operator * (ZpCenterPolynomial&) const;
	NTRU_N get_N() const{ return this->N; }
	NTRU_p get_p() const{ return this->p; }
	int degree() const;															// Gets the last nonzero index. If this is the zero polynomial, returns -1
	void toByteArray(char dest[]) const;

	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
};

struct ZqCenterPolynomial {														// Polynomial with coefficients in {q/2-1, ..., q/2}
	private:
	NTRU_N N;
	int* coefficients = NULL;

	class ZqCentered {															// Handling elements in Z3 with centered elements (-1,0,1)
		NTRU_q q;
		int  q_1;																// Necessary for fast modulus q computation. Set as q-1
		int negq_1;																// This will help in the centering process. Set as ~q_1

		public:
		ZqCentered(NTRU_q _q_): q(_q_), q_1(_q_-1), negq_1(~q_1) {}
		int addition(int a, int b) const{
    		return this->mods_q(a+b);
		}
		int mods_q(int a) const{
    		int r, q_div_2 = this->q >> 1;
    		if(a >= 0) {
    	    	r = a & this->q_1;												// Equivalent to a % q since q is a power of 2
    		} else {
    	    	r = -(-a & this->q_1)&this->q_1;								// Computing q - (q - (a % q)) with a fast algorithm
    	    	if(r == 0) return r;											// Possible at least for the multiples of q
    		}																	// Optimization possible with the pre-computing of the negatives in Zq
    		if(r < q_div_2) {													// At this point we know -q < r < q
    	    	if(r > -q_div_2) return r;
    		    else return r&this->q_1;										// Equivalent to r + q
    		} else return r | this->negq_1;										// This is equivalent to r - this->q when r < q
		}

		NTRU_q get_q() const{return this->q;}
	};

	private: ZqCentered _Zq_;

	public:
	ZqCenterPolynomial(const ZqCenterPolynomial& P);
	ZqCenterPolynomial(NTRU_N, NTRU_q);											// Returns the zero polynomial in this ring
	ZqCenterPolynomial(const ZqPolynomial& P);									// Copies and centers a ZqPolynomial
	ZqCenterPolynomial(const ZpCenterPolynomial&, NTRU_q);						// Copies the content of a ZpCenterPolynomial
	ZqCenterPolynomial(const char bytes[], unsigned length, NTRU_N, NTRU_q);	// Building polynomial from array of bytes
	~ZqCenterPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}

	ZqCenterPolynomial& operator = (const ZqCenterPolynomial& P);
	ZqCenterPolynomial& operator = (const ZpCenterPolynomial& P);

	ZqCenterPolynomial operator + (const ZqCenterPolynomial& P) const;
	ZqCenterPolynomial operator * (const ZqCenterPolynomial& P) const;
	ZqCenterPolynomial operator * (const ZpCenterPolynomial& P) const;			// This function sees the ZpCenterPolynomial as a ZqCenterPolynomial

	bool operator == (const ZqCenterPolynomial& P) const;
	inline bool operator == (int t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}

	inline int degree() const{													// Returns degree of polynomial
		int deg = this->N;
		while(this->coefficients[--deg] == 0 && deg > 0) {}
		return deg;
	}
	inline int operator[](int i) const{
		if(i < 0) i = -i;
		if(i > this->N) i %= this->N;
		return this->coefficients[i];
	}
	inline NTRU_N get_N() const { return this->N; }
	inline NTRU_q get_q() const { return this->_Zq_.get_q(); }

	void mods_p(NTRU_p _p_);
	static ZqCenterPolynomial randomTernary(unsigned d, NTRU_N, NTRU_q, bool=false, NTRU_p=_3_);	// Returns polynomial with d amount of 1 and d amounts of -1.
																				// If the bool is true then the return value is "multiplied" by _p_.
	void toBytes(char* dest) const;												// -Writes the coefficients into an array of bytes. If a certain coefficient is
																				//  negative, +=q is applied in order to write a positive number
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
};

bool operator == (const ZqCenterPolynomial&, const ZpCenterPolynomial);			// Polynomial comparison as if their coefficients were in Z rather than in Zp or Zq

}
#endif