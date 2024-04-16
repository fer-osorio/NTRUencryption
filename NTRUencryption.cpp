#include"NTRUencryption.hpp"

using namespace NTRUPolynomial;
using namespace NTRU;

Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_): N(_N_), q(_q_), p(_p_), d(_d_), privateKey(_N_, _p_), privateKeyInv_p(_N_, _p_),
publicKey(_N_, _q_) {
	try {
	    this->setKeys();
	}catch(const char* exp) {
	    std::cout << "\nIn file NTRUencryption.cpp, function Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_): N(_N_),q(_q_), p(_p_), d(_d_), "
        "privateKey(_N_, _p_), privateKeyInv_p(_N_, _p_), publicKey(_N_, _q_)\n";
        std::cout << exp;
	}
	(this->privateKey*this->privateKeyInv_p).println("fp*Fp");
}

ZqCenterPolynomial Encryption::encrypt(const ZpCenterPolynomial& msg) {
    ZqCenterPolynomial _msg_(msg, this->q);
    ZqCenterPolynomial r = ZqCenterPolynomial::randomTernary((unsigned)this->d, this->N, this->q, true); // Ternary polynomial multiplied by p (default p = 3)
    return this->publicKey*r + _msg_;
}

ZqCenterPolynomial Encryption::encrypt(const char fstring[]) {
    int l = -1;
    while(fstring[++l] != 0) {}
    ZqCenterPolynomial _msg_(ZpCenterPolynomial(this->N, this->p, fstring, l), this->q);
    ZqCenterPolynomial r = ZqCenterPolynomial::randomTernary((unsigned)this->d, this->N, this->q, true); // Ternary polynomial multiplied by p (default p = 3)
    return this->publicKey*r + _msg_;
}

ZpCenterPolynomial Encryption::decrypt(const ZqCenterPolynomial& e_msg) {
    ZqCenterPolynomial a = e_msg*this->privateKey;
    a.mods_p(this->p);
    ZpCenterPolynomial b = ZpCenterPolynomial(a, this->p);
    b = b*this->privateKeyInv_p;
    return b;
}

void Encryption::setKeys() {
    ZpPolynomial  _privateKey_(this->N, this->d+1, this->d);
    ZpPolynomial  _privateKeyInv_p_(this->N, this->q);
	ZpPolynomial  Zp_gcdXNmns1(this->N, this->q);
	ZqPolynomial  privateKeyZq(this->N, this->q);
	ZqPolynomial  privateKeyInv_q(this->N, this->q);
	ZqCenterPolynomial g(this->N, this->q);
	ZqCenterPolynomial Fq(this->N, this->q);
	ZqPolynomial::Z2Polynomial privateKeyZ2(_privateKey_);
	ZqPolynomial::Z2Polynomial privateKeyInv_2(this->N, 0);
	ZqPolynomial::Z2Polynomial Z2_gcdXNmns1(this->N, 0);
	int counter, k = 2, l = 1;

	try{
	    Zp_gcdXNmns1 = _privateKey_.gcdXNmns1(_privateKeyInv_p_);
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption::setKeys()\n";
        throw;
	}
	try{
	    Z2_gcdXNmns1 = privateKeyZ2.gcdXNmns1(privateKeyInv_2);
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption::setKeys()\n";
        throw;
	}

	counter = 1;
    while(Zp_gcdXNmns1 != 1 || Z2_gcdXNmns1 != 1) {
        if((counter & 3) != 0) {                                                // If we have not tried to much times, just permute the coefficients
            _privateKey_.permute();                                             // counter & 3 == counter % 4
	        privateKeyZ2 = _privateKey_;                                        // ...
        } else {
	        this->d--;                                                          // To much tries, increasing the numbers of non-zero coefficients
            _privateKey_ = ZpPolynomial(this->N, this->d+1, this->d);           // ...
            privateKeyZ2 = _privateKey_;                                        // ...
	    }
	    try{ Zp_gcdXNmns1=_privateKey_.gcdXNmns1(_privateKeyInv_p_); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRUencryption::setKeys()\n";
            throw;
	    }
	    try{ Z2_gcdXNmns1 = privateKeyZ2.gcdXNmns1(privateKeyInv_2); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRUencryption::setKeys()\n";
            throw;
    	}
	    counter++;
	}

    privateKeyZq = _privateKey_;
	privateKeyInv_q = ZqPolynomial(privateKeyInv_2, this->q);

	while(k < this->q) {
	    privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
        k <<= l; l <<= 1;
	}
	Fq = privateKeyInv_q;
	this->privateKey = _privateKey_;
	this->privateKeyInv_p = _privateKeyInv_p_;
	if(Fq*this->privateKey == 1) {
	    //(Fq*this->privateKey).println("Fq*fq");
	    if(this->privateKey*this->privateKeyInv_p == 1) {
	        //(this->privateKey*this->privateKeyInv_p).println("Fp*fp");
	        if(counter > 1)
	            std::cout << "Private key was found after " <<counter<< " attempts.\n";
	        else
	            std::cout << "Private key was found after "<< counter << " attempt.\n";
	    } else {
	        throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zp[x]/(x^N-1) ring not found.\n";
	    }
	} else {
	    throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zq[x]/(x^N-1) ring not found\n";
	}
	g = ZqCenterPolynomial::randomTernary((unsigned)this->d, this->N, this->q, true);
	this->publicKey = g*Fq;
}
