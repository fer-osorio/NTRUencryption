#include"NTRUencryption.hpp"

using namespace NTRUPolynomial;
using namespace NTRU;

Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_):
N(_N_), q(_q_), p(_p_), d(_d_), privateKey(_N_, _q_), privateKeyInv_p(_N_, _q_),
publicKey(ZpPolynomial(_N_, _d_, _d_), _q_) {
	this->setPrivateKeyAndInv();
	ZqCenterPolynomial f_Fp = this->privateKey*this->privateKeyInv_p;
	f_Fp.mods_p(_3_);
	f_Fp.println("f_Fp");
}

void Encryption::encrypt(Message& msg) {
    //ZqCenterPolynomial msg_pol(msg.content, msg.len, this->N, this->q);
}

void Encryption::setPrivateKeyAndInv() {
    ZpPolynomial  _privateKey_(this->N, this->d+1, this->d);
    ZpPolynomial  _privateKeyInv_p_(this->N, this->q);
	ZpPolynomial  Zp_gcdXNmns1(this->N, this->q);
	ZqPolynomial  privateKeyZq(this->N, this->q);
	ZqPolynomial  privateKeyInv_q(this->N, this->q);
	ZqPolynomial::Z2Polynomial privateKeyZ2(_privateKey_);
	ZqPolynomial::Z2Polynomial privateKeyInv_2(this->N, 0);
	ZqPolynomial::Z2Polynomial Z2_gcdXNmns1(this->N, 0);
	int counter, k = 2, l = 1;

	try{
	    Zp_gcdXNmns1 = _privateKey_.gcdXNmns1(_privateKeyInv_p_);
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption"
        "::setPrivateKeyAndInv()\n";
        throw;
	}
	try{
	    Z2_gcdXNmns1 = privateKeyZ2.gcdXNmns1(privateKeyInv_2);
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption"
        "::setPrivateKeyAndInv()\n";
        throw;
	}
	counter = 1;

    while((Zp_gcdXNmns1 != 1 || Z2_gcdXNmns1 != 1) /*&& counter < 3*/) {
        if((counter & 3) != 0) {                                                // If we have not tried to much times, just permute the coefficients
            _privateKey_.permute();                                         // counter & 3 == counter % 4
	        privateKeyZ2 = _privateKey_;                                    // ...
        } else {
	        this->d--;                                                          // To much tries, increasing the numbers of non-zero coefficients
            _privateKey_ = ZpPolynomial(this->N, this->d+1, this->d);  // ...
            privateKeyZ2 = _privateKey_;                                    // ...
	    }
	    try{ Zp_gcdXNmns1=_privateKey_.gcdXNmns1(_privateKeyInv_p_); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRU"
            "encryption::setPrivateKeyAndInv()\n";
            throw;
	    }
	    try{ Z2_gcdXNmns1 = privateKeyZ2.gcdXNmns1(privateKeyInv_2); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRU"
            "encryption::setPrivateKeyAndInv()\n";
            throw;
    	}
	    counter++;
	}
	Z2_gcdXNmns1.println("Z2_gcdXNmns1");
	(privateKeyZ2*privateKeyInv_2).println("privateKeyZ2*privateKeyInv_2");
	if(counter > 1)
	    std::cout << "Private key was found after " <<counter<< " attempts.\n";
	else
	    std::cout << "Private key was found after "<< counter << " attempt.\n";

    privateKeyZq = _privateKey_;
	privateKeyInv_q = ZqPolynomial(privateKeyInv_2, this->q);
	while(k < this->q) {
	    privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
        k <<= l; l <<= 1;
	}
	//privateKeyInv_q.println("Fq");
	this->privateKey = ZqCenterPolynomial(_privateKey_, this->q);
	this->privateKeyInv_p = ZqCenterPolynomial(_privateKeyInv_p_, this->q);
	this->publicKey = this->publicKey * privateKeyInv_q;
	(privateKeyZq*privateKeyInv_q).println("privateKeyZq*privateKeyInv_q");
}

