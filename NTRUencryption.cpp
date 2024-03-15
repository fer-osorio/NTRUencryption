#include"NTRUencryption.hpp"
#include<random>
#include<ctime>

static unsigned _seed_ = (unsigned)time(NULL);

class RandInt {                                                                 // Little class for random integers. Taken from The C++ Programming Language 4th
    public:                                                                     // Edition Bjarne Stroustrup
    RandInt(int low, int high, unsigned seed): re{seed}, dist{low,high} {}
    int operator()() { return dist(re); }                                       // Draw an int
    private:
    std::default_random_engine re;
    std::uniform_int_distribution<> dist;
};

NTRUencryption::NTRUencryption(NTRUencryption::NTRU_N _N_, NTRUencryption::
NTRU_q _q_): N(_N_), q(_q_) {
    //int upperBound = _q_ >> 1, i, k, inv_k;
    NTRUPolynomial Np0(_N_, _q_), Np1(_N_, _q_);
    NTRUPolynomial quorem[2];
    int sz = this->N >> 2;
    int szplus = sz + 100;
    //Np0.println();
    //Np0.coefficients[szplus] = 3;
    //Np0.println(); std::cout << '\n';
    /*for(i = 0; i < upperBound; i++) {
        k = (i << 1) + 1;
        inv_k = Np0.invModq(k);
        std::cout << "(" << k << ")^-1 = " << inv_k << " | ";
        std::cout << k << "*" << inv_k << " mod " << _q_ << " = ";
        std::cout << Np0.modq(k*inv_k) << '\n';
        if(Np0.modq(k*inv_k) != 1) std::cout << "\nSomething went wrong...\n";
    }*/
    std::cout << '\n';
    Np0.thisCoeffOddRandom(szplus);
    Np1.thisCoeffOddRandom(sz);
    Np0.println("\nNp0");
    Np1.println("\nNp1");
    try {Np0.division(Np1, quorem);}
    catch(const char* exp) {std::cout << exp;}
    quorem[0].println("\nquotient");
    quorem[1].println("\nremainder");
    if(Np1*quorem[0] + quorem[1] == Np0)
        std::cout << "\nSuccesful division.\n";
}

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator+
(const NTRUPolynomial& P) const{
    NTRUencryption::NTRUPolynomial r(this->max_N(P), this->max_q(P));           // Initializing polynomial with the biggest coefficients
    const NTRUencryption::NTRUPolynomial *small, *big;
    int i;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = &P; }                                              // polynomial with the biggest N

    for(i = 0; i < small->N; i++)
        r.coefficients[i] = r.modq(this->coefficients[i] + P.coefficients[i]);  // Addition element by element till the smallest degree of the arguments
    for(; i < big->N; i++)
        r.coefficients[i] = big->coefficients[i];                               // Just copying, equivalent to filling with zeros the small polynomial
    return r;
}

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator*
(const NTRUPolynomial& P) const{
    NTRUencryption::NTRUPolynomial r(this->max_N(P), this->max_q(P));           // Initializing with zeros
    const NTRUencryption::NTRUPolynomial *small, *big;
    int i, j, k;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                              // polynomial with the biggest N

    for(i = 0; i < r.N; i++) {                                                  // Convolution process
        k = min(i, small->N);
        for(j = 0; j <= k; j++) r.coefficients[i]+=                             // Some optimization through a 64 bits buffer can be implemented here
            r.modq(small->coefficients[j] * big->coefficients[i-j]);
        for(; j < small->N; j++) r.coefficients[i] +=                           // And here
            r.modq(small->coefficients[j] * big->coefficients[r.N + i-j]);
        r.coefficients[i] = r.modq(r.coefficients[i]);
    }
    return r;
}

void NTRUencryption::NTRUPolynomial::division(const NTRUPolynomial P,
NTRUPolynomial result[2]) const {
    if(P == 0) {
        throw "\nIn NTRUencryption.cpp, function void NTRUencryption::NTRUPoly"
        "nomial::division(const NTRUPolynomial P, NTRUPolynomial result[2]) co"
        "nst. Division by zero...\n";
    }
    NTRU_N _N_ = this->max_N(P);
    NTRU_q _q_ = this->max_q(P);                                                // We'll work in the 'biggest' polynomial ring
    if(*this == 0) {                                                            // Case zero divided by anything
        result[0] = NTRUPolynomial(_N_,_q_);                                    // Zero polynomial
        result[1] = NTRUPolynomial(_N_,_q_);                                    // Zero polynomial
        return;
    }

    const int dividendDegree = this->degree();
    const int divisorDegree  = P.degree();
    int degreeDiff;                                                             // Difference between degrees
    int remDeg;                                                                 // Remainder degree
    int leadCoeffDivsrInv;                                                      // Inverse (modulus q) of leading coefficient of the divisor
    int i;                                                                      // For counting

    if(dividendDegree < divisorDegree) {                                        // Case dividend has smaller degree than divisor
        result[0] = 0; result[1] = *this;
        return;
    }
    try{
        //std::cout<<'\n'<<P.coefficients[divisorDegree]<<divisorDegree<<'\n';  // Debugging purposes
        leadCoeffDivsrInv = invModq(P.coefficients[divisorDegree]);
    } catch(const char* exp) {
        std::cout << "\nIn NTRUencryption.cpp, function void NTRUencryption::"
        "NTRUPolynomial::division(const NTRUPolynomial P,NTRUPolynomial resul"
        "t[2]) const\n";
        throw;
    }                                                                           // At this point we know leading coefficient has an inverse in Zq
    degreeDiff = dividendDegree - divisorDegree;                                // At this point we know degreeDiff >= 0
    remDeg = dividendDegree;
    result[1] = NTRUPolynomial(_N_, _q_);
    result[1].copyCoefficients(*this);                                          // Initializing remainder with dividend (this)
    result[0] = NTRUPolynomial(_N_, _q_);

    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        //std::cout << "\nremDeg = " << remDeg << ", degreeDiff = "             // Debugging
        //<< degreeDiff << std::endl;                                           // Purposes
        result[0].coefficients[degreeDiff] = result[0].modq(
        leadCoeffDivsrInv * result[1].coefficients[remDeg]);                    // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] -= result[0].coefficients[degreeDiff]*
            P.coefficients[i-degreeDiff];
            result[1].coefficients[i]=result[1].modq(result[1].coefficients[i]);
        }

        while(result[1].coefficients[remDeg] < 0)
            result[1].coefficients[remDeg] += this->q;                          // In case of negative difference (congruent with 0 mod q)

        if(result[1].coefficients[remDeg] != 0)                                 // No congruence with 0 mod q, throwing exception
            throw "\nIn NTRUencryption.cpp, function void NTRUencryption::NTRU"
            "Polynomial::division(const NTRUPolynomial P,NTRUPolynomial result"
            "[2]) const. result[1].coefficients[remDeg] != 0\n";                // At this point we know result[1].coefficients[remDeg] = 0

        while(remDeg >= 0 && result[1].coefficients[remDeg] == 0) remDeg--;     // Updating value of the degree of the remainder
    }
}

void NTRUencryption::NTRUPolynomial::print(const char* name) const{
    char start[] = "0   [";                                                     // Start of the string will be printed
    char numBuf[10];                                                            // Buffer necessary for the int -> string conversion
    int qlen, strLen;                                                           // q length in characters, start length in characters
    int i = 0, j = 0;
    int deg = this->degree();
    uintToString((unsigned)q, numBuf);                                          // Conversion from number to string
    qlen =  len(numBuf);
    strLen = len(start);
    std::cout << name << " = \n";
    std::cout << start;
    do {
        if(i != 0 && (i & 31) == 0 && i != deg) {                               // Since 2^5 = 32, then i&31 = i % 32
            std::cout << '\n';
            j++; uintToString((unsigned)j, numBuf);
            std::cout << numBuf;                                                // Printing current coefficient
            strLen = len(numBuf);
            printSpaces((unsigned)this->max(qlen - strLen + 1,0));              // Padding with spaces
        }
        uintToString((unsigned)this->coefficients[i], numBuf);                  // Optimize by returning the length of the string
        std::cout << numBuf;                                                    // Printing current coefficient
        strLen = len(numBuf);
        printSpaces((unsigned)this->max(qlen - strLen ,0));                     // Padding with spaces
        if(i < deg) std::cout << ',';
    }while(++i <= deg);
    std::cout << ']';
}

int NTRUencryption::NTRUPolynomial::invModq(int t) const{
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
}

void NTRUencryption::NTRUPolynomial::thisCoeffOddRandom(int deg) {
    RandInt rn{0, (q-1)>>2, _seed_++};                                          // Random integers from 0 to (q-1)/2
    if(deg < 0 || deg >= this->N) deg = this->N - 1;
    for(int i = 0; i <= deg; i++)
        this->coefficients[i] = (rn()<<1) + 1;                                  // Assigning rn()*2 + 1. This number is odd, bigger than zero and smaller than q
}
