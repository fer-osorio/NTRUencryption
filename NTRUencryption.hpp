#include<iostream>
#include"NTRUPolynomial.hpp"

using namespace NTRUPolynomial;

namespace NTRU{
class Encryption {
	public:																		// Attributes
	const NTRU_N N;
	const NTRU_q q;
	const NTRU_p p;
	private: int d;

	ZpCenterPolynomial privateKey;
	ZpCenterPolynomial privateKeyInv_p;											// Private key inverse modulo p
	ZqCenterPolynomial publicKey;

	public:
	Encryption(NTRU_N _N_, NTRU_q _q_, int _d_=0,NTRU_p _p_=_3_);
	ZqCenterPolynomial encrypt(const ZpCenterPolynomial&);
	ZqCenterPolynomial encrypt(const char fstring[]);							// Encryption of formatted string
	ZpCenterPolynomial decrypt(const ZqCenterPolynomial&);

	NTRU_N get_N() const{ return this->N; }
	NTRU_p get_p() const{ return this->p; }
	void savePrivateKey_txt();

	private: void setKeys();													// Creation of the keys
};
}