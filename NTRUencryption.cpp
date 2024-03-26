#include"NTRUencryption.hpp"

NTRUencryption::NTRUencryption(NTRU_N _N_,NTRU_q _q_, int _d_, NTRU_p _p_):
N(_N_), q(_q_), d(_d_), p(_p_), privateKey(_N_,_d_+ 1, _d_) {
	this->privateKey.println("Private key");*/

	/*NTRU_ZpPolynomial Np0(_N_,_d_+ 21, _d_+ 2), Np1(_N_, _d_- 1, _d_- 20);
	NTRU_ZpPolynomial quorem[2], Bezout[2], gcd;

	std::cout << '\n';
	Np0.println("\nNp0");
	Np1.println("\nNp1");

    try {Np0.division(Np1, quorem);}
	catch(const char* exp) {std::cout << exp;}

    quorem[0].println("\nquotient");
	quorem[1].println("\nremainder");

	if(Np1*quorem[0] + quorem[1] == Np0 && Np1.degree() > quorem[1].degree())
	    std::cout << "\nSuccesful division.\n";

	try{ gcd = Np0.gcdXNmns1(Bezout); gcd.println("\ngcd(Np0,x^N-1)"); }
	catch(const char* exp) {std::cout << exp;}*/

	this->setPrivateKeyAndInv();
}

void NTRUencryption::setPrivateKeyAndInv() {
    privateKey = NTRU_ZpPolynomial::ZpPolModXNmns1(this->N, this->N / 3 + 1, this->N / 3);
    NTRU_ZpPolynomial::ZpPolModXNmns1 inverse(this->N);
	NTRU_ZpPolynomial _gcdXNmns1_ = privateKey.gcdXNmns1(inverse);

    while(_gcdXNmns1_ != 1) {
	    _gcdXNmns1_.println("\n_gcdXNmns1_");
	    privateKey.permute();
	    _gcdXNmns1_ = privateKey.gcdXNmns1(inverse);
	}
	_gcdXNmns1_.println("\n_gcdXNmns1_");
}
