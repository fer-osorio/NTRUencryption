#include"NTRUencryption.hpp"

NTRUencryption::NTRUencryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_):N(_N_),
q(_q_), p(_p_), d(_d_), privateKey(_N_, _d_+1, _d_), privateKeyInv_p(_N_) {
	this->setPrivateKeyAndInv();
	(this->privateKey*this->privateKeyInv_p).println("privateKey*privateKeyInv_p");
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

    while((Zp_gcdXNmns1 != 1 || Z2_gcdXNmns1 != _1_) /*&& counter < 3*/) {
        if((counter & 3) != 0) {                                                // If we have not tried to much times, just permute the coefficients
            this->privateKey.permute();                                         // counter & 3 == counter % 4
	        privateKeyZ2 = this->privateKey;                                    // ...
        } else {
	        this->d--;                                                          // To much tries, increasing the numbers of non-zero coefficients
            this->privateKey = NTRU_ZpPolynomial(this->N, this->d+1, this->d);  // ...
            privateKeyZ2 = this->privateKey;                                    // ...
	    }
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
	Z2_gcdXNmns1.println("Z2_gcdXNmns1");
	(privateKeyZ2*privateKeyInv_2).println("privateKeyZ2*privateKeyInv_2");
	if(counter > 1)
	    std::cout << "Private key was found after " <<counter<< " attempts.\n";
	else
	    std::cout << "Private key was found after "<< counter << " attempt.\n";

    privateKeyZq = this->privateKey;
	privateKeyInv_q = NTRU_ZqPolynomial(privateKeyInv_2, this->q);
	while(k < this->q) {
	    privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
        k <<= l; l <<= 1;
	}
	//privateKeyInv_q.println("Fq");
	(privateKeyZq*privateKeyInv_q).println("privateKeyZq*privateKeyInv_q");
}
