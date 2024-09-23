#include<fstream>
#include"NTRUencryption.hpp"

using namespace NTRUPolynomial;
using namespace NTRU;

Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_):N(_N_),q(_q_),p(_p_),d(_d_),publicKey(_N_,_q_),privateKey(_N_,_p_),privateKeyInv_p(_N_,_p_) {
	try {
	    this->setKeys();
	}catch(const char* exp) {
	    std::cout <<
	    "\nIn file NTRUencryption.cpp, function "
	    "Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_): N(_N_),q(_q_), p(_p_), d(_d_), "
        "privateKey(_N_, _p_), privateKeyInv_p(_N_, _p_), publicKey(_N_, _q_)\n";
        std::cout << exp;
	}
}

ZqPolynomial Encryption::encrypt(const ZpPolynomial& msg) {
    ZqPolynomial encryptedMsg = msg.encrypt(this->publicKey);
    return encryptedMsg;
}

ZqPolynomial Encryption::encrypt(const char fstring[]) {
    int l = -1; while(fstring[++l] != 0) {}
    ZpPolynomial msg(this->N, this->p, fstring, l);
    ZqPolynomial encryptedMsg = msg.encrypt(this->publicKey);
    return encryptedMsg;
}

ZpPolynomial Encryption::decrypt(const ZqPolynomial& e_msg) {
    ZpPolynomial msg = mods_p(e_msg*this->privateKey);
    msg = msg*this->privateKeyInv_p;
    return msg;
}

void Encryption::savePrivateKey_txt() {
    char buff[301];
    std::ofstream file;
    buff[300] = 0;

    this->privateKey.toBytes(buff);
    file.open("PrivateKey.txt");
    if(file.is_open()) {
        file.write(buff, 301);
        file.close();
    } else {
        throw "File could not be written.";
    }
}

void Encryption::setKeys() {
    ZpPolynomial  Zp_privateKey(this->N, this->d+1, this->d);
    ZpPolynomial  Zp_privateKeyInv(this->N, this->q);
	ZpPolynomial  Zp_gcdXNmns1(this->N, this->q);
	ZqPolynomial  Zq_privateKey(this->N, this->q);
	ZqPolynomial  Zq_privateKeyInv(this->N, this->q);
	ZqPolynomial::Z2Polynomial Z2_privateKey(Zp_privateKey);
	ZqPolynomial::Z2Polynomial Z2_privateKeyInv(this->N, 0);
	ZqPolynomial::Z2Polynomial Z2_gcdXNmns1(this->N, 0);
	int counter, k = 2, l = 1;

	try{
	    Zp_gcdXNmns1 = Zp_privateKey.gcdXNmns1(Zp_privateKeyInv);
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption::setKeys()\n";
        throw;
	}
	try{
	    Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv);
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption::setKeys()\n";
        throw;
	}

	counter = 1;
    while(Zp_gcdXNmns1 != 1 || Z2_gcdXNmns1 != 1) {
        if((counter & 3) != 0) {                                                // If we have not tried to much times, just permute the coefficients
            Zp_privateKey.permute();                                            // counter & 3 == counter % 4
	        Z2_privateKey = Zp_privateKey;                                      // ...
        } else {
	        this->d--;                                                          // To much tries, increasing the numbers of non-zero coefficients
            Zp_privateKey = ZpPolynomial(this->N, this->d+1, this->d);          // ...
            Z2_privateKey = Zp_privateKey;                                      // ...
	    }
	    try{ Zp_gcdXNmns1=Zp_privateKey.gcdXNmns1(Zp_privateKeyInv); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRUencryption::setKeys()\n";
            throw;
	    }
	    try{ Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRUencryption::setKeys()\n";
            throw;
    	}
	    counter++;
	}

    Zq_privateKey    = Zp_privateKey;
	Zq_privateKeyInv = ZqPolynomial(Z2_privateKeyInv, this->q);

	while(k < this->q) {
	    Zq_privateKeyInv = Zq_privateKeyInv*(2 - Zq_privateKey*Zq_privateKeyInv);
        k <<= l; l <<= 1;
	}                                                                           // -At this line, we have just created the private key and its inverse
	if(Zq_privateKeyInv*Zp_privateKey == 1) {
	    if(Zp_privateKey*Zp_privateKeyInv == 1) {
	        if(counter > 1)
	            std::cout << "Private key was found after " <<counter<< " attempts.\n";
	        else
	            std::cout << "Private key was found after "<< counter << " attempt.\n";
	    } else {
	        (Zp_privateKey*Zp_privateKeyInv).println("this->privateKey*this->privateKeyInv_p");
	        throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zp[x]/(x^N-1) ring not found.\n";
	    }
	} else {
	    (Zp_privateKey*Zp_privateKeyInv).println("Zq_privateKeyInv*this->privateKey");
	    throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zq[x]/(x^N-1) ring not found\n";
	}
	this->privateKey      = Zp_privateKey;
	this->privateKeyInv_p = Zp_privateKeyInv;
	this->publicKey       = Zq_privateKeyInv.getNTRUpublicKey();
}
