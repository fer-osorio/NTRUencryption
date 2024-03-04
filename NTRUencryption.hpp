// Operating with the polynomials in the ring Zp[x]/x^N-1.
#include <iostream>

struct NTRUPolynomial {

	public:
	enum NTRU_N {_509 = 509, _677 = 677, _701 = 701, _821 = 821};				// All the possible values for the N
	enum NTRU_q {_2048, _4096, _8192};											// All the possible values for the q

	private:																	// Private types and attributes
	enum log2NTRUq {_11 = 11, _12 = 12, _13 = 13};								// For each possible q: log_2(q)
	NTRU_N N;
	NTRU_q q;
	int* coefficients = NULL;													// Coefficients of the polynomial
	inline NTRUPolynomial(): N(_509), q(_2048) {}								// Private default constructor; initializes N and q, maintains coefficients as NULL

	public:
	inline NTRUPolynomial(NTRU_N _N, NTRU_q _q): N(_N), q(_q) {					// Constructor, initialized polynomial with zeros
		this->coefficients = new int[_N];
		for(int i = 0; i < _N; i++) this->coefficients[i] = 0;
	}
	inline ~NTRUPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
	}
																				// Arithmetic
	NTRUPolynomial operator + (const NTRUPolynomial&) const;
	NTRUPolynomial operator * (const NTRUPolynomial&) const;					// Multiplication will coincide with convolution

	NTRUPolynomial& operator = (const NTRUPolynomial&);

	private:
	inline int min(int a, int b) {												// Returns the smaller of two numbers
		if(a < b) return a;
		return b;
	}
};

struct NTRUPolynomial_2array {													// Array of NTRUPolynomials with just two elements
	NTRUPolynomial polynomial[2];
};