// Operating with the polynomials in the ring Zp[x]/x^N-1.
#include<iostream>

#define DECIMAL_BASE 10

class NTRUencryption {
	public:
	enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821};		// All the possible values for the N
	enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192};					// All the possible values for the q
	const NTRU_N N;
	const NTRU_q q;

	NTRUencryption(NTRU_N _N_, NTRU_q _q_): N(_N_), q(_q_) {}

	private:																	// Private types and attributes
	struct NTRUPolynomial {														// Representation of the polynomials
		int* coefficients = NULL;												// Coefficients of the polynomial
		const NTRU_N N;
		const NTRU_q q;

		inline NTRUPolynomial(NTRU_N _N_, NTRU_q _q_): N(_N_), q(_q_) {			// Constructor, initialized polynomial with zeros
			this->coefficients = new int[this->N];
			for(int i = 0; i < this->N; i++) this->coefficients[i] = 0;
		}
		inline NTRUPolynomial(NTRUPolynomial& P): N(P.N), q(P.q) {
			this->coefficients = new int[P.N];
			for(int i=0; i<P.N; i++) this->coefficients[i] = P.coefficients[i];
		}
		inline ~NTRUPolynomial() {
			if(this->coefficients != NULL) delete [] this->coefficients;
		}
																				// Arithmetic
		NTRUPolynomial operator + (const NTRUPolynomial&) const;				// Addition element by element
		NTRUPolynomial operator * (const NTRUPolynomial&) const;				// Multiplication will coincide with convolution
		void division(const NTRUPolynomial t, NTRUPolynomial result[2]) const;	// Computes quotient and remainder between this and t, saves the result in result[2]

		inline NTRUPolynomial& operator = (const NTRUPolynomial& t) {			// Assignment
			if(this != &t)                                                    	// Guarding against self assignment
        		for(int i = 0; i < this->N; i++)
        			this->coefficients[i] = t.coefficients[i];
    		return *this;
		}
		inline NTRUPolynomial& operator = (int t) {								// Assignment with single integer
        	for(int i=0; i < N; i++) this->coefficients[i] = t;
    		return *this;
		}
		inline bool operator == (int t) const {									// Comparison with a single integer
			return this->degree() == 0 && this->coefficients[0] == t;
		}
		inline int degree() const {												// Returns degree of polynomial
			int deg = this->N;
			while(this->coefficients[--deg] == 0 && deg > 0) {}
			return deg;
		}
		void print() const;

		inline void println() const {
			print(); std::cout<<'\n';
		}
		inline int modq(int t) const{											// returns t mod q using bit wise operations. The result will be positive
			while(t < 0) t += q;												// Adding q till we get t positive. This won't affect the result
    		return t &= q-1;													// Since q is a power of 2, the expression t &= q-1 is equivalent to t %= q
		}
		int invModq(int t) const;												// Calculates inverse modulus q

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
	};

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
int static uintToString(unsigned n, char* dest);

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