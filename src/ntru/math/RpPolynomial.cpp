#include"../../../include/ntru/polynomials.hpp"
#include"../../../include/ntru/parameters_constants.hpp"
#include"../../../include/print_debug/print_array_table.hpp"

using namespace NTRU;

RpPolynomial::RpPolynomial() {
	this->coefficients = new Z3[NTRU_N];
	for(int i = 0; i < NTRU_N; i++) this->coefficients[i] = _0_;
}

RpPolynomial::RpPolynomial(const RpPolynomial& P) {
	this->coefficients = new Z3[NTRU_N];
	for(int i = 0; i < NTRU_N; i++) this->coefficients[i] = P.coefficients[i];
}

RpPolynomial& RpPolynomial::operator = (const RpPolynomial& P) {
    if(this != &P) {													        // Guarding against self assignment
        if(this->coefficients != NULL) delete[] this->coefficients;
        this->coefficients = new Z3[NTRU_N];
	    for(int i = 0; i < NTRU_N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

int RpPolynomial::operator [] (int i) const{
	if(i < 0) i = -i;
	if(i > NTRU_N) i %= NTRU_N;
	return this->coefficients[i];
}

int RpPolynomial::degree() const{
    int deg = NTRU_N;
	while(this->coefficients[--deg] == 0 && deg > 0) {}
	return deg;
}

void RpPolynomial::print(const char* name, bool centered, const char* tail) const{
    short coeffs[NTRU_N];
    int coeffAmount = this->degree() + 1, i;
    for(i = 0; i < coeffAmount; i++) {
        if(this->coefficients[i] == 2) {
            if(centered) coeffs[i] = -1;                                        // Printing the polynomials with coefficient in {-1, 0, 1}
            else coeffs[i] = 2;
        }
        else coeffs[i] = this->coefficients[i];                                 // ...
    }
    print_table<short>(coeffs, (size_t)this->degree()+1, name, tail, 3);
}

void RpPolynomial::println(const char* name, bool centered) const{
	this->print(name, centered, "\n");
}

#if NTRU_HAS_GMPXX
// GMPXX-dependent implementations
mpz_class RpPolynomial::toNumber() const {
    mpz_class r = 0, base = 3;
    for(int i = NTRU_N-1; i >= 0; i--) r = r*base + this->coefficients[i];      // -Horner's algorithm
    return r;
}

/*RpPolynomial RpPolynomial::fromNumber(const mpz_class& num) {
    RpPolynomial result;
    mpz_class temp = num;

    // Implementation details...
    return result;
}*/
#endif // NTRU_HAS_GMPXX
