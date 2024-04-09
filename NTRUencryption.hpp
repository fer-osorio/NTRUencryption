#include<iostream>
#include"NTRUPolynomials.hpp"

class NTRUencryption {
	public:																		// Attributes
	const NTRU_N N;
	const NTRU_q q;
	const NTRU_p p;
	private: int d;

	public:
	//ZpPolynomial publicKey;
	NTRU_ZpPolynomial privateKey;
	NTRU_ZpPolynomial privateKeyInv_p;											// Private key inverse modulo p

	public:NTRUencryption(NTRU_N _N_, NTRU_q _q_, int _d_=0, NTRU_p _p_=_3_);

	void setPrivateKeyAndInv();													// Creates private key and inverse of the private key
};
