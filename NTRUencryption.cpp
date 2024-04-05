#include"NTRUencryption.hpp"

NTRUencryption::NTRUencryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_):N(_N_),
q(_q_), d(_d_), p(_p_), privateKey(_N_, _d_+1, _d_), privateKeyInv_p(_N_) {
    NTRU_ZqPolynomial::Z2Polynomial t(this->privateKey);
    t.test(_N_,_N_/3);
	this->privateKey.test(_N_, _N_/3 - 1);
	this->setPrivateKeyAndInv();
	(privateKey*privateKeyInv_p).println("privateKey*privateKeyInv_p");
}

void NTRUencryption::setPrivateKeyAndInv() {
	NTRU_ZpPolynomial  Zp_gcdXNmns1(this->N, this->q);
	NTRU_ZqPolynomial::Z2Polynomial Z2_gcdXNmns1(this->privateKey);
	NTRU_ZqPolynomial::Z2Polynomial Z2inverse(this->N, 0);
	int counter, k = 2, l = 1;

	try{
	    Zp_gcdXNmns1 = this->privateKey.gcdXNmns1(this->privateKeyInv_p);
	}catch(const char* exp) {
        std::cout<<"\nIn file NTRUencryption.cpp, function void NTRUencryption"
        "::setPrivateKeyAndInv()\n";
        throw;
	}
	try{
	    Z2_gcdXNmns1 = Z2_gcdXNmns1.gcdXNmns1(Z2_gcdXNmns1);
	}catch(const char* exp) {
        std::cout<<"\nIn file NTRUencryption.cpp, function void NTRUencryption"
        "::setPrivateKeyAndInv()\n";
        throw;
	}
	counter = 1;

	Z2_gcdXNmns1 = Z2_gcdXNmns1.gcdXNmns1(Z2inverse);
    while(Zp_gcdXNmns1 != 1 && Z2_gcdXNmns1 != _1_) {
	    this->privateKey.permute();
	    Z2_gcdXNmns1 = this->privateKey;
	    try{ Zp_gcdXNmns1=this->privateKey.gcdXNmns1(this->privateKeyInv_p); }
	    catch(const char* exp) {
            std::cout<<"\nIn file NTRUencryption.cpp, function void NTRU"
            "encryption::setPrivateKeyAndInv()\n";
            throw;
	    }
	    try{ Z2_gcdXNmns1 = Z2_gcdXNmns1.gcdXNmns1(Z2_gcdXNmns1); }
	    catch(const char* exp) {
            std::cout<<"\nIn file NTRUencryption.cpp, function void NTRU"
            "encryption::setPrivateKeyAndInv()\n";
            throw;
    	}
	    counter++;
	}
	while(k < this->q) {
        k >>= l; l >>= 1;
	}

	if(counter > 1)
	    std::cout << "Private key was found after " <<counter<< " attempts.\n";
	else
	    std::cout << "Private key was found after "<< counter << " attempt.\n";
}
