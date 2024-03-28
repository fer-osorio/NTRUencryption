#include"NTRUencryption.hpp"

NTRUencryption::NTRUencryption(NTRU_N _N_,NTRU_q _q_, int _d_, NTRU_p _p_):
N(_N_),q(_q_),d(_d_),p(_p_), privateKey(_N_,_d_+ 1,_d_),privateKeyInv_p(_N_) {
	this->privateKey.test(_N_/3);
	this->setPrivateKeyAndInv();
	(privateKey*privateKeyInv_p).println("privateKey*privateKeyInv_p");
	this->privateKey.println("Private key");
}

void NTRUencryption::setPrivateKeyAndInv() {
	NTRU_ZpPolynomial _gcdXNmns1_ =
	this->privateKey.gcdXNmns1(this->privateKeyInv_p);
	int counter = 1;

    while(_gcdXNmns1_ != 1) {
	    privateKey.permute();
	    try{ _gcdXNmns1_ = this->privateKey.gcdXNmns1(this->privateKeyInv_p); }
	    catch(const char* exp) {
	        std::cout << "\nIn file NTRUencryption.cpp, function void "
	        "NTRUencryption::setPrivateKeyAndInv()\n";
	        throw;
	    }
	    counter++;
	}
	if(counter > 1)
	    std::cout << "Private key was found after " <<counter<< " attempts.\n";
	else
	    std::cout << "Private key was found after "<< counter << " attempt.\n";
}
