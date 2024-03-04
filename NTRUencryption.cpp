#include"NTRUencryption.hpp"

NTRUencryption::NTRUPolynomial NTRUencryption::NTRUPolynomial::operator +
(const NTRUPolynomial& t) const{
    NTRUencryption::NTRUPolynomial r;
    for(int i = 0; i < N; i++) {
        r.coefficients[i] = this->coefficients[i] + t.coefficients[i];
        if(r.coefficients[i] >= 0) r.coefficients[i] &= q-1;                      // Since q is a power of 2, &= q-1 is equivalent to %= q
        else if(r.coefficients[i] <= -q)
                r.coefficients[i] = -(-r.coefficients[i] & q-1);                  // In the negative case, we first change to positive and then we apply modulus
    }
    return r;
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
