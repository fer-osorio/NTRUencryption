#include<iostream>

#define DECIMAL_BASE 10

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821 };		// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };						// All the possible values for the q
enum NTRU_p {_3_	= 3 };

struct NTRU_ZpPolynomial {														// Representation of the polynomials in Zp[x]/(x^N-1)
	private:
	NTRU_N N;
	NTRU_p p = _3_;
	int* coefficients = NULL;
	int* permutation  = NULL;												// This could be enhanced by making this attributes static
	int* coeffCopy 	  = NULL;												// Copy of the coefficients. Useful at the moment of permute the coefficients

	public:
	static const int Z3addition[3][3];											// Addition table of the Z3 ring (integers modulo 3)
	static const int Z3subtraction[3][3];										// Subtraction table of the Z3 ring (integers modulo 3)
	static const int Z3product[3][3];											// Product table of the Z3 ring (integers modulo 3)

	private: struct ZpPolynomial {														// Representation of the polynomials in Zp[x] (coefficients in integers mod p)
		private:
		int* coefficients = NULL;												// Coefficients of the polynomial
		NTRU_p p = _3_;
		int  degree;

		private: inline ZpPolynomial(): degree(-1) {}

		private: inline ZpPolynomial(int _degree_, NTRU_p _p_)					// Second argument is used only to avoid ambiguity
		: p(_p_),  degree(_degree_) {											// Initializes with zeros
			int _coeffAmount_  = this->coeffAmount(), i;
			this->coefficients = new int[_coeffAmount_];
			for(i = 0; i < _coeffAmount_; i++)
				this->coefficients[i] = 0;
		}

		public: ZpPolynomial(int zeros, int ones, int twos);					// Creates a polynomial with the amount of zeros (+1), ones and twos specified

		inline ZpPolynomial(const ZpPolynomial& P): p(P.p),
		degree(P.degree) {
			int _coeffAmount_  = this->coeffAmount(), i;
			this->coefficients = new int[_coeffAmount_];
			for(i = 0; i < _coeffAmount_; i++)
				this->coefficients[i] = P.coefficients[i];
		}
		inline ZpPolynomial(const NTRU_ZpPolynomial& P): p(P.get_p()),			// Initializing a ZpPolynomial with a ZpPolyModXmns1
		degree(P.degree()){
			int _coeffAmount_  = this->coeffAmount(), i;
			this->coefficients = new int[_coeffAmount_];
			for(i = 0; i < _coeffAmount_; i++)
				this->coefficients[i] = P[i];
		}
		inline ~ZpPolynomial() {
			if(this->coefficients != NULL) delete[] this->coefficients;
		}
		ZpPolynomial& operator = (const ZpPolynomial& P);					// Copy assignment

		inline ZpPolynomial& operator = (int t) {								// Assignment with single integer
			if(this->coefficients == NULL)
				this->coefficients = new int[1];									// Dealing with already initialized object, so this->N is well defined
			if(t < 0) t = -t;
			if(t >= this->p) t %= this->p;
			this->coefficients[0] = t;
			this->degree = 0;
			return *this;
		}

		ZpPolynomial operator + (const ZpPolynomial& P) const;
		ZpPolynomial operator - (const ZpPolynomial& P) const;
		inline ZpPolynomial operator - () const{
			ZpPolynomial r = *this;
			for(int i = 0; i <= r.degree; i++)
				r.coefficients[i] = Z3subtraction[0][r.coefficients[i]];
			return r;
		}
		ZpPolynomial operator * (const ZpPolynomial& P) const;					// Classical multiplication (compare with convolution)

		inline bool operator == (const ZpPolynomial& P) {
			if(this->degree == P.degree && this->p == P.p) {					// The comparison this->p == P.p is naive, but maybe will be useful in the future
				for(int i = 0; i <= this->degree; i++)
					if(this->coefficients[i] != P.coefficients[i])
						return false;
				return true;
			}
			else return false;
		}
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
			while(this->coefficients[this->degree] == 0 && this->degree > 0) {
    	    	this->degree--;														// Fitting the polynomial to its degree
    	    }
		}
		inline NTRU_p get_p() const{ return this->p; }
		void print(const char* name = "") const;
		inline void println(const char* name = "") const{
			print(name); std::cout<<'\n';
		}
	};

	private: inline NTRU_ZpPolynomial(): N(_509_) {}								// Initializing N, coefficients is left as NULL

	public:
	inline NTRU_ZpPolynomial(const NTRU_ZpPolynomial& P): N(P.N), p(P.p) {
		this->coefficients = new int[P.N];
		for(int i=0; i<P.N; i++)
			this->coefficients[i] = P.coefficients[i];
	}
	NTRU_ZpPolynomial(const ZpPolynomial& P, NTRU_N _N_);
	NTRU_ZpPolynomial(NTRU_N _N_, int ones=0, int twos=0, NTRU_p _p_=_3_);		// Ones and twos will dictate the amount of 1 and 2 respectively
	inline ~NTRU_ZpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
		if(this->permutation  != NULL) delete [] this->permutation;
		if(this->coeffCopy 	  != NULL) delete [] this->coeffCopy;
	}
	// Arithmetic
	NTRU_ZpPolynomial  operator + (const NTRU_ZpPolynomial&) const;				// Addition element by element
	NTRU_ZpPolynomial  operator - (const NTRU_ZpPolynomial&) const;				// Subtraction element by element
	NTRU_ZpPolynomial  operator * (const NTRU_ZpPolynomial&) const;				// Multiplication will coincide with convolution (compare with classical)
	NTRU_ZpPolynomial& operator-= (const NTRU_ZpPolynomial&);

	void division(const NTRU_ZpPolynomial& P,NTRU_ZpPolynomial rslt[2]) const;			// The quotient and the remainder are saved in rslt
	NTRU_ZpPolynomial gcdXNmns1(NTRU_ZpPolynomial& thisBezout) const;

	inline NTRU_ZpPolynomial operator - () const{
		NTRU_ZpPolynomial r = *this;
		int i, _degree_ = this->degree();
		for(i = 0; i <= _degree_; i++)
			r.coefficients[i] = Z3subtraction[0][r.coefficients[i]];
		return r;
	}

	inline bool operator == (int t) const {										// Comparison with a single integer
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	inline bool operator != (int t) const {										// Comparison with a single integer
		return this->degree() != 0 || this->coefficients[0] != t;
	}
	inline bool operator == (const NTRU_ZpPolynomial& P) {
		if(this->N == P.N && this->p == P.p) {									// The comparison this->p == P.p is naive, but maybe will be useful in the future
			for(int i = 0; i < this->N; i++)
				if(this->coefficients[i]!=P.coefficients[i])
					return false;
			return true;
		}
		else return false;
	}
	inline bool operator != (const NTRU_ZpPolynomial& P) {
		if(this->N == P.N && this->p == P.p) {									// The comparison this->p != P.p is naive, but maybe will be useful in the future
			for(int i = 0; i < this->N; i++)
				if(this->coefficients[i] != P.coefficients[i])
					return true;
			return false;
		}
		else return true;
	}
	NTRU_ZpPolynomial& operator = (const NTRU_ZpPolynomial& P);					// Assignment

	NTRU_ZpPolynomial& operator = (const ZpPolynomial& P);				// Assignment from ZpPolynomial to NTRU_ZpPolynomial

	inline NTRU_ZpPolynomial& operator = (int t) {								// Assignment with single integer
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
	void print(const char* name = "") const;
	inline void println(const char* name = "") const{
		print(name); std::cout<<'\n';
	}
	void test(NTRU_N _N_, int d);
};

struct NTRU_ZqPolynomial {														// Representation of the polynomials in Zp[x]/(x^N-1)
	private: struct Z2Polynomial {												// Representation of the polynomials in Z2[x]/(x^N-1)
		private: enum Z2 {_0_ = 0, _1_ = 1};									// Integers modulo 2 (binary numbers)
		inline friend Z2 operator - (Z2 a,Z2 b){								// Subtraction modulus 2 (coincide with addition)
			if(a!=b) return _1_;
			return _0_;
		}
		inline friend Z2 operator * (Z2 a,Z2 b){								// Multiplication modulus 2
			if(a==0) return _0_;
			return  b ;
		}
		inline friend void operator += (Z2& a, Z2 b) {
			if(a != b) a = _1_;
			a = _0_;
		}
		inline friend void operator -= (Z2& a, Z2 b) {							// Addition and subtraction coincide, I do this just to evade confusion with
			if(a != b) a = _1_;													// notation
			a = _0_;
		}
		private:
		NTRU_N N;
		Z2* coefficients = NULL;

		private: inline Z2Polynomial(): N(_509_) {}
		private: inline Z2Polynomial(NTRU_N _N_): N(_N_) {
			this->coefficients = new Z2[_N_];
			for(int i = 0; i < _N_; i++) this->coefficients[i] = _0_;
		}

		public:
		inline Z2Polynomial(const NTRU_ZpPolynomial& P): N(P.get_N()) {
			this->coefficients = new Z2[this->N];
			for(int i = 0; i < this->N; i++) {
				if(P[i] == 0 || P[i] == 2) this->coefficients[i] = _0_;
				else this->coefficients[i] = _1_;
			}
		}
		inline ~Z2Polynomial() {
			delete[] this->coefficients;
		}
		inline Z2Polynomial& operator = (const Z2Polynomial& P) {
			if(this != &P) {													// Guarding against self assignment
				if(this->N != P.N) {											// Delete pass coefficients array. If this->N != P.N is true, there is no reason
					delete[] this->coefficients;								// to delete the array
					this->coefficients = new Z2[P.N];
					this->N = P.N;
				}
				for(int i = 0; i < this->N; i++)
					this->coefficients[i] = P.coefficients[i];
			}
			return *this;
		}
		inline Z2Polynomial& operator = (Z2 t) {
			if(this->coefficients==NULL) this->coefficients = new Z2[this->N];	// In case of have been generated from the (private) default constructor
			this->coefficients[0] = t;
			for(int i = 1; i < this->N; i++) this->coefficients[i] = _0_;
			return *this;
		}
		Z2Polynomial operator - (const Z2Polynomial&) const;					// In Z2, addition (+) coincide with subtraction (-)
		Z2Polynomial operator *	(const Z2Polynomial&) const;
		void division(const Z2Polynomial& P,Z2Polynomial res[2]) const;			// Division between this and P, result[2] will save the res[2]
		Z2Polynomial gcdXNmns1(Z2Polynomial& thisBezout) const;			// Greatest common between this and x^N-1 polynomial. Writing
																				// gcd = u·(x^N - 1) + v·this, thisBezout == v
		inline bool operator == (Z2 t) const {
			return this->degree() == 0 && this->coefficients[0] == t;
		}
		inline bool operator != (Z2 t) const {
			return this->degree() != 0 || this->coefficients[0] != t;
		}

		inline int degree() const{												// Returns degree of polynomial
			int deg = this->N;
			while(this->coefficients[--deg] == _0_ && deg > 0) {}
			return deg;
		}
	};

	private:
	NTRU_N N;
	NTRU_q q;
	int* coefficients = NULL;

	private: inline NTRU_ZqPolynomial(): N(_509_), q(_2048_) {}

	public:
	inline NTRU_ZqPolynomial(const NTRU_ZpPolynomial& P,
	NTRU_q _q_): N(P.get_N()), q(_q_) {
		this->coefficients = new int[this->N];
		for(int i = 0; i < this->N; i++) this->coefficients[i] = P[i];
	}
	inline ~NTRU_ZqPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	inline NTRU_ZqPolynomial& operator = (const NTRU_ZqPolynomial& P) {
		if(this != &P) {														// Guarding against self assignment
			if(this->N != P.N) {												// Delete pass coefficients array. If this->N != P.N is true, there is no reason
				delete[] this->coefficients;									// to delete the array
				this->coefficients = new int[P.N];
				this->N = P.N;
			}
			this->q = P.q;
			for(int i = 0; i < this->N; i++)
				this->coefficients[i] = P.coefficients[i];
		}
		return *this;
	}

	NTRU_ZqPolynomial operator * (const NTRU_ZqPolynomial& P) const;
	NTRU_ZqPolynomial operator - (const NTRU_ZqPolynomial& P) const;
	NTRU_ZqPolynomial gcdXNmns1_Z2(const NTRU_ZqPolynomial& P) const;			// Greatest common divisor seeing the coefficients in Z2 (integers modulus 2)
};