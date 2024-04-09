#include<iostream>

#ifndef _NTRUPOLYNOMIALS_HPP_
#define _NTRUPOLYNOMIALS_HPP_

#define DECIMAL_BASE 10

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821 };		// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };						// All the possible values for the q
enum NTRU_p {_3_	= 3 };

enum Z2 {_0_ = 0, _1_ = 1};														// Integers modulo 2 (binary numbers)

inline Z2 operator + (Z2 a,Z2 b) {												// Addition modulus 2
	if(a!=b) return _1_;
	return _0_;
}
inline Z2 operator - (Z2 a,Z2 b) {												// Addition and subtraction coincide in Z2. This is just for evade problems
	if(a!=b) return _1_;														// with notation
	return _0_;
}
inline Z2 operator * (Z2 a, Z2 b) {												// Multiplication modulus 2
	if(a==0) return _0_;
	return  b ;
}
inline void operator += (Z2& a, Z2 b) {
	if(a != b) a = _1_;
	else a = _0_;
}
inline void operator -= (Z2& a, Z2 b) {
	if(a != b) a = _1_;
	else a = _0_;
}

class Zq{																		// Representation of the group of integers modulus q: Zq
	NTRU_q q;
	int  q_1;

	public: Zq(NTRU_q _q_): q(_q_), q_1(_q_-1) {}
	public: inline NTRU_q get_q() const{ return q; }

	int mod_q(int t) const{														// operation t % q
		while(t < 0) t += this->q;												// Ensuring a positive t
		return t & this->q_1;													// Since q is a power of two, t & (q-1) is equivalent to t%q, but much faster
	}

	int add(int a, int b) const{
		int r = a + b;
		while(r < 0) r += this->q;
		return r & this->q_1;
	}
	int subtract(int a, int b) const{
		int r = a - b;
		while(r < 0) r += this->q;
		return r & this->q_1;
	}
	int product(int a, int b) const{
		int r = a * b;
		while(r < 0) r += this->q;
		return r & this->q_1;
	}
	int negative(int a) const{
		while(a < 0) a += this->q;
		a &= this->q_1;
		return this->q - a;
	}
};

struct NTRU_ZpPolynomial {														// Representation of the polynomials in Zp[x]/(x^N-1)
	private:
	NTRU_N N;
	NTRU_p p = _3_;
	int* coefficients = NULL;
	int* permutation  = NULL;													// This could be enhanced by making this attributes static
	int* coeffCopy 	  = NULL;													// Copy of the coefficients. Useful at the moment of permute the coefficients

	public:
	static const int Z3addition[3][3];											// Addition table of the Z3 ring (integers modulo 3)
	static const int Z3subtraction[3][3];										// Subtraction table of the Z3 ring (integers modulo 3)
	static const int Z3product[3][3];											// Product table of the Z3 ring (integers modulo 3)

	private: struct ZpPolynomial {												// Representation of the polynomials in Zp[x] (coefficients in integers mod p)
		private:
		int* coefficients = NULL;												// Coefficients of the polynomial
		NTRU_p p = _3_;
		int  degree;

		private: ZpPolynomial(): degree(-1) {}
		private: ZpPolynomial(int _degree_, NTRU_p _p_);
		public:	 ZpPolynomial(int zeros, int ones, int twos);					// Creates a polynomial with the amount of zeros (+1), ones and twos specified

		ZpPolynomial(const ZpPolynomial& P);
		ZpPolynomial(const NTRU_ZpPolynomial& P);								// Initializing a ZpPolynomial with a ZpPolyModXmns1
		~ZpPolynomial() {
			if(this->coefficients != NULL) delete[] this->coefficients;
		}

		inline NTRU_p get_p() const{ return this->p; }

		ZpPolynomial& operator = (const ZpPolynomial& P);						// Copy assignment
		ZpPolynomial& operator = (int);											// Assignment with single integer

		ZpPolynomial operator + (const ZpPolynomial& P) const;					// Arithmetic
		ZpPolynomial operator - (const ZpPolynomial& P) const;					// ...
		ZpPolynomial operator - () const;										// ...
		ZpPolynomial operator * (const ZpPolynomial& P) const;					// Classical multiplication (compare with convolution)

		bool operator == (const ZpPolynomial& P);
		inline bool operator != (int t) const {									// Comparison with a single integer
			return this->degree != 0 || this->coefficients[0] != t;
		}
		inline int operator [] (int i) const {
			if(i < 0) i = -i;
			if(i > degree) i %= degree;
			return this->coefficients[i];
		}

		inline int coeffAmount() const {return this->degree + 1;}

		inline bool operator == (int t) const {									// Comparison with a single integer
			return this->degree == 0 && this->coefficients[0] == t;
		}
		inline void correctDegree() {
			while(this->coefficients[this->degree] == 0 && this->degree > 0)
    	    this->degree--;														// Fitting the polynomial to its degree
		}

		void print(const char* name, const char* tail) const;
		void println(const char* name = "") const;
	};

