#include"NTRUencryption.hpp"

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator +
(const NTRUPolynomial& t) const{
    NTRUencryption::NTRUPolynomial r;                                           // Initializing with zeros
    for(int i = 0; i < N; i++) {
        r.coefficients[i] = modq(this->coefficients[i] + t.coefficients[i]);       // Addition element by element. The addition is performed in the Zp group
    }
    return r;
}

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator *
(const NTRUPolynomial& t) const{
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
}

NTRUencryption::NTRUPolynomial& NTRUencryption::NTRUPolynomial::operator =
(const NTRUPolynomial& t) {
    if(this != &t) {                                                            // Guarding against self assignment
        delete [] this->coefficients;
        this->coefficients = new int[N];                                         // Creating array in the heap and updating degree
        for(int i=0; i < N; i++) this->coefficients[i] = t.coefficients[i];
    }
    return *this;
}

int NTRUencryption::modq(int t) {
    while(t < 0) t += q;                                                        // Adding q till we get t positive. This won't affect the result
    return t &= q-1;                                                            // Since q is a power of 2, the expression t &= q-1 is equivalent to t %= q
}
