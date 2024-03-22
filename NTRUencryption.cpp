#include"NTRUencryption.hpp"
#include<random>
#include<ctime>

const int NTRUencryption::ZpPolymodXminus1::Z3addition[3][3] = {{0, 1, 2},      // Addition table of the Z3 ring (integers modulo 3)
                                                                {1, 2, 0},      // ...
                                                                {2, 0, 1}};     // ...

const int NTRUencryption::ZpPolymodXminus1::Z3subtraction[3][3] = {{0, 2, 1},   // Addition table of the Z3 ring (integers modulo 3)
                                                                   {1, 0, 2},   // ...
                                                                   {2, 1, 0}};  // ...

const int NTRUencryption::ZpPolymodXminus1::Z3product[3][3] = {{0, 0, 0},       // Product table of the Z3 ring (integers modulo 3)
                                                               {0, 1, 2},       // ...
                                                               {0, 2, 1}};      // ...
static unsigned _seed_ = (unsigned)time(NULL);

class RandInt {                                                                 // Little class for random integers. Taken from The C++ Programming Language 4th
    public:                                                                     // Edition Bjarne Stroustrup
    RandInt(int low, int high, unsigned seed): re{seed}, dist{low,high} {}
    int operator()() { return dist(re); }                                       // Draw an int
    private:
    std::default_random_engine re;
    std::uniform_int_distribution<> dist;
};


NTRUencryption::ZpPolymodXminus1::ZpPolymodXminus1(NTRU_N _N_,int ones,
int negOnes, NTRU_p _p_): N(_N_), p(_p_) {
    int i, j;
    RandInt rn{0, _N_-1, _seed_++};                                             // Random integers from 0 to N-1
    if(ones < 0) ones = -ones;                                                  // Guarding against invalid values of ones and negOnes. In particular the
    if(negOnes < 0) negOnes = -negOnes;                                         // inequality ones + negOnes < N must follow
    while(ones + negOnes >= this->N) {                                          // Dividing by two till getting inside the allowed range
        ones <<= 1;                                                             // ...
        negOnes <<= 1;                                                          // ...
    }
	this->coefficients = new int[_N_];
	for(i = 0; i < _N_; i++) this->coefficients[i] = 0;
	while(ones > 0) {                                                           // Putting the ones first
        j = rn();
        if(this->coefficients[j] == 0) {
            this->coefficients[j] = 1;
            ones--;
        }
	}
	while(negOnes > 0) {                                                        // Then the negative ones
        j = rn();
        if(this->coefficients[j] == 0) {
            this->coefficients[j] = 2;                                          // Two is congruent with -1 modulo 3
            negOnes--;
        }
	}
}                                                                               // Maybe is some room for optimization using JV theorem

void NTRUencryption::ZpPolymodXminus1::setPermutation() {                       // Naive way of setting a permutation
    int i, j, k, *tmp = new int[this->N];
    RandInt rn{0, 0x7FFFFFFF, _seed_++};                                        // Random integers from 0 to the maximum number for and int
    if(this->permutation==NULL) this->permutation = new int[this->N];
    for(i = 0; i < this->N; i++)  tmp[i] = i;
    for(i = 0, j = this->N; i < this->N; i++, j--) {
        k = rn()%j;
        this->permutation[i] = tmp[k];
        tmp[k] = tmp[j-1];
    }
    delete[] tmp;
}
                                                                                // Implementation of the permutation
void NTRUencryption::ZpPolymodXminus1::permute() {
	int i;
	if(this->permutation == NULL) this->setPermutation();
	if(this->coeffCopy   == NULL) {                                             // If there is no a copy, create the copy
	    this->coeffCopy = new int[this->N];
	    for(i = 0; i < this->N; i++)
	        this->coeffCopy[i]=this->coefficients[i];
	}
	for(i = 0; i < this->N; i++)                                                // Permute coefficients
		this->coefficients[i] = this->coeffCopy[permutation[i]];
	for(i = 0; i < this->N; i++)                                                // Copy coefficients
		this->coeffCopy[i] = this->coefficients[i];
}

