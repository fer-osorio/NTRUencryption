#include"NTRUencryption.hpp"

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator +
(const NTRUPolynomial& t) const {
    NTRUencryption::NTRUPolynomial r(this->N, this->q);                         // Initializing with zeros
    for(int i = 0; i < r.N; i++)
        r.coefficients[i] = modq(this->coefficients[i] + t.coefficients[i]);       // Addition element by element. The addition is performed in the Zp group
    return r;
}

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator *
(const NTRUPolynomial& t) const {
    NTRUencryption::NTRUPolynomial r(this->N, this->q);                         // Initializing with zeros
    int i, j;
    for(i = 0; i < r.N; i++) {                                                  // Convolution process
        for(j = 0; j <= i; j++)
            r.coefficients[i] += modq(this->coefficients[j]*t.coefficients[i-j]);  // Some optimization through a 64 bits buffer can be implemented here
        for(; j < N; j++)
            r.coefficients[i]+=modq(this->coefficients[j]*t.coefficients[N+i-j]);  // And here
    }
    return r;
}

void NTRUencryption::NTRUPolynomial::division(const NTRUPolynomial t,
NTRUPolynomial result[2]) const {
    if(t == 0) {
        throw "\nIn NTRUencryption.cpp, function void NTRUencryption::NTRUPoly"
        "nomial::division(const NTRUPolynomial t, NTRUPolynomial result[2]) co"
        "nst. Division by zero...\n";
        return;
    }
    if(*this == 0) {                                                            // Case zero divided by anything
        result[0] = 0; result[1] = 0;
        return;
    }

    const int dividendDegree = this->degree();
    const int divisorDegree  = t.degree();
    int degreeDiff;                                                              // Difference between degrees
    int remDeg;                                                                 // Remainder degree
    int leadCoeffDivsrInv;                                                       // Inverse (modulus q) of leading coefficient of the divisor
    int i;                                                                      // For counting

    if(dividendDegree < divisorDegree) {                                        // Case dividend has smaller degree than divisor
        result[0] = 0; result[1] = *this;
        return;
    }
    try{
        leadCoeffDivsrInv = invModq(t.coefficients[dividendDegree]);
    } catch(char* excp) {
        std::cout << "\nIn NTRUencryption.cpp, function void NTRUencryption::"
        "NTRUPolynomial::division(const NTRUPolynomial t,NTRUPolynomial resul"
        "t[2]) const\n";
        throw excp;
    }                                                                           // At this point we know leading coefficient has an inverse in Zq
    degreeDiff = dividendDegree - divisorDegree;                                 // At this point we know degreeDiff >= 0
    remDeg = divisorDegree;
    result[1] = *this;                                                          // Initializing remainder with dividend (this)
    result[0] = 0;                                                              // Setting quotient as zero

    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        result[0].coefficients[degreeDiff] =
        leadCoeffDivsrInv * result[1].coefficients[remDeg];                       // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--)
            result[1].coefficients[i] -= modq(
            result[0].coefficients[degreeDiff] * t.coefficients[i - degreeDiff]);   // Updating remainder

        while(result[1].coefficients[remDeg] < 0)
            result[1].coefficients[remDeg] += q;                                 // In case of negative difference (congruent with 0 mod q)

        if(result[1].coefficients[remDeg] != 0)                                  // No congruence with 0 mod q, throwing exception
            throw "\nIn NTRUencryption.cpp, function void NTRUencryption::NTRU"
            "Polynomial::division(const NTRUPolynomial t,NTRUPolynomial result"
            "[2]) const. result[1].coefficients[remDeg] != 0\n";                 // At this point we know result[1].coefficients[remDeg] = 0

        while(remDeg > 0 && result[1].coefficients[--remDeg] == 0) {}            // Updating value of the degree of the remainder
    }
}

void NTRUencryption::NTRUPolynomial::print() const{
    char start[] = "0   [";                                                     // Start of the string will be printed
    char numBuf[10];                                                            // Buffer necessary for the int -> string conversion
    int qlen, strLen;                                                           // q length in characters, start length in characters
    int i = 0,j = 0;
    int deg = this->degree();
    uintToString((unsigned)q, numBuf);                                          // Conversion from number to string
    qlen =  len(numBuf);
    strLen = len(start);
    std::cout << start;
    do {
        uintToString(this->coefficients[i], numBuf);                             // Optimize by returning the length of the string
        strLen = len(numBuf);
        std::cout << numBuf;                                                    // Printing current coefficient
        printSpaces(unsigned(qlen - strLen));                                   // Padding with spaces
        if(i < deg) std::cout << ',';
        if(i != 0 && (i & 31) == 0 && i != deg) std::cout << '\n';              // Since 2^5 = 32, then i&31 = i % 32
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

int uintToString(unsigned n, char* dest) {
    unsigned i = 0, j = 0;
    char buff = 0;
    do {
        buff = (char)(n % DECIMAL_BASE);                                         // Taking last current digit
        dest[i++] = buff + 48;                                                   // Saving last current digit
        n -= (unsigned)buff; n /= DECIMAL_BASE;                                  // Taking out last current digit from the number n
    } while(n > 0);
    dest[i--] = 0;                                                              // Putting a zero at the end and returning one place
    for(; j < i; j++,i--) {                                                     // The number is backwards; reversing the order of the digits
        buff = dest[j];
        dest[j] = dest[i];
        dest[i] = buff;
    }
    return 0;
}
