#include"NTRUencryption.hpp"

using namespace NTRUPolynomial;
using namespace NTRU;

ZpCenterPolynomial g(_821_, _3_);
ZqCenterPolynomial Fq(_821_, _8192_);

Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_):N(_N_),
q(_q_), p(_p_), d(_d_), privateKey(_N_, _p_), privateKeyInv_p(_N_, _p_),
publicKey(ZqCenterPolynomial::randomTernary((unsigned)_d_, _N_, _q_)) {
	this->setKeys();
	(this->privateKey*this->privateKeyInv_p).println("fp*Fp");
}

ZqCenterPolynomial Encryption::encrypt(const ZpCenterPolynomial& msg) {
    ZqCenterPolynomial _msg_(msg, this->q);
    if(_msg_ == msg) std::cout << "\nEncryption::encrypt. Correct lifting to ZqCenterPolynomial\n";
    else std::cout << "\nEncryption::encrypt. Incorrect lifting to ZqCenterPolynomial\n";
    ZqCenterPolynomial r = ZqCenterPolynomial::randomTernary((unsigned)this->d, this->N, this->q, true); // Ternary polynomial multiplied by p (default p = 3)
    ZqCenterPolynomial s = r*g;
    if(s == this->publicKey*r*this->privateKey) std::cout << "\nEncryption::encrypt. (Fq*g)*r*f == r*g\n";
    else std::cout << "\nEncryption::encrypt.  (Fq*g)*r*f != r*g\n";
    return this->publicKey*r + _msg_;
}

ZpCenterPolynomial Encryption::decrypt(const ZqCenterPolynomial& e_msg) {
    ZqCenterPolynomial a = e_msg*this->privateKey;
    a.mods_p(this->p);
    ZpCenterPolynomial b = ZpCenterPolynomial(a, this->p);
    if(a == b) std::cout << "\nEncryption::decrypt. Correct lowering to ZpCenterPolynomial\n";
    else std::cout << "\nEncryption::decrypt. Incorrect lowering to ZpCenterPolynomial\n";
    b = b*this->privateKeyInv_p;
    return b;
}

void Encryption::setKeys() {
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
        "::setKeys()\n";
        throw;
	}
	try{
	    Z2_gcdXNmns1 = privateKeyZ2.gcdXNmns1(privateKeyInv_2);std::cout << "NTRUencryption.cpp::Encryption::setKeys(), line 49" << std::endl;
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption"
        "::setKeys()\n";
        throw;
	}
	counter = 1;

    while((Zp_gcdXNmns1 != 1 || Z2_gcdXNmns1 != 1) /*&& counter < 3*/) {
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
            std::cout<<"\nIn file Encryption.cpp, function void NTRU"
            "encryption::setKeys()\n";
            throw;
	    }
	    try{ Z2_gcdXNmns1 = privateKeyZ2.gcdXNmns1(privateKeyInv_2); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRU"
            "encryption::setKeys()\n";
            throw;
    	}
	    counter++;
	    if((counter & 31) == 0) std::cout << counter << std::endl;
	}
	Z2_gcdXNmns1.println("Z2_gcdXNmns1");
	_privateKey_.println("privateKeyZp");
	privateKeyZ2.println("privateKeyZ2");
	privateKeyInv_2.println("privateKeyInv_2");
	(privateKeyZ2*privateKeyInv_2).println("privateKeyZ2*privateKeyInv_2");
	if(counter > 1)
	    std::cout << "Private key was found after " <<counter<< " attempts.\n";
	else
	    std::cout << "Private key was found after "<< counter << " attempt.\n";

    privateKeyZq = _privateKey_;
	privateKeyInv_q = ZqPolynomial(privateKeyInv_2, this->q);                   // Something funny is going on here

	if(privateKeyInv_q == privateKeyInv_2)
	    std::cout << "Correct cast from Z2Polynomial to ZqPolynomial. privateKeyInv_q == privateKeyInv_2" << std::endl;
	else
	    std::cout << "Incorrect cast from Z2Polynomial to ZqPolynomial. privateKeyInv_q != privateKeyInv_2" << std::endl;

	//privateKeyInv_2.println("privateKeyInv_2");
	//privateKeyInv_q.println("privateKeyInv_q");

	while(k < this->q) {
	    privateKeyInv_q = privateKeyInv_q*(2 - privateKeyZq*privateKeyInv_q);
        k <<= l; l <<= 1;
	}
	this->privateKey = _privateKey_;
	if(this->privateKey == _privateKey_) std::cout << "\nCorrect cast from ZpPolynomial to ZpCenteredPolynomial; privateKey == _privateKey_\n";
	else {
	    std::cout << "\nInorrect cast from ZpPolynomial to ZpCenteredPolynomial; privateKey = _privateKey_\n";
	    this->privateKey.printTheDifferences(_privateKey_,"this->privateKey", "_privateKey_");
	}
    privateKeyInv_q.println("privateKeyInv_q");
    (privateKeyZq*privateKeyInv_q).println("prvKeyZq*prvKeyInv_q");
	this->privateKeyInv_p = _privateKeyInv_p_;
	ZqCenterPolynomial gq = this->publicKey;
	Fq = privateKeyInv_q;

	//this->publicKey = g*privateKeyInv_q;                          // Maybe something funny is going on here
	this->publicKey = gq*Fq;

	ZqCenterPolynomial fq(this->privateKey, this->q);
	(fq*Fq).println("fq*Fq");

	if(this->publicKey*this->privateKey == gq) std::cout << "\nNo problem with g*Fq\n";
	else std::cout << "\nSomething is going on with g*Fq\n";

	//(this->publicKey*this->privateKey).println("h*fp");
	fq.println("fq");
	//(this->publicKey*fq).println("h*fp");
}