NTRUencryption::ZpPolymodXminus1 NTRUencryption::ZpPolymodXminus1::operator +
(const ZpPolymodXminus1& P) const{
    NTRUencryption::ZpPolymodXminus1 r(this->max_N(P));                         // Initializing result in the "biggest polynomial ring"
    const NTRUencryption::ZpPolymodXminus1 *small, *big;
    int i;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                            // polynomial with the biggest N

    for(i = 0; i < small->N; i++)
        r.coefficients[i]=Z3addition[this->coefficients[i]][P.coefficients[i]]; // Addition element by element till the smallest degree of the arguments
    for(; i < big->N; i++)
        r.coefficients[i] = big->coefficients[i];                               // Just copying, equivalent to filling with zeros the small polynomial
    return r;
}

NTRUencryption::ZpPolymodXminus1 NTRUencryption::ZpPolymodXminus1::operator -
(const ZpPolymodXminus1& P) const{
    const NTRUencryption::ZpPolymodXminus1 *small, *big;
    NTRUencryption::ZpPolymodXminus1 r(this->max_N(P));                         // Initializing result in the "biggest polynomial ring"
    int i;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                            // polynomial with the biggest N

    for(i = 0; i < small->N; i++)
        r.coefficients[i] =
        Z3subtraction[this->coefficients[i]][P.coefficients[i]];                // Subtraction element by element till the smallest degree of the arguments
    for(; i < big->N; i++)
        r.coefficients[i] = big->coefficients[i];                               // Just copying, equivalent to filling with zeros the small polynomial
    return r;
}

NTRUencryption::ZpPolymodXminus1 NTRUencryption::ZpPolymodXminus1::operator *
(const ZpPolymodXminus1& P) const{
    NTRUencryption::ZpPolymodXminus1 r(this->max_N(P));                         // Initializing with zeros
    const NTRUencryption::ZpPolymodXminus1 *small, *big;
    int i, j, k;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                            // polynomial with the biggest N

    for(i = 0; i < r.N; i++) {                                                  // Convolution process
        k = min(i, small->N);
        for(j = 0; j <= k; j++)
            r.coefficients[i] = Z3addition[ r.coefficients[i] ]
            [ Z3product[ small->coefficients[j] ][ big->coefficients[i-j] ] ];
        for(; j < small->N; j++)
            r.coefficients[i] = Z3addition[ r.coefficients[i] ]
            [Z3product[small->coefficients[j] ][ big->coefficients[r.N+i-j]]];
    }
    return r;
}

NTRUencryption::ZpPolymodXminus1& NTRUencryption::ZpPolymodXminus1::operator -=
(const ZpPolymodXminus1& P) {
    NTRU_N _N_ = this->min_N(P);                                                // The limit will be till the smallest N
    for(int i = 0; i < _N_; i++)
        this->coefficients[i] =
            Z3subtraction[ this->coefficients[i] ][ P.coefficients[i] ];
    return *this;
}

