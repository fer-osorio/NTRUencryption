#include"NTRUencryption.hpp"

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator +
(const NTRUPolynomial& t) const {
    NTRUencryption::NTRUPolynomial r;                                           // Initializing with zeros
    for(int i = 0; i < N; i++)
        r.coefficients[i] = modq(this->coefficients[i] + t.coefficients[i]);       // Addition element by element. The addition is performed in the Zp group
    return r;
}

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator *
(const NTRUPolynomial& t) const {
    NTRUencryption::NTRUPolynomial r;                                           // Initializing with zeros
    int i, j;
    for(i = 0; i < N; i++) {                                                    // Convolution process
        for(j = 0; j <= i; j++)
            r.coefficients[i] += modq(this->coefficients[j]*t.coefficients[i-j]);  // Some optimization through a 64 bits buffer can be implemented here
        for(; j < N; j++)
            r.coefficients[i] += modq(this->coefficients[j]*t.coefficients[N+i-j]);// And here
    }
    return r;
}

void NTRUencryption::NTRUPolynomial::division(const NTRUPolynomial t,
NTRUPolynomial result[2]) const {
    if(t == 0) {
        throw "\nIn NTRUencryption.cpp, function void NTRUencryption::NTRUPolyn"
        "omial::division(const NTRUPolynomial t,NTRUPolynomial result[2]) const"
        ". Division by zero...\n";
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

int NTRUencryption::invModq(int t) {
    const int exp = (q >> 1) - 1;											    // exp = q/2 - 1
	int bit = 1;															    // Single bit; it will 'run' trough the bits of exp
	int r = (t = modq(t));                                                      // Making sure 0 <= t < q. Assigning to r
	if((t & 1) == 0) throw "\nIn NTRUencryption.cpp, function int NTRUencrypti"
	    "on::invModq(int t). No inverse modulus q for even numbers.\n";
	for(; (exp & bit) != 0; bit <<= 1) {                                        // Using exponentiation algorithm to find the inverse
		(r *= r) *= t;                                                          // Enhanced for this particular case (q-1 has the form 111...111)
		r = modq(r);
	}
	return r;
}
