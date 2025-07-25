#include"../../include/ntru/polynomials.hpp"
#include"../../include/ntru/exceptions.hpp"
#include"../core/parameter_validation.hpp"
#include"../utils/print.hpp"

using namespace NTRU;

R2Polynomial::R2Polynomial() {
	this->coefficients = new Z2[_N_];
	for(int i = 0; i < _N_; i++) this->coefficients[i] = _0_;
}

R2Polynomial::R2Polynomial(const R2Polynomial& P) {
    this->coefficients = new Z2[_N_];
    for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
}

R2Polynomial::R2Polynomial(const RpPolynomial& P) {
	this->coefficients = new Z2[_N_];
	for(int i = 0; i < _N_; i++) {
	    if(P[i] != RpPolynomial::_0_) this->coefficients[i] = _1_;              // Two won't go to zero because it's the additive inverse of one in Z3, therefore
	    else this->coefficients[i] = _0_;                                       // it must go to the additive inverse of one in Z2, which if itself
    }
}

R2Polynomial& R2Polynomial::operator = (const R2Polynomial& P)  {
	if(this != &P) {													        // Guarding against self assignment
		if(this->coefficients != NULL) delete[] this->coefficients;
		this->coefficients = new Z2[_N_];
		for(int i = 0; i < _N_; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

R2Polynomial& R2Polynomial::operator = (const RpPolynomial& P) {
	if(this->coefficients != NULL) delete[] this->coefficients;                 // In case of having an object created with the default (private) constructor
	this->coefficients = new Z2[_N_];
	for(int i = 0; i < _N_; i++) {                                              // Fitting the RpPolynomial in a R2Polynomial
	    if(P[i] != RpPolynomial::_0_) this->coefficients[i] = _1_;
	    else this->coefficients[i] = _0_;
	}
	return *this;
}

R2Polynomial& R2Polynomial::operator = (Z2 t)  {
	if(this->coefficients != NULL) delete[] this->coefficients;                 // In case of have been generated from the (private) default constructor
	this->coefficients = new Z2[_N_];
	this->coefficients[0] = t;
	for(int i = 1; i < _N_; i++) this->coefficients[i] = _0_;
	return *this;
}

R2Polynomial R2Polynomial::operator + (const R2Polynomial& P) const {
    R2Polynomial r;                                                             // -Initializing result with zeros
    for(int i = 0; i < _N_; i++)
        r.coefficients[i] = this->coefficients[i] + P.coefficients[i];          // -Addition element by element till the smallest degree of the arguments
    return r;
}

R2Polynomial R2Polynomial::operator - (const R2Polynomial& P) const{
    return *this + P;                                                           // In this polynomial ring, subtraction coincide with addition
}

R2Polynomial R2Polynomial::operator * (const R2Polynomial& P) const{ // Classical polynomial multiplication algorithm
    R2Polynomial r;                                                             // -Initializing with zeros
    int i, j, k, l;

	for(i = 0; i < _N_; i++) {
		if(this->coefficients[i] != _0_) {                                      // -Polynomial over binary field, here we know  this->coefficients[i] is 1
		    k = _N_ - i;
		    for(j = 0; j < k; j++) {                                            // -Adding and multiplying while the inequality i+j < N holds
		        l = i+j;
			    if(P.coefficients[j] != r.coefficients[l]) r.coefficients[l] = _1_;
			    else r.coefficients[l] = _0_;
		    }
		    for(k = 0; k < i; j++, k++) {                                       // Using the definition of convolution polynomial rings
			    if(P.coefficients[j] != r.coefficients[k]) r.coefficients[k] = _1_; // Notice i+j = i +(k+N-i), so i+j is congruent with k mod N
			    else r.coefficients[k] = _0_;
		    }
		}
	}
	return r;
}

void R2Polynomial::division(const R2Polynomial& P, R2Polynomial result[2]) const{
    if(P == _0_) {
        throw MathException("In void R2Polynomial::division(const R2Polynomial&, R2Polynomial[]) const: Division by Zero");
    }
    if(*this == _0_) {                                                          // Case zero divided by anything
        result[0] = _0_;                                                        // Zero polynomial
        result[1] = _0_;                                                        // Zero polynomial
        return;
    }
    const int dividendDegree = this->degree();
    const int divisorDegree  = P.degree();
    const Z2 leadCoeffDivsrInv = P.coefficients[divisorDegree];                 // In Z3, each element is its own inverse
    int degreeDiff;                                                             // Difference between degrees
    int remDeg;                                                                 // Remainder degree
    int i;                                                                      // For counting
    if(dividendDegree < divisorDegree) {                                        // Case dividend has smaller degree than divisor
        result[0] = _0_; result[1] = *this;
        return;
    }                                                                           // At this point we know leading coefficient has an inverse in Zq
    degreeDiff = dividendDegree - divisorDegree;                                // At this point we know degreeDiff >= 0
    remDeg = dividendDegree;
    result[1] = *this;                                                          // Initializing remainder with dividend (this)
    result[0] = R2Polynomial();
    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        result[0].coefficients[degreeDiff] =
        leadCoeffDivsrInv * result[1].coefficients[remDeg];                     // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] -=
            result[0].coefficients[degreeDiff]*P.coefficients[i-degreeDiff];
        }
        while(remDeg >= 0 && result[1].coefficients[remDeg] == _0_) remDeg--;   // Updating value of the degree of the remainder
    }
}

R2Polynomial R2Polynomial::gcdXNmns1(R2Polynomial& thisBezout) const{
    R2Polynomial gcd;                                                           // Initializing result with zeros
    R2Polynomial remainders;
    R2Polynomial tmp[2] = {R2Polynomial(), R2Polynomial()};
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    Z2 leadCoeff = this->coefficients[deg];                                     // Lead coefficient of this polynomial
    R2Polynomial quoRem[2]={R2Polynomial(), R2Polynomial()};

    quoRem[0].coefficients[_N_-deg] = leadCoeff;                                // Start of division algorithm between virtual polynomial x^N-1 and this
    for(i = deg-1, j = _N_ - 1; i >= 0; i--, j--) {                             // First coefficient of quotient and first subtraction
        quoRem[1].coefficients[j] = this->coefficients[i];                      // All x in Z2, -x = x
    }
    quoRem[1].coefficients[0] = _1_;                                            // Putting the -1 that is at the end of the polynomial x^N-1. 1 == -1 in Z2
    for(i = _N_-1 - deg, j = _N_-1; j >= deg; i = j - deg) {                        // Continuing with division algorithm; i is the place of the next coefficient of
        quoRem[0].coefficients[i] = leadCoeff * quoRem[1].coefficients[j];      // the quotient, j is the degree of the remainders.
        for(k = deg, l = j; k >= 0; k--, l--) {                                 // Multiplication-subtraction step
            quoRem[1].coefficients[l] -=
            quoRem[0].coefficients[i] * this->coefficients[k];
        }
        while(quoRem[1].coefficients[j] == _0_) {j--;};
    }                                                                           // End of division algorithm between virtual polynomial x^N-1 and this

    thisBezout = _1_;                                                           // Initializing values for the execution of the rest of the EEA
    tmp[1] = quoRem[0];                                                         // In this ring, -a = a for all polynomial a
    tmp[0] = tmp[1];                                                            // v[-1] = 0, v[0] = 1 ==> v[1] = v[-1] - q[1]*v[0] = - q[1]
    gcd = *this;                                                                // ...
    remainders = quoRem[1];                                                     // ...

	while(remainders != _0_) {                                                  // EEA implementation (continuation)
        try{ gcd.division(remainders, quoRem); }
        catch(MathException& exp) {
            exp.add_trace_point("Caught in RpPolynomial::gcdXNmns1(RpPolynomial&) const, while executing division");
            throw;
        }
        tmp[1] = thisBezout - quoRem[0]*tmp[0];                                 // u[k+2] = u[k] - q[k+2]*u[k+1]
        thisBezout = tmp[0];                                                    // Updating values
        tmp[0] = tmp[1];                                                        // ...
        gcd = remainders;                                                       // ...
        remainders = quoRem[1];                                                 // ...
	}
	return gcd;
}

bool R2Polynomial::operator == (const R2Polynomial& P) const{
	for(int i = 0; i < _N_; i++) if(this->coefficients[i] != P.coefficients[i])  return false;
	return true;
}

R2Polynomial::Z2 R2Polynomial::operator[](int i) const{
	if(i < 0) i = -i;
	if(i >= _N_) i %= _N_;
	return this->coefficients[i];
}

int R2Polynomial::degree() const{												// Returns degree of polynomial
	int deg = _N_;
	while(this->coefficients[--deg] == _0_ && deg > 0) {}
	return deg;
}

void R2Polynomial::print(const char* name,const char* tail) const{
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printIntArray(array, (unsigned)coeffAmount, 2, name, tail);
    delete[] array;
}

void R2Polynomial::println(const char* name) const{
    this->print(name, "\n");
}