void NTRUencryption::ZpPolymodXminus1::division(const ZpPolymodXminus1& P,
ZpPolymodXminus1 result[2]) const{
    if(P == 0) {
        throw "\nIn NTRUencryption.cpp, function void NTRUencryption::NTRUPoly"
        "nomial::division(const ZpPolymodXminus1 P,ZpPolymodXminus1 result[2])"
        " const. Division by zero...\n";
    }
    NTRU_N _N_ = this->max_N(P);                                                // We'll work in the 'biggest' polynomial ring
    if(*this == 0) {                                                            // Case zero divided by anything
        result[0] = ZpPolymodXminus1(_N_);                                      // Zero polynomial
        result[1] = ZpPolymodXminus1(_N_);                                      // Zero polynomial
        return;
    }

    const int dividendDegree = this->degree();
    const int divisorDegree  = P.degree();
    int degreeDiff;                                                             // Difference between degrees
    int remDeg;                                                                 // Remainder degree
    const int leadCoeffDivsrInv = P.coefficients[divisorDegree];                // In Z3, each element is its own inverse
    int i;                                                                      // For counting

    if(dividendDegree < divisorDegree) {                                        // Case dividend has smaller degree than divisor
        result[0] = 0; result[1] = *this;
        return;
    }
    /*try{
        //std::cout<<'\n'<<P.coefficients[divisorDegree]<<divisorDegree<<'\n';  // Debugging purposes
        leadCoeffDivsrInv = invModq(P.coefficients[divisorDegree]);
    } catch(const char* exp) {
        std::cout << "\nIn NTRUencryption.cpp, function void NTRUencryption::"
        "ZpPolymodXminus1::division(const ZpPolymodXminus1 P,ZpPolymodXminus1 resul"
        "t[2]) const\n";
        throw;
    }*/                                                                         // At this point we know leading coefficient has an inverse in Zq

    degreeDiff = dividendDegree - divisorDegree;                                // At this point we know degreeDiff >= 0
    remDeg = dividendDegree;
    result[1] = ZpPolymodXminus1(_N_);
    result[1].copyCoefficients(*this);                                          // Initializing remainder with dividend (this)
    result[0] = ZpPolymodXminus1(_N_);

    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        //std::cout << "\nremDeg = " << remDeg << ", degreeDiff = "             // Debugging
        //<< degreeDiff << std::endl;                                           // Purposes
        result[0].coefficients[degreeDiff] = Z3product[
        leadCoeffDivsrInv][result[1].coefficients[remDeg]];                     // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] =
            Z3subtraction[result[1].coefficients[i]][Z3product[
            result[0].coefficients[degreeDiff]][P.coefficients[i-degreeDiff]]];
        }

        if(result[1].coefficients[remDeg] != 0)                                 // No congruence with 0 mod q, throwing exception
            throw "\nIn NTRUencryption.cpp, function void NTRUencryption::NTRU"
            "Polynomial::division(const ZpPolymodXminus1 P,ZpPolymodXminus1 "
            "result[2]) const. result[1].coefficients[remDeg] != 0\n";          // At this point we know result[1].coefficients[remDeg] = 0

        while(remDeg >= 0 && result[1].coefficients[remDeg] == 0) remDeg--;     // Updating value of the degree of the remainder
    }
}

NTRUencryption::ZpPolymodXminus1 NTRUencryption::ZpPolymodXminus1::gcdXNminus1( // EEDA will mean Extended Euclidean Division Algorithm
ZpPolymodXminus1 Bezout[2]) const{                                              // Bezout[2] will hold the Bezout coefficients
    ZpPolymodXminus1 gcd;                                                       // Initializing result in the "biggest polynomial ring"
    ZpPolymodXminus1 remainders;
    ZpPolymodXminus1 Bezout_0_Buff[2];
    ZpPolymodXminus1 Bezout_1_Buff[2];
    ZpPolymodXminus1 quoRem[2] = {ZpPolymodXminus1(this->N),
                                  ZpPolymodXminus1(this->N)};
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    int leadCoeff = this->coefficients[deg];                                    // Lead coefficient of this polynomial

    quoRem[0].coefficients[this->N-deg] = leadCoeff;                            // Start of division algorithm between virtual polynomial x^N-1 and this
    for(i = deg-1, j = this->N - 1; i >= 0; i--, j--) {                           // First coefficient of quotient and first subtraction
        quoRem[1].coefficients[j] =
        Z3subtraction[0][Z3product[ leadCoeff ][ this->coefficients[i]] ];
    }
    quoRem[1].coefficients[0] = 2;                                              // Putting the -1 that is at the end of the polynomial x^N-1
    for(i = this->N-1 - deg, j = this->N-1; j >= deg; i = j - deg) {            // Continuing with division algorithm; i is the place of the next coefficient of
        quoRem[0].coefficients[i] =                                             // the quotient, j is the degree of the remainders.
        Z3product[leadCoeff][quoRem[1].coefficients[j]];
        for(k = deg, l = j; k >= 0; k--, l--) {                                 // Multiplication-subtraction step
            quoRem[1].coefficients[l] =
            Z3subtraction[ quoRem[1].coefficients[l] ]
            [Z3product[ quoRem[0].coefficients[i] ][ this->coefficients[k] ]];
        }
        while(quoRem[1].coefficients[j] == 0) {j--;};
    }
    //quoRem[1].coefficients[0] = Z3subtraction[quoRem[1].coefficients[0]][1];    // Subtracting the -1 that is at the end of the polynomial x^N-1
                                                                                // End of division algorithm between virtual polynomial x^N-1 and this

    Bezout[0].N = this->N; Bezout[0] = 1;                                       // u[0] = 1
    Bezout_0_Buff[0].N = this->N; Bezout_0_Buff[0] = 0;                         // u[1] = 0
    Bezout_0_Buff[1].N = this->N; Bezout_0_Buff[1] = 1;                         // u[2] = u[0] - q[0]*u[1] = 1

    Bezout[1].N = this->N; Bezout[1] = 0;                                       // v[0] = 0
    Bezout_1_Buff[0].N = this->N; Bezout_1_Buff[0] = 1;                         // v[1] = 1
    Bezout_1_Buff[1].N = this->N; Bezout_1_Buff[1] = Bezout[0] - quoRem[0];     // v[2] = v[0] - q[0]*v[1] = v[0] - q[0]

    Bezout[0] = Bezout_0_Buff[0];
    Bezout_0_Buff[0] = Bezout_0_Buff[1];

    //std::cout << "\nline 220::Bezout[1].getN() = " << Bezout[1].getN() << ", Bezout_1_Buff[0].getN() = " << Bezout_1_Buff[0].getN() << std::endl; // Debugging

    Bezout[1] = Bezout_1_Buff[0];
    Bezout_1_Buff[0] = Bezout_1_Buff[1];

    gcd = *this;
    remainders = quoRem[1];
	while(remainders != 0) {
        try{ gcd.division(remainders, quoRem); }
        catch(const char* exp) {
            std::cout << "\nIn NTRUencryption.cpp; function NTRUencryption::"
            "ZpPolymodXminus1::gcd(const ZpPolymodXminus1& P) const\n";
            throw;
        }
        std::cout << "quoRem[0].degree() = " << quoRem[0].degree() << "\n";
        std::cout << "Bezout_0_Buff[0].degree() = " << Bezout_0_Buff[0].degree() << "\n";
        std::cout << "Bezout_1_Buff[0].degree() = " << Bezout_1_Buff[0].degree() << "\n";
        Bezout_0_Buff[1] = Bezout[0] - quoRem[0]*Bezout_0_Buff[0];              // u[k+2] = u[k] - q[k+2]*u[k+1]
        Bezout_1_Buff[1] = Bezout[1] - quoRem[0]*Bezout_1_Buff[0];              // v[k+2] = v[k] - q[k+2]*v[k+1]
        Bezout[0] = Bezout_0_Buff[0];
        Bezout[1] = Bezout_1_Buff[0];
        Bezout_0_Buff[0] = Bezout_0_Buff[1];
        Bezout_1_Buff[0] = Bezout_1_Buff[1];
        gcd = remainders;
        remainders = quoRem[1];
	}
	return gcd;
}

void NTRUencryption::ZpPolymodXminus1::print(const char* name) const{
    char start[] = "0   [";                                                     // Start of the string will be printed
    char numBuf[10];                                                            // Buffer necessary for the int -> string conversion
    int qlen, strLen;                                                           // q length in characters, start length in characters
    int i = 0, j = 0;
    int deg = this->degree();
    int startLen;

    intToString((int)this->p, numBuf);                                          // Conversion from number to string
    qlen   = len(numBuf) + 1;
    startLen = len(start);

    std::cout << name << " = \n";
    std::cout << start;
    do {
        if(i != 0 && (i & 31) == 0 && i != deg) {                               // Since 2^5 = 32, then i&31 = i % 32
            std::cout << '\n';
            j++; intToString(j, numBuf);
            std::cout << numBuf;                                                // Printing current coefficient
            strLen = len(numBuf);
            printSpaces((unsigned)this->max(startLen - strLen,0));              // Padding with spaces
        }
        intToString(this->coefficients[i], numBuf);                             // Optimize by returning the length of the string
        std::cout << numBuf;                                                    // Printing current coefficient
        strLen = len(numBuf);
        printSpaces((unsigned)this->max(qlen - strLen ,0));                     // Padding with spaces
        if(i < deg) std::cout << ',';
    }while(++i <= deg);
    std::cout << ']';
}

