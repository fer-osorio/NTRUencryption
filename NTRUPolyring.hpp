// Operating with the polynomials in the ring Zp[x]/x^N-1.
#include <iostream>

struct NTRUPolyring {
	enum NTRU_N {};																	// All the possible values for the N
	enum NTRU_q {};																	// All the possible values for the q
	int* coefficients = NULL;
	inline NTRUPolyring() {}
};