	private: NTRU_ZpPolynomial(): N(_509_) {}									// Initializing N, coefficients is left as NULL

	public:
	NTRU_ZpPolynomial(const NTRU_ZpPolynomial& P);
	NTRU_ZpPolynomial(const ZpPolynomial& P, NTRU_N _N_);
	NTRU_ZpPolynomial(NTRU_N _N_, int ones=0, int twos=0, NTRU_p _p_=_3_);		// Ones and twos will dictate the amount of 1 and 2 respectively
	~NTRU_ZpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
		if(this->permutation  != NULL) delete [] this->permutation;
		if(this->coeffCopy 	  != NULL) delete [] this->coeffCopy;
	}
	// Arithmetic
	NTRU_ZpPolynomial  operator + (const NTRU_ZpPolynomial&) const;				// Addition element by element
	NTRU_ZpPolynomial  operator - (const NTRU_ZpPolynomial&) const;				// Subtraction element by element
	NTRU_ZpPolynomial  operator - () const;
	NTRU_ZpPolynomial  operator * (const NTRU_ZpPolynomial&) const;				// Multiplication will coincide with convolution (compare with classical)
	NTRU_ZpPolynomial& operator-= (const NTRU_ZpPolynomial&);

	void division(const NTRU_ZpPolynomial& P,NTRU_ZpPolynomial rslt[2]) const;	// The quotient and the remainder are saved in rslt
	NTRU_ZpPolynomial gcdXNmns1(NTRU_ZpPolynomial& thisBezout) const;

	inline bool operator == (int t) const {										// Comparison with a single integer
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	inline bool operator != (int t) const {										// Comparison with a single integer
		return this->degree() != 0 || this->coefficients[0] != t;
	}
	bool operator == (const NTRU_ZpPolynomial& P) const;
	bool operator != (const NTRU_ZpPolynomial& P) const;

	NTRU_ZpPolynomial& operator = (const NTRU_ZpPolynomial& P);					// Assignment
	NTRU_ZpPolynomial& operator = (const ZpPolynomial& P);						// Assignment from ZpPolynomial to NTRU_ZpPolynomial
	NTRU_ZpPolynomial& operator = (int t);										// Assignment with single integer

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

struct NTRU_ZqPolynomial {														// Representation of the polynomials in Zp[x]/(x^N-1)
	private:
	NTRU_N N;
	Zq _Zq_;
	int* coefficients = NULL;

	public: struct Z2Polynomial {												// Representation of the polynomials in Z2[x]/(x^N-1)
		private:
		NTRU_N N;
		Z2* coefficients = NULL;

		private: Z2Polynomial(): N(_509_) {}
		private: Z2Polynomial(NTRU_N _N_): N(_N_) {
			this->coefficients = new Z2[_N_];
			for(int i = 0; i < _N_; i++) this->coefficients[i] = _0_;
		}

		public:
		Z2Polynomial(const Z2Polynomial& P);
		Z2Polynomial(const NTRU_ZpPolynomial& P);
		Z2Polynomial(NTRU_N _N_, int ones);
		~Z2Polynomial() {
			if(this->coefficients != NULL) delete[] this->coefficients;
		}

		inline NTRU_N get_N() const{ return this->N; }

		Z2Polynomial& operator = (const Z2Polynomial& P);
		Z2Polynomial& operator = (const NTRU_ZpPolynomial& P);
		Z2Polynomial& operator = (Z2 t);
		Z2Polynomial operator + (const Z2Polynomial&) const;					// In Z2, addition (+) coincide with subtraction (-)
		Z2Polynomial operator - (const Z2Polynomial&) const;					// In Z2, addition (+) coincide with subtraction (-)
		Z2Polynomial operator *	(const Z2Polynomial&) const;
		void division(const Z2Polynomial& P,Z2Polynomial res[2]) const;			// Division between this and P, result[2] will save the res[2]
		Z2Polynomial gcdXNmns1(Z2Polynomial& thisBezout) const;					// Greatest common between this and x^N-1 polynomial. Writing
																				// gcd = u·(x^N - 1) + v·this, thisBezout == v
		inline bool operator == (Z2 t) const {
			return this->degree() == 0 && this->coefficients[0] == t;
		}
		bool operator == (const Z2Polynomial& P) const;
		inline bool operator != (Z2 t) const {
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

	private:NTRU_ZqPolynomial(): N(_509_), _Zq_(_2048_) {}

	public:
	NTRU_ZqPolynomial(NTRU_N, NTRU_q);
	NTRU_ZqPolynomial(const NTRU_ZpPolynomial& P,NTRU_q _q_);
	NTRU_ZqPolynomial(const NTRU_ZqPolynomial& P);
	NTRU_ZqPolynomial(const Z2Polynomial& P,NTRU_q _q_);
	~NTRU_ZqPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	NTRU_ZqPolynomial& operator = (const NTRU_ZqPolynomial& P);
	NTRU_ZqPolynomial& operator = (const NTRU_ZpPolynomial& P);

	inline int operator[](int i) const{
		if(i < 0) i = -i;
		if(i > this->N) i %= this->N;
		return this->coefficients[i];
	}
	inline NTRU_N get_N() const{ return this->N; }
	inline NTRU_q get_q() const{ return this->_Zq_.get_q(); }

	NTRU_ZqPolynomial operator - (const NTRU_ZqPolynomial& P) const;
	NTRU_ZqPolynomial operator * (const NTRU_ZqPolynomial& P) const;

	friend NTRU_ZqPolynomial operator - (int, const NTRU_ZqPolynomial& P);

	inline int degree() const{													// Returns degree of polynomial
		int deg = this->N;
		while(this->coefficients[--deg] == 0 && deg > 0) {}
		return deg;
	}
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
};

struct CenteredPolynomial {
	private:
	NTRU_N N;
	int* coefficients = NULL;

	class ZpCentered {															// Handling elements in Z3 with centered elements (-1,0,1)
		NTRU_p p;
		public: static const int Z3[3];
		private:
		static const int Z3add [3][3];											// Centered addition in Z3
		static const int Z3subs[3][3];											// Centered subtraction in Z3

		public:
		ZpCentered(NTRU_p _p_): p(_p_) {}
		int addition(int a, int b)	const;										// Supposing a,b are in the set {-p/2 - 1,...,0,..,p/2 - 1}
		int subtraction(int a, int b)const;										// Supposing a,b are in the set {-p/2 - 1,...,0,..,p/2 - 1}
		int product(int a, int b) 	const;										// Supposing a,b are in the set {-p/2 - 1,...,0,..,p/2 - 1}

		NTRU_p get_p() const{return this->p;}
	};

	class ZqCentered {															// Handling elements in Z3 with centered elements (-1,0,1)
		NTRU_q q;
		int  q_1;
		int negq_1;																// This will help in the centering process

		public:
		ZqCentered(NTRU_q _q_): q(_q_), q_1(_q_-1), negq_1(~q_1) {}
		int addition(int a, int b)	const;										// Supposing a,b are in the set {-p/2 - 1,...,0,..,p/2 - 1}
		int subtraction(int a, int b)const;										// Supposing a,b are in the set {-p/2 - 1,...,0,..,p/2 - 1}
		int mods_q(int a) const;

		NTRU_q get_q() const{return this->q;}
	};

	private: ZpCentered _Zp_;
	private: ZqCentered _Zq_;

	public:
	CenteredPolynomial(NTRU_N, NTRU_p, NTRU_q);
	CenteredPolynomial(const NTRU_ZpPolynomial& P, NTRU_q _q_);
	CenteredPolynomial(const NTRU_ZqPolynomial& P, NTRU_p _p_);
	~CenteredPolynomial();

	CenteredPolynomial operator*(const CenteredPolynomial& P) const;

	inline int degree() const{													// Returns degree of polynomial
		int deg = this->N;
		while(this->coefficients[--deg] == 0 && deg > 0) {}
		return deg;
	}
	inline NTRU_p get_p() const { return this->_Zp_.get_p(); }
	inline NTRU_q get_q() const { return this->_Zq_.get_q(); }
};

#endif