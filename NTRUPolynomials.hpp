#include<iostream>

inline static int min(int a, int b) {
	if(a < b) return a;
	return b;
}
inline static int max(int a, int b) {
	if(a < b) return b;
	return a;
}

struct NTRU_ZpPolynomial {															// Representation of the polynomials in Zp[x] (coefficients in integers mod p)
	public:																		// Not necessary at all, just for readability
	enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821 };		// All the possible values for the N
	enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };						// All the possible values for the q
	enum NTRU_p {_3_	= 3 };

	inline static NTRU_N min_N(NTRU_N a, NTRU_N b) {
		if(a < b) return a;
		return b;
	}
	inline static NTRU_N max_N(NTRU_N a, NTRU_N b) {
		if(a < b) return b;
		return a;
	}

	private:
	int* coefficients = NULL;													// Coefficients of the polynomial
	int  degree;
	NTRU_p p = _3_;

	public:
	static const int Z3addition[3][3];											// Addition table of the Z3 ring (integers modulo 3)
	static const int Z3subtraction[3][3];										// Subtraction table of the Z3 ring (integers modulo 3)
	static const int Z3product[3][3];											// Product table of the Z3 ring (integers modulo 3)

	struct ZpPolModXNmns1 {														// Representation of the polynomials in Zp[x]/(x^N-1)
		private:
		NTRU_N N;
		NTRU_p p = _3_;
		int* coefficients = NULL;
		int* permutation  = NULL;
		int* coeffCopy 	  = NULL;												// Copy of the coefficients. Useful at the moment of permute the coefficients

		private:inline ZpPolModXNmns1(): N(_509_) {}								// Initializing N, coefficients is left as NULL

		public:
		inline ZpPolModXNmns1(const ZpPolModXNmns1& P): N(P.N), p(P.p) {
			this->coefficients = new int[P.N];
			for(int i=0; i<P.N; i++)
				this->coefficients[i] = P.coefficients[i];
		}
		ZpPolModXNmns1(NTRU_N _N_,int ones=0,int negOnes=0,NTRU_p _p_=_3_);	// Ones and negOnes will dictate the amount of 1 and -1 respectively
		inline ~ZpPolModXNmns1() {
			if(this->coefficients != NULL) delete [] this->coefficients;
			if(this->permutation  != NULL) delete [] this->permutation;
			if(this->coeffCopy 	  != NULL) delete [] this->coeffCopy;
		}
		// Arithmetic
		ZpPolModXNmns1  operator +  (const ZpPolModXNmns1&) const;			// Addition element by element
		ZpPolModXNmns1  operator -  (const ZpPolModXNmns1&) const;			// Subtraction element by element
		ZpPolModXNmns1  operator *  (const ZpPolModXNmns1&) const;			// Multiplication will coincide with convolution
		ZpPolModXNmns1& operator -= (const ZpPolModXNmns1&);

		inline bool operator == (int t) const {								// Comparison with a single integer
			return this->degree() == 0 && this->coefficients[0] == t;
		}
		inline bool operator != (int t) const {								// Comparison with a single integer
			return this->degree() != 0 || this->coefficients[0] != t;
		}
		inline bool operator == (const ZpPolModXNmns1& P) {
			if(this->N == P.N && this->p == P.p) {							// The comparison this->p == P.p is naive, but maybe will be useful in the future
				for(int i = 0; i < this->N; i++)
					if(this->coefficients[i]!=P.coefficients[i])
						return false;
				return true;
			}
			else return false;
		}
		inline bool operator != (const ZpPolModXNmns1& P) {
			if(this->N == P.N && this->p == P.p) {							// The comparison this->p != P.p is naive, but maybe will be useful in the future
				for(int i = 0; i < this->N; i++)
					if(this->coefficients[i] != P.coefficients[i])
						return true;
				return false;
			}
			else return true;
		}
		inline ZpPolModXNmns1& operator = (const ZpPolModXNmns1& P) {			// Assignment
			if(this != &P) {												// Guarding against self assignment
				if(this->coefficients == NULL) {
					this->coefficients = new int[P.N];
					this->N = P.N;
				} else if(this->N != P.N) {
					delete [] this->coefficients;
					this->coefficients = new int[P.N];
					this->N = P.N;
				}
	    		for(int i = 0; i < P.N; i++)
	    			this->coefficients[i] = P.coefficients[i];
	    	}
			return *this;
		}
		inline ZpPolModXNmns1& operator = (int t) {							// Assignment with single integer
			if(this->coefficients == NULL)
				this->coefficients = new int[this->N];						// Dealing with already initialized object, so this->N is well defined
			if(t < 0) t = -t;
			if(t >= this->p) t %= this->p;
			this->coefficients[0] = t;
	    	for(int i=1; i < this->N; i++) this->coefficients[i] = 0;		// Filling with zeros the rest of the array
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

		inline int degree() const{											// Returns degree of polynomial
			int deg = this->N;
			while(this->coefficients[--deg] == 0 && deg > 0) {}
			return deg;
		}
		inline void copyCoefficients(const NTRU_ZpPolynomial& P) {
			int upperlimit = min(this->N, P.degree), i;
			for(i = 0; i < upperlimit; i++)
				this->coefficients[i] = P.coefficients[i];
		}
		void print(const char* name = "") const;
		inline void println(const char* name = "") const{
			print(name); std::cout<<'\n';
		}
	};
	private: inline NTRU_ZpPolynomial() {this->degree = -1;}

	private: inline NTRU_ZpPolynomial(int _degree_, NTRU_p _p_ = _3_)
	: degree(_degree_),	p(_p_) {												// Initializes with zeros
		int _coeffAmount_  = this->coeffAmount(), i;
		this->coefficients = new int[_coeffAmount_];
		for(i = 0; i < _coeffAmount_; i++)
			this->coefficients[i] = 0;
	}
	public:
	inline NTRU_ZpPolynomial(const NTRU_ZpPolynomial& P): p(P.p), degree(P.degree) {
		int _coeffAmount_  = this->coeffAmount(), i;
		this->coefficients = new int[_coeffAmount_];
		for(i = 0; i < _coeffAmount_; i++)
			this->coefficients[i] = P.coefficients[i];
	}
	inline NTRU_ZpPolynomial(const ZpPolModXNmns1& P): p(P.get_p()),				// Initializing a NTRU_ZpPolynomial with a ZpPolyModXmns1
	degree(P.degree()){
		int _coeffAmount_  = this->coeffAmount(), i;
		this->coefficients = new int[_coeffAmount_];
		for(i = 0; i < _coeffAmount_; i++)
			this->coefficients[i] = P[i];
	}
	inline ~NTRU_ZpPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	inline NTRU_ZpPolynomial& operator = (const NTRU_ZpPolynomial& P) {
		if(this != &P) {													// Guarding against self assignment
			int _coeffAmount_ = P.coeffAmount(), i;
			if(this->degree != P.degree) {
				delete[] this->coefficients;
				this->coefficients = new int[_coeffAmount_];
				this->degree = P.degree;
			}
			this->p = P.p;
			for(i = 0; i < _coeffAmount_; i++)
				this->coefficients[i] = P.coefficients[i];
		}
		return *this;
	}

	inline NTRU_ZpPolynomial operator - (const NTRU_ZpPolynomial& P) const{
		const NTRU_ZpPolynomial *small, *big;
		NTRU_ZpPolynomial r(this->maxDeg(P));                         			// Initializing result with zeros
		int i, smallCoeffAmount, bigCoeffAmount;
		if(this->degree < P.degree) { small = this; big = &P; }         	// 'small' points to the polynomial with the smallest degree, 'big' points to the
		else { small = &P; big = this; }                                	// polynomial with the biggest degree
		smallCoeffAmount = small->coeffAmount();
		bigCoeffAmount   = big->coeffAmount();
    	for(i = 0; i < smallCoeffAmount; i++)
    		r.coefficients[i] =
    		Z3subtraction[this->coefficients[i]][P.coefficients[i]];    	// Subtraction element by element till the smallest degree of the arguments
    	for(; i < bigCoeffAmount; i++)
    		r.coefficients[i] = big->coefficients[i];                   	// Just copying, equivalent to filling with zeros the small polynomial

        	if(small->degree == big->degree) {									// If degree are different, there is a possibility of r[r.degree] == 0
        		while(r.coefficients[r.degree] == 0 && r.degree > 0) {
        			r.degree--;													// Fitting the polynomial to its degree
        			r = NTRU_ZpPolynomial(r);
        		}
        	}
    		return r;
	}

	inline NTRU_ZpPolynomial operator * (const NTRU_ZpPolynomial& P) const{
		NTRU_ZpPolynomial r(this->degree + P.degree);
		int i,j,k;
		for(i = 0; i <= this->degree; i++) {
			if(this->coefficients[i] != 0)
			for(j = 0; j <= P.degree; j++) {
				if(P.coefficients[j] != 0)
				r.coefficients[i+j] = Z3addition[r.coefficients[i+j]]
				[Z3product[this->coefficients[i]][P.coefficients[j]]];
			}
		}
		return r;
	}

	inline int coeffAmount() const {return this->degree + 1;}
	inline int maxDeg(const NTRU_ZpPolynomial& P) const{
		if(this->degree > P.degree) return this->degree;
		return P.degree;
	}
	void division(const NTRU_ZpPolynomial& P,NTRU_ZpPolynomial rslt[2]) const;			// Division assuming the polynomials has its elements in Z3
																			// The quotient and the remainder are saved in rslt
	NTRU_ZpPolynomial gcdXNminus1(NTRU_ZpPolynomial Bezout[2]) const;					// Greatest common divisor between X^N-1 and P. Coefficients in Z3
									 											// Bezout coefficients are polynomials u and v such that u*a + v*b = gcd(a,b)

		//int invModq(int t) const;												// Calculates inverse modulus q

		/*inline int modq(int t) const{											// returns t mod q using bit wise operations. The result will be positive
			while(t < 0) t += q;												// Adding q till we get t positive. This won't affect the result
    		return t &= q-1;													// Since q is a power of 2, the expression t &= q-1 is equivalent to t %= q
		}*/
		//void thisCoeffOddRandom(int deg);										// Assigns a random odd integer between 0 and q to each coefficient till deg
	inline bool operator == (int t) const {								// Comparison with a single integer
		return this->degree == 0 && this->coefficients[0] == t;
	}
};