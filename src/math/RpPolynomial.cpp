#include"../../include/ntru/polynomials.hpp"
#include"../core/parameter_validation.hpp"
#include"../utils/print.hpp"

using namespace NTRU;

RpPolynomial::RpPolynomial() {
	this->coefficients = new Z3[_N_];
	for(int i = 0; i < _N_; i++) this->coefficients[i] = _0_;
}

RpPolynomial::RpPolynomial(const RpPolynomial& P) {
	this->coefficients = new Z3[_N_];
	for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
}

RpPolynomial& RpPolynomial::operator = (const RpPolynomial& P) {
    if(this != &P) {													        // Guarding against self assignment
        if(this->coefficients != NULL) delete[] this->coefficients;
        this->coefficients = new Z3[_N_];
	    for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

int RpPolynomial::operator [] (int i) const{
	if(i < 0) i = -i;
	if(i > _N_) i %= _N_;
	return this->coefficients[i];
}

int RpPolynomial::degree() const{
    int deg = _N_;
	while(this->coefficients[--deg] == 0 && deg > 0) {}
	return deg;
}

static int64_t multiplyBy_3(int64_t t) {
    return (t << 1) + t;                                                        // -This expression is equivalent to t*2 + t
}

mpz_class RpPolynomial::toNumber() const{                                       // -Interprets this->coefficients as a number in base 3
    mpz_class r = 0, base = 3;
    for(int i = _N_-1; i >= 0; i--) r = r*base + this->coefficients[i];         // -Horner's algorithm
    return r;
}

void RpPolynomial::print(const char* name, bool centered, const char* tail) const{
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    if(centered) for(i = 0; i < coeffAmount; i++) if(array[i] == 2) array[i] = -1; // Printing the polynomials with coefficient in {-1, 0, 1}
    printIntArray(array, (unsigned)coeffAmount, 3, name, tail);
    delete[] array;
}

void RpPolynomial::println(const char* name, bool centered) const{
	this->print(name, centered, "\n");
}
