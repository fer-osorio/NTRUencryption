#include<iostream>
#include"ZpPolynomial.hpp"

#define DECIMAL_BASE 10

class NTRUencryption {
	public:																		// Attributes
	const NTRU_N N;
	const NTRU_q q;
	const int 	 d;
	const NTRU_p p;

	NTRUencryption(NTRU_N _N_, NTRU_q _q_, int _d_ = 0, NTRU_p _p_ = _3_);

	private:
	//ZpPolynomial publicKey;
	ZpPolynomial privateKey;
	//ZpPolynomial privateKeyInvp;											// Private key inverse modulo p

	void setPrivateKeyAndInv();													// Creates private key and inverse of the private key

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