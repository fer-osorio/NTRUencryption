#include<iostream>

#define DECIMAL_BASE 10

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821 };		// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };						// All the possible values for the q
enum NTRU_p {_3_	= 3 };

inline static int min(int a, int b) {
	if(a < b) return a;
	return b;
}
inline static int max(int a, int b) {
	if(a < b) return b;
	return a;
}

struct NTRU_ZpPolynomial {															// Representation of the polynomials in Zp[x] (coefficients in integers mod p)
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

		ZpPolModXNmns1(NTRU_N _N_, int ones=0, int negOnes=0, NTRU_p _p_=_3_);	// Ones and negOnes will dictate the amount of 1 and -1 respectively

		inline ~ZpPolModXNmns1() {
			if(this->coefficients != NULL) delete [] this->coefficients;
			if(this->permutation  != NULL) delete [] this->permutation;
			if(this->coeffCopy 	  != NULL) delete [] this->coeffCopy;
		}
		// Arithmetic
		ZpPolModXNmns1  operator + (const ZpPolModXNmns1&) const;			// Addition element by element
		ZpPolModXNmns1  operator - (const ZpPolModXNmns1&) const;			// Subtraction element by element
		ZpPolModXNmns1  operator * (const ZpPolModXNmns1&) const;			// Multiplication will coincide with convolution
		ZpPolModXNmns1& operator-= (const ZpPolModXNmns1&);

		NTRU_ZpPolynomial gcdXNmns1(ZpPolModXNmns1& thisBezout) const;

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
		inline ZpPolModXNmns1& operator = (const NTRU_ZpPolynomial& P) {		// Assignment from NTRU_ZpPolynomial to ZpPolModXNmns1
			int upperLimit = min(this->N, P.coeffAmount()), i;
			if(this->coefficients == NULL)
				this->coefficients = new int[this->N];
	    	for(i = 0; i < upperLimit; i++)
	    		this->coefficients[i] = P.coefficients[i];
	    	if(upperLimit < this->N)
	    		for(; i < this->N; i++)
	    			this->coefficients[i] = 0;
	    	else for(; i < upperLimit; i++)
	    		this->coefficients[i%this->N] =
	    		Z3addition[this->coefficients[i%this->N]][P.coefficients[i]];
	    	this->p = P.p;
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

	/*private:*/public: inline NTRU_ZpPolynomial(int _degree_, NTRU_p _p_ = _3_)
	: degree(_degree_),	p(_p_) {												// Initializes with zeros
		int _coeffAmount_  = this->coeffAmount(), i;
		this->coefficients = new int[_coeffAmount_];
		for(i = 0; i < _coeffAmount_; i++)
			this->coefficients[i] = 0;
	}
	public:
	inline NTRU_ZpPolynomial(const NTRU_ZpPolynomial& P): p(P.p),
	degree(P.degree) {
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
	inline NTRU_ZpPolynomial& operator = (int t) {							// Assignment with single integer
		if(this->coefficients == NULL)
			this->coefficients = new int[1];						// Dealing with already initialized object, so this->N is well defined
		if(t < 0) t = -t;
		if(t >= this->p) t %= this->p;
		this->coefficients[0] = t;
		this->degree = 0;
		return *this;
	}

	inline NTRU_ZpPolynomial operator - (const NTRU_ZpPolynomial& P) const;

	inline NTRU_ZpPolynomial operator - () const {
		NTRU_ZpPolynomial r = *this;
		for(int i = 0; i <= r.degree; i++)
			r.coefficients[i] = Z3subtraction[0][r.coefficients[i]];
		return r;
	}

	inline NTRU_ZpPolynomial operator * (const NTRU_ZpPolynomial& P) const;

	inline bool operator != (int t) const {								// Comparison with a single integer
		return this->degree != 0 || this->coefficients[0] != t;
	}

	inline int coeffAmount() const {return this->degree + 1;}
	inline int maxDeg(const NTRU_ZpPolynomial& P) const{
		if(this->degree > P.degree) return this->degree;
		return P.degree;
	}
	void division(const NTRU_ZpPolynomial& P,NTRU_ZpPolynomial rslt[2]) const;	// The quotient and the remainder are saved in rslt

	NTRU_ZpPolynomial gcdXNmns1(int N, NTRU_ZpPolynomial& Bezout2) const;		// Greatest common divisor between X^N-1 and P. Coefficients in Z3
									 											// Bezout coefficients are polynomials u and v such that u*a + v*b = gcd(a,b),
									 											// supposing b is *this polynomial, Bezout2 will contain v polynomial

		//int invModq(int t) const;												// Calculates inverse modulus q

		/*inline int modq(int t) const{											// returns t mod q using bit wise operations. The result will be positive
			while(t < 0) t += q;												// Adding q till we get t positive. This won't affect the result
    		return t &= q-1;													// Since q is a power of 2, the expression t &= q-1 is equivalent to t %= q
		}*/
		//void thisCoeffOddRandom(int deg);										// Assigns a random odd integer between 0 and q to each coefficient till deg
	inline bool operator == (int t) const {								// Comparison with a single integer
		return this->degree == 0 && this->coefficients[0] == t;
	}
	void print(const char* name = "") const;
	inline void println(const char* name = "") const{
		print(name); std::cout<<'\n';
	}
};

																				// Functions for printing
inline static int len(char* str) {												// Length of a string
	int l = -1;
	while(str[++l] != 0) {}
	return l;
}

inline static int intToString(int n, char* dest) {								// String representation of unsigned integer it returns the length of the strign
    int i = 0, j = 0, l = 0;
    char buff = 0;
    if(n < 0) {
    	dest[i++] = '-';
    	n = -n;
    }
    do {
        buff = (char)(n % DECIMAL_BASE);                                        // Taking last current digit
        dest[i++] = buff + 48;                                                  // Saving last current digit
        n -= (int)buff; n /= DECIMAL_BASE;                                 		// Taking out last current digit from the number n
    } while(n > 0);
    l = i;
    dest[i--] = 0;                                                              // Putting a zero at the end and returning one place
    for(; j < i; j++,i--) {                                                     // The number is backwards; reversing the order of the digits
        buff = dest[j];
        dest[j] = dest[i];
        dest[i] = buff;
    }
    return l;
}
inline static int copyString(const char* origin,char* dest) {
    int i = 0;                                                                  // Counting variable. At the end it will contain the length of the origin string
    for(; origin[i] != 0; i++) {dest[i] = origin[i];}                           // Coping element by element
    dest[i] = 0;                                                                // End of string.
    return i;                                                                   // Returning string length
}
inline static int printSpaces(unsigned t) {
	while(t-- > 0) std::cout << ' ' ;
	return 0;
}