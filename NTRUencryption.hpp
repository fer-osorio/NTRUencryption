// Operating with the polynomials in the ring Zp[x]/x^N-1.
#include<iostream>

class NTRUencryption {
	public:
	enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821};		// All the possible values for the N
	enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192};					// All the possible values for the q
	static const NTRU_N N;
	static const NTRU_q q;

	private:																	// Private types and attributes
	enum log2NTRUq {_11 = 11, _12 = 12, _13 = 13};								// For each possible q: log_2(q)
	static const log2NTRUq L2NTRU_q;

	struct NTRUPolynomial {														// Representation of the polynomials
		int* coefficients = NULL;												// Coefficients of the polynomial

		inline NTRUPolynomial() {												// Constructor, initialized polynomial with zeros
			this->coefficients = new int[N];
			for(int i = 0; i < N; i++) this->coefficients[i] = 0;
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
        		for(int i=0; i<N; i++) this->coefficients[i] = t.coefficients[i];
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
			int deg = N;
			while(this->coefficients[--deg] == 0 && deg > 0) {}
			return deg;
		}
	};
	inline static int modq(int t) {												// returns t mod q using bit wise operations. The result will be positive
		while(t < 0) t += q;													// Adding q till we get t positive. This won't affect the result
    	return t &= q-1;														// Since q is a power of 2, the expression t &= q-1 is equivalent to t %= q
	}

	static int invModq(int t);													// Calculates inverse modulus q}

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