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
	NTRU_ZqPolynomial  privateKeyZq(this->N, this->q);
	NTRU_ZqPolynomial  privateKeyInv_q(this->N, this->q);
	NTRU_ZqPolynomial::Z2Polynomial privateKeyZ2(this->privateKey);
	NTRU_ZqPolynomial::Z2Polynomial privateKeyInv_2(this->N, 0);
	NTRU_ZqPolynomial::Z2Polynomial Z2_gcdXNmns1(this->N, 0);
	int counter, k = 2, l = 1;

	try{
	    Zp_gcdXNmns1 = this->privateKey.gcdXNmns1(this->privateKeyInv_p);
	}catch(const char* exp) {
        std::cout<<"\nIn file NTRUencryption.cpp, function void NTRUencryption"
        "::setPrivateKeyAndInv()\n";
        throw;
	}
	try{
	    Z2_gcdXNmns1 = privateKeyZ2.gcdXNmns1(privateKeyInv_2);
	}catch(const char* exp) {
        std::cout<<"\nIn file NTRUencryption.cpp, function void NTRUencryption"
        "::setPrivateKeyAndInv()\n";
        throw;
	}
	counter = 1;

    while(Zp_gcdXNmns1 != 1 || Z2_gcdXNmns1 != _1_) {
	    this->privateKey.permute();
	    privateKeyZ2 = this->privateKey;
	    try{ Zp_gcdXNmns1=this->privateKey.gcdXNmns1(this->privateKeyInv_p); }
	    catch(const char* exp) {
            std::cout<<"\nIn file NTRUencryption.cpp, function void NTRU"
            "encryption::setPrivateKeyAndInv()\n";
            throw;
	    }
	    try{ Z2_gcdXNmns1 = privateKeyZ2.gcdXNmns1(privateKeyInv_2); }
	    catch(const char* exp) {
            std::cout<<"\nIn file NTRUencryption.cpp, function void NTRU"
            "encryption::setPrivateKeyAndInv()\n";
            throw;
    	}
	    counter++;
	}
	(privateKeyZ2*privateKeyInv_2).println("privateKeyZ2*privateKeyInv_2");
	if(counter > 1)
	    std::cout << "Private key was found after " <<counter<< " attempts.\n";
	else
	    std::cout << "Private key was found after "<< counter << " attempt.\n";

    privateKeyZq = this->privateKey;
	privateKeyInv_q = NTRU_ZqPolynomial(privateKeyInv_2, this->q);
	while(k < this->q) {
	    privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
	    //(privateKeyZq*privateKeyInv_q).println();
        k <<= l; l <<= 1;
	}/*
	privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
	privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
	privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
	privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
	privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
	privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);*/
	std::cout << "\nk = " << k << "\n";
	(privateKeyZq*privateKeyInv_q).println();
}
