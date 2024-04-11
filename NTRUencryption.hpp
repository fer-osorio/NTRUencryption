#include<iostream>
#include"NTRUPolynomial.hpp"

namespace NTRU{
class Encryption {
	public:																		// Attributes
	const NTRU_N N;
	const NTRU_q q;
	const NTRU_p p;
	private: int d;

	NTRUPolynomial::ZqCenterPolynomial publicKey;
	NTRUPolynomial::ZqCenterPolynomial privateKey;
	NTRUPolynomial::ZqCenterPolynomial privateKeyInv_p;							// Private key inverse modulo p

	public:Encryption(NTRU_N _N_, NTRU_q _q_, int _d_=0,NTRU_p _p_=_3_);
	void encrypt(NTRUPolynomial::Message&);

	private: void setPrivateKeyAndInv();										// Creates private key and inverse of the private key
};
}