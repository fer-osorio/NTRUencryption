// Operating with the polynomials in the ring Zp[x]/x^N-1.
#include<iostream>


class NTRUencryption {
	public:
	enum NTRU_N {_509 = 509, _677 = 677, _701 = 701, _821 = 821};				// All the possible values for the N
	enum NTRU_q {_2048 = 2048, _4096 = 4096, _8192 = 8192};						// All the possible values for the q
	static const NTRU_N N;
	static const NTRU_q q;

	private:																	// Private types and attributes
	enum log2NTRUq {_11 = 11, _12 = 12, _13 = 13};								// For each possible q: log_2(q)
	static const log2NTRUq L2NTRU_q;

	static const int _3inverseMod2048 = 683;									//  3*683  mod 2048 = 1
	static const int _neg3inverseMod2048 = 1365;								// -3*1365 mod 2048 = 1
	static const int _3inverseMod4096 = 2731;									//  3*2731 mod 4096 = 1
	static const int _neg3inverseMod4096 = 1365;								// -3*1365 mod 4096 = 1
	static const int _3inverseMod8192 = 2731;									//  3*2731 mod 8192 = 1
	static const int _neg3inverseMod8192 = 5461;								// -3*5461 mod 8192 = 1

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

		NTRUPolynomial& operator = (const NTRUPolynomial&);						// Assignment

		inline bool operator == (int t) const {									// Comparison with a single integer
			return this->degree() == 0 && this->coefficients[0] == t;
		}

		inline int degree() const {												// Returns degree of polynomial
			int deg = N;
			while(this->coefficients[--deg] == 0) {}
			return deg;
		}

		private:
		inline int min(int a, int b) {											// Returns the smaller of two numbers
			if(a < b) return a;
			return b;
		}
	};

	struct NTRUPolynomialPair {													// Array of NTRUPolynomials with just two elements
		NTRUPolynomial polynomial[2];
	};

	static int modq(int t);														// returns t mod q using bit wise operations. The result will be positive
};