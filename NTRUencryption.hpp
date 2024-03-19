// Operating with the polynomials in the ring Zp[x]/x^N-1.
#include<iostream>

#define DECIMAL_BASE 10

class NTRUencryption {
	public:
	enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821 };	// All the possible values for the N
	enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };					// All the possible values for the q
	enum NTRU_p {_3_	= 3 };
	const NTRU_N N;
	const NTRU_q q;
	const NTRU_p p;
	const int 	 d;

	private:																	// Private types and attributes
	struct NTRUPolynomial {														// Representation of the polynomials
		int* coefficients = NULL;												// Coefficients of the polynomial
		NTRU_N N;
		NTRU_q q;
		NTRU_p p;

		NTRUPolynomial(NTRU_N _N_, NTRU_q _q_, int ones = 0, int negOnes = 0,	// Ones and negOnes will dictate the amount of 1 and -1 respectively
		NTRU_p _p_=_3_);

		inline NTRUPolynomial(NTRUPolynomial& P): N(P.N), q(P.q) {
			this->coefficients = new int[P.N];
			for(int i=0; i<P.N; i++) this->coefficients[i] = P.coefficients[i];
		}
		inline ~NTRUPolynomial() {
			if(this->coefficients != NULL) delete [] this->coefficients;
		}

		inline NTRUPolynomial(): N(_509_), q(_2048_) {}							// Initializing N and q, coefficients is left as zero

		// Arithmetic
		NTRUPolynomial operator + (const NTRUPolynomial&) const;				// Addition element by element
		NTRUPolynomial operator - (const NTRUPolynomial&) const;				// Subtraction element by element
		NTRUPolynomial operator * (const NTRUPolynomial&) const;				// Multiplication will coincide with convolution
		void division(const NTRUPolynomial& P, NTRUPolynomial result[2]) const;	// Computes quotient and remainder between this and P, saves the result in result[2]
		NTRUPolynomial gcd(const NTRUPolynomial&,								// Greatest common divisor
								 NTRUPolynomial Bezout[2])const;				// Bezout coefficients are polynomials u and v such that u*a + v*b = gcd(a,b)

		inline NTRUPolynomial& operator = (const NTRUPolynomial& P) {			// Assignment
			if(this != &P) {                                                    // Guarding against self assignment
				if(this->coefficients == NULL)
					this->coefficients = new int[P.N];
				else if(this->N != P.N) {
					delete [] this->coefficients;
					this->coefficients = new int[P.N];
				}
				this->N = P.N;
				this->q = P.q;
        		for(int i = 0; i < this->N; i++)
        			this->coefficients[i] = P.coefficients[i];
        	}
    		return *this;
		}
		inline NTRUPolynomial& operator = (int t) {								// Assignment with single integer
			if(this->coefficients == NULL)
				this->coefficients = new int[this->N];							// Dealing with already initialized object, so this->N is well defined
			this->coefficients[0] = t;
        	for(int i=1; i < this->N; i++) this->coefficients[i] = 0;			// Filling with zeros the rest of the array
    		return *this;
		}
		inline int degree() const {												// Returns degree of polynomial
			int deg = this->N;
			while(this->coefficients[--deg] == 0 && deg > 0) {}
			return deg;
		}
		void print(const char* name = "") const;

		inline void println(const char* name = "") const {
			print(name); std::cout<<'\n';
		}

		int invModq(int t) const;												// Calculates inverse modulus q

		inline int modq(int t) const{											// returns t mod q using bit wise operations. The result will be positive
			while(t < 0) t += q;												// Adding q till we get t positive. This won't affect the result
    		return t &= q-1;													// Since q is a power of 2, the expression t &= q-1 is equivalent to t %= q
		}
		void thisCoeffOddRandom(int deg);										// Assigns a random odd integer between 0 and q to each coefficient till deg

		inline bool operator == (int t) const {									// Comparison with a single integer
			return this->degree() == 0 && this->coefficients[0] == t;
		}
		inline bool operator != (int t) const {									// Comparison with a single integer
			return this->degree() != 0 || this->coefficients[0] != t;
		}
		inline bool operator == (const NTRUPolynomial& P) {
			if(this->N == P.N && this->q == P.q) {
				for(int i = 0; i < this->N; i++)
					if(this->coefficients[i] != P.coefficients[i]) return false;
				return true;
			}
			else return false;
		}
		inline bool operator != (const NTRUPolynomial& P) {
			if(this->N == P.N && this->q == P.q) {
				for(int i = 0; i < this->N; i++)
					if(this->coefficients[i] != P.coefficients[i]) return true;
				return false;
			}
			else return true;
		}
		private:
		inline NTRU_N min_N(const NTRUPolynomial& P) const{
			if(this->N < P.N) return this->N;
			return P.N;
		}
		inline NTRU_N max_N(const NTRUPolynomial& P) const{
			if(this->N < P.N) return P.N;
			return this->N;
		}
		inline NTRU_q max_q(const NTRUPolynomial& P) const{
			if(this->q < P.q) return P.q;
			return this->q;
		}
		inline int min(int a, int b) const{
			if(a < b) return a;
			return b;
		}
		inline int max(int a, int b) const{
			if(a < b) return b;
			return a;
		}
		inline void copyCoefficients(const NTRUPolynomial& P) {
			NTRU_N _N_ = this->min_N(P);
			for(int i=0; i<_N_; i++) this->coefficients[i]=P.coefficients[i];
		}
	};

	public:
	NTRUencryption(NTRU_N _N_, NTRU_q _q_, NTRU_p _p_ = _3_, int _d_ = 0);

	inline static int _3inverseModq(NTRU_q _q_) {
		switch(_q_) {
			case _2048_:return 683;												//  3*683  mod 2048 = 1
			case _4096_:return 2731;											//  3*2731 mod 4096 = 1
			case _8192_:return 2731;											//  3*2731 mod 8192 = 1
		}
	}
	inline static int neg3inverseModq(NTRU_q _q_) {
		switch(_q_) {
			case _2048_:return 1365;											// -3*1365 mod 2048 = 1
			case _4096_:return 1365;											// -3*1365 mod 4096 = 1
			case _8192_:return 5461;											// -3*5461 mod 8192 = 1
		}
	}
};
																				// Functions for printing
inline static int len(char* str) {												// Length of a string
	int l = -1;
	while(str[++l] != 0) {}
	return l;
}

inline static int uintToString(unsigned n, char* dest) {								// String representation of unsigned integer
    unsigned i = 0, j = 0;
    char buff = 0;
    do {
        buff = (char)(n % DECIMAL_BASE);                                        // Taking last current digit
        dest[i++] = buff + 48;                                                  // Saving last current digit
        n -= (unsigned)buff; n /= DECIMAL_BASE;                                 // Taking out last current digit from the number n
    } while(n > 0);
    dest[i--] = 0;                                                              // Putting a zero at the end and returning one place
    for(; j < i; j++,i--) {                                                     // The number is backwards; reversing the order of the digits
        buff = dest[j];
        dest[j] = dest[i];
        dest[i] = buff;
    }
    return 0;
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