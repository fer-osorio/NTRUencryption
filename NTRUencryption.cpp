
#include"NTRUencryption.hpp"

NTRUencryption::NTRUencryption(NTRU_N _N_,NTRU_q _q_, int _d_, NTRU_p _p_):
N(_N_), q(_q_), d(_d_), p(_p_), privateKey(_N_,_d_+ 1, _d_) {
	this->privateKey.println("Private key");
	this->privateKey.test(300);
	//this->setPrivateKeyAndInv();
}

void NTRUencryption::setPrivateKeyAndInv() {
    NTRU_ZpPolynomial::ZpPolModXNmns1 inverse(this->N);
	NTRU_ZpPolynomial _gcdXNmns1_ = privateKey.gcdXNmns1(inverse);

    while(_gcdXNmns1_ != 1) {
	    _gcdXNmns1_.println("\n_gcdXNmns1_");
	    privateKey.permute();
	    _gcdXNmns1_ = privateKey.gcdXNmns1(inverse);                            // Put try statement
	}
	_gcdXNmns1_.println("\n_gcdXNmns1_");
}
