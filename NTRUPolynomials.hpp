#include<iostream>

#define DECIMAL_BASE 10

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821 };		// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };						// All the possible values for the q
enum NTRU_p {_3_	= 3 };

struct NTRU_ZpPolynomial {														// Representation of the polynomials in Zp[x] (coefficients in integers mod p)
	private:
	int* coefficients = NULL;													// Coefficients of the polynomial
	NTRU_p p = _3_;
	int  degree;

	public:
	static const int Z3addition[3][3];											// Addition table of the Z3 ring (integers modulo 3)
	static const int Z3subtraction[3][3];										// Subtraction table of the Z3 ring (integers modulo 3)
	static const int Z3product[3][3];											// Product table of the Z3 ring (integers modulo 3)

	struct PolyModXNmns1 {														// Representation of the polynomials in Zp[x]/(x^N-1)
		private:
		NTRU_N N;
		NTRU_p p = _3_;
		int* coefficients = NULL;
		int* permutation  = NULL;												// This could be enhanced by making this attributes static
		int* coeffCopy 	  = NULL;												// Copy of the coefficients. Useful at the moment of permute the coefficients

		private:inline PolyModXNmns1(): N(_509_) {}								// Initializing N, coefficients is left as NULL

		public:
		inline PolyModXNmns1(const PolyModXNmns1& P): N(P.N), p(P.p) {
			this->coefficients = new int[P.N];
			for(int i=0; i<P.N; i++)
				this->coefficients[i] = P.coefficients[i];
		}
		PolyModXNmns1(const NTRU_ZpPolynomial& P, NTRU_N _N_);
		PolyModXNmns1(NTRU_N _N_, int ones=0, int twos=0, NTRU_p _p_=_3_);		// Ones and twos will dictate the amount of 1 and 2 respectively

		inline ~PolyModXNmns1() {
			if(this->coefficients != NULL) delete [] this->coefficients;
			if(this->permutation  != NULL) delete [] this->permutation;
			if(this->coeffCopy 	  != NULL) delete [] this->coeffCopy;
		}
		// Arithmetic
		PolyModXNmns1  operator + (const PolyModXNmns1&) const;					// Addition element by element
		PolyModXNmns1  operator - (const PolyModXNmns1&) const;					// Subtraction element by element
		PolyModXNmns1  operator * (const PolyModXNmns1&) const;					// Multiplication will coincide with convolution (compare with classical)
		PolyModXNmns1& operator-= (const PolyModXNmns1&);

		NTRU_ZpPolynomial gcdXNmns1(PolyModXNmns1& thisBezout) const;

		inline bool operator == (int t) const {									// Comparison with a single integer
			return this->degree() == 0 && this->coefficients[0] == t;
		}
		inline bool operator != (int t) const {									// Comparison with a single integer
			return this->degree() != 0 || this->coefficients[0] != t;
		}
		inline bool operator == (const PolyModXNmns1& P) {
			if(this->N == P.N && this->p == P.p) {								// The comparison this->p == P.p is naive, but maybe will be useful in the future
				for(int i = 0; i < this->N; i++)
					if(this->coefficients[i]!=P.coefficients[i])
						return false;
				return true;
			}
			else return false;
		}
		inline bool operator != (const PolyModXNmns1& P) {
			if(this->N == P.N && this->p == P.p) {								// The comparison this->p != P.p is naive, but maybe will be useful in the future
				for(int i = 0; i < this->N; i++)
					if(this->coefficients[i] != P.coefficients[i])
						return true;
				return false;
			}
			else return true;
		}
		PolyModXNmns1& operator = (const PolyModXNmns1& P);					// Assignment

		PolyModXNmns1& operator = (const NTRU_ZpPolynomial& P);				// Assignment from NTRU_ZpPolynomial to PolyModXNmns1

		inline PolyModXNmns1& operator = (int t) {								// Assignment with single integer
			if(this->coefficients == NULL)
				this->coefficients = new int[this->N];							// Dealing with already initialized object, so this->N is well defined
			if(t < 0) t = -t;
			if(t >= this->p) t %= this->p;
			this->coefficients[0] = t;
	    	for(int i=1; i < this->N; i++) this->coefficients[i] = 0;			// Filling with zeros the rest of the array
			return *this;
		}
		inline int operator[](int i) const{
			if(i < 0) i = -i;
			if(i > this->N) i %= this->N;
			return this->coefficients[i];
		}

		inline NTRU_N get_N() const{ return this->N; }
		inline NTRU_p get_p() const{ return this->p; }

		void setPermutation();
		void permute();

		inline int degree() const{												// Returns degree of polynomial
			int deg = this->N;
			while(this->coefficients[--deg] == 0 && deg > 0) {}
			return deg;
		}
		/*inline void copyCoefficients(const NTRU_ZpPolynomial& P) {
			int upperlimit = min(this->N, P.degree), i;
			for(i = 0; i < upperlimit; i++)
				this->coefficients[i] = P.coefficients[i];
		}*/
		void print(const char* name = "") const;
		inline void println(const char* name = "") const{
			print(name); std::cout<<'\n';
		}
		void test(int d);
	};

	private: inline NTRU_ZpPolynomial(): degree(-1) {}

	private: inline NTRU_ZpPolynomial(int _degree_, NTRU_p _p_)					// Second argument is used only to avoid ambiguity
	: p(_p_),  degree(_degree_) {												// Initializes with zeros
		int _coeffAmount_  = this->coeffAmount(), i;
		this->coefficients = new int[_coeffAmount_];
		for(i = 0; i < _coeffAmount_; i++)
			this->coefficients[i] = 0;
	}

	public:
	NTRU_ZpPolynomial(int zeros, int ones, int twos);							// Creates a polynomial with the amount of zeros (+1), ones and twos specified

	inline NTRU_ZpPolynomial(const NTRU_ZpPolynomial& P): p(P.p),
	degree(P.degree) {
		int _coeffAmount_  = this->coeffAmount(), i;
		this->coefficients = new int[_coeffAmount_];
		for(i = 0; i < _coeffAmount_; i++)
			this->coefficients[i] = P.coefficients[i];
	}
	inline NTRU_ZpPolynomial(const PolyModXNmns1& P): p(P.get_p()),				// Initializing a NTRU_ZpPolynomial with a ZpPolyModXmns1
	degree(P.degree()){
		int _coeffAmount_  = this->coeffAmount(), i;
		this->coefficients = new int[_coeffAmount_];
		for(i = 0; i < _coeffAmount_; i++)
			this->coefficients[i] = P[i];
	}
	inline ~NTRU_ZpPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	NTRU_ZpPolynomial& operator = (const NTRU_ZpPolynomial& P);					// Copy assignment

	inline NTRU_ZpPolynomial& operator = (int t) {								// Assignment with single integer
		if(this->coefficients == NULL)
			this->coefficients = new int[1];									// Dealing with already initialized object, so this->N is well defined
		if(t < 0) t = -t;
		if(t >= this->p) t %= this->p;
		this->coefficients[0] = t;
		this->degree = 0;
		return *this;
	}

	NTRU_ZpPolynomial operator + (const NTRU_ZpPolynomial& P) const;
	NTRU_ZpPolynomial operator - (const NTRU_ZpPolynomial& P) const;

	inline NTRU_ZpPolynomial operator - () const{
		NTRU_ZpPolynomial r = *this;
		for(int i = 0; i <= r.degree; i++)
			r.coefficients[i] = Z3subtraction[0][r.coefficients[i]];
		return r;
	}
	NTRU_ZpPolynomial operator * (const NTRU_ZpPolynomial& P) const;			// Classical multiplication (compare with convolution)

	inline bool operator == (const NTRU_ZpPolynomial& P) {
		if(this->degree == P.degree && this->p == P.p) {						// The comparison this->p == P.p is naive, but maybe will be useful in the future
			for(int i = 0; i <= this->degree; i++)
				if(this->coefficients[i] != P.coefficients[i])
					return false;
			return true;
		}
		else return false;
	}
	inline bool operator != (int t) const {										// Comparison with a single integer
		return this->degree != 0 || this->coefficients[0] != t;
	}

	inline int coeffAmount() const {return this->degree + 1;}

	void division(const NTRU_ZpPolynomial& P,NTRU_ZpPolynomial rslt[2]) const;	// The quotient and the remainder are saved in rslt
	inline bool operator == (int t) const {										// Comparison with a single integer
		return this->degree == 0 && this->coefficients[0] == t;
	}
	inline void correctDegree() {
		while(this->coefficients[this->degree] == 0 && this->degree > 0) {
        	this->degree--;														// Fitting the polynomial to its degree
        }
	}
	void print(const char* name = "") const;
	inline void println(const char* name = "") const{
		print(name); std::cout<<'\n';
	}
};

struct NTRU_ZqPolyModXNmns1 {													// Representation of the polynomials in Zp[x]/(x^N-1)
	private: struct Z2PolyModXNmns1{
		private: enum Z2 {_0_ = 0, _1_ = 1};									// Integers modulo 2 (binary numbers)
		inline friend Z2 operator + (Z2 a,Z2 b){									// Addition modulus 2 (coincide with subtraction)
			if(a!=b) return _1_;
			return _0_;
		}
		inline friend Z2 operator * (Z2 a,Z2 b){									// Multiplication modulus 2
			if(a==0) return _0_;
			return  b ;
		}
		private:
		NTRU_N N;
		Z2* coefficients = NULL;

		private: inline Z2PolyModXNmns1(): N(_509_) {}

		public:
		inline Z2PolyModXNmns1(const NTRU_ZpPolynomial::PolyModXNmns1& P):
		N(P.get_N()) {
			this->coefficients = new Z2[this->N];
			for(int i = 0; i < this->N; i++) {
				if(P[i] == 0 || P[i] == 2) this->coefficients[i] = _0_;
				else this->coefficients[i] = _1_;
			}
		}
		Z2PolyModXNmns1 gcdXNmns1(const Z2PolyModXNmns1&) const;
	};

	private:
	NTRU_N N;
	NTRU_q q;
	int* coefficients = NULL;

	private: inline NTRU_ZqPolyModXNmns1(): N(_509_), q(_2048_) {}

	public:
	inline NTRU_ZqPolyModXNmns1(const NTRU_ZpPolynomial::PolyModXNmns1& P,
	NTRU_q _q_): N(P.get_N()), q(_q_) {
		this->coefficients = new int[this->N];
		for(int i = 0; i < this->N; i++) this->coefficients[i] = P[i];
	}
	inline ~NTRU_ZqPolyModXNmns1() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}

	NTRU_ZqPolyModXNmns1 operator * (const NTRU_ZqPolyModXNmns1& P) const;
	NTRU_ZqPolyModXNmns1 operator - (const NTRU_ZqPolyModXNmns1& P) const;
	NTRU_ZqPolyModXNmns1 gcdXNmns1_Z2(const NTRU_ZqPolyModXNmns1& P) const;		// Greatest common divisor seeing the coefficients in Z2 (integers modulus 2)
};