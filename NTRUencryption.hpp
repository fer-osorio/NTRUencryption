#include<iostream>
#include"NTRUPolynomial.hpp"

namespace NTRU{
class Encryption {
	public:																		// Attributes
	const NTRU_N N;
	const NTRU_q q;
	const NTRU_p p;
	private: int d;

	NTRUPolynomial::ZqCenterPolynomial privateKey;
	NTRUPolynomial::ZqCenterPolynomial privateKeyInv_p;							// Private key inverse modulo p
	NTRUPolynomial::ZqCenterPolynomial publicKey;

	public:Encryption(NTRU_N _N_, NTRU_q _q_, int _d_=0,NTRU_p _p_=_3_);
	NTRUPolynomial::ZqCenterPolynomial encrypt(const NTRUPolynomial::ZpCenterPolynomial&);
	NTRUPolynomial::ZpCenterPolynomial decrypt(const NTRUPolynomial::ZqCenterPolynomial&);

	NTRU_N get_N() { return this->N; }

	private: void setKeys();													// Creation of the keys
};
}