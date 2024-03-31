#include<iostream>
#include"NTRUPolynomials.hpp"

class NTRUencryption {
	public:																		// Attributes
	const NTRU_N N;
	const NTRU_q q;
	const int 	 d;
	const NTRU_p p;

	NTRUencryption(NTRU_N _N_, NTRU_q _q_, int _d_ = 0, NTRU_p _p_ = _3_);

	private:
	//ZpPolynomial publicKey;
	NTRU_ZpPolynomial privateKey;
	NTRU_ZpPolynomial privateKeyInv_p;												// Private key inverse modulo p

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