/*int NTRUencryption::ZpPolymodXminus1::invModq(int t) const{
    const int exp = (this->q >> 1) - 1;											// exp = q/2 - 1
	int bit = 1;															    // Single bit; it will 'run' trough the bits of exp
	int r = (t = this->modq(t));                                                // Making sure 0 <= t < q. Assigning to r
	if((t & 1) == 0) throw "\nIn NTRUencryption.cpp, function int NTRUencrypti"
	    "on::invModq(int t). No inverse modulus q for even numbers.\n";
	for(; (exp & bit) != 0; bit <<= 1) {                                        // Using exponentiation algorithm to find the inverse
		(r *= r) *= t;                                                          // Enhanced for this particular case (q-1 has the form 111...111)
		r = this->modq(r);
	}
	return r;
}*/

/*void NTRUencryption::ZpPolymodXminus1::thisCoeffOddRandom(int deg) {
    RandInt rn{0, (this->q-1)>>2, _seed_++};                                    // Random integers from 0 to (q-1)/4
    if(deg < 0 || deg >= this->N) deg = this->N - 1;
    for(int i = 0; i <= deg; i++)
        this->coefficients[i] = (rn()<<1) + 1;                                  // Assigning rn()*2 + 1. This number is odd, bigger than zero and smaller than q
}*/

NTRUencryption::NTRUencryption(NTRUencryption::NTRU_N _N_, NTRUencryption::
NTRU_q _q_, int _d_, NTRU_p _p_): N(_N_), q(_q_), d(_d_), p(_p_) {
    /*this->privateKey = ZpPolymodXminus1(_N_,_d_+ 1, _d_);                     // Polynomial f
    this->privateKey.println("Private key");*/

    /*ZpPolymodXminus1 Np0(_N_,_d_+ 21, _d_+ 2), Np1(_N_, _d_- 1, _d_- 20);
    ZpPolymodXminus1 quorem[2], Bezout[2], gcd;

    std::cout << '\n';
    Np0.println("\nNp0");
    Np1.println("\nNp1");

    try {Np0.division(Np1, quorem);}
    catch(const char* exp) {std::cout << exp;}

    quorem[0].println("\nquotient");
    quorem[1].println("\nremainder");

    if(Np1*quorem[0] + quorem[1] == Np0 && Np1.degree() > quorem[1].degree())
        std::cout << "\nSuccesful division.\n";

    try{ gcd = Np0.gcdXNminus1(Bezout); gcd.println("\ngcd(Np0,x^N-1)"); }
    catch(const char* exp) {std::cout << exp;}*/

    this->setPrivateKeyAndInv();
}

void NTRUencryption::setPrivateKeyAndInv() {
    privateKey = ZpPolymodXminus1(this->N, this->N / 3 + 1, this->N / 3);
    ZpPolymodXminus1 bezout[2];
    ZpPolymodXminus1 _gcdXminus1_ = privateKey.gcdXNminus1(bezout);

    while(_gcdXminus1_ != 1) {
        _gcdXminus1_.println("\n_gcdXminus1_");
        privateKey.permute();
        _gcdXminus1_ = privateKey.gcdXNminus1(bezout);
    }
    _gcdXminus1_.println("\n_gcdXminus1_");
    bezout[1].println("private key inverse");
    (bezout[1]*privateKey).println("privateKey*bezout[1]");
}
