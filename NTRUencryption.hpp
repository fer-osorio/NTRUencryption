// Operating with the polynomials in the ring Zp[x]/x^N-1.
#include <iostream>

class NTRUencryption {
	public:
	enum NTRU_N {_509 = 509, _677 = 677, _701 = 701, _821 = 821};				// All the possible values for the N
	enum NTRU_q {_2048 = 2048, _4096 = 4096, _8192 = 8192};						// All the possible values for the q

	private:																	// Private types and attributes
	enum log2NTRUq {_11 = 11, _12 = 12, _13 = 13};								// For each possible q: log_2(q)
	static const NTRU_N N;
	static const NTRU_q q;
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

		NTRUPolynomial& operator = (const NTRUPolynomial&);

		private:
		inline int min(int a, int b) {											// Returns the smaller of two numbers
			if(a < b) return a;
			return b;
		}
	};

	struct NTRUPolynomial_2array {												// Array of NTRUPolynomials with just two elements
		NTRUPolynomial polynomial[2];
	};
};