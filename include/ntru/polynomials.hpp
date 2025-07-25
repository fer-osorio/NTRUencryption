#include<gmpxx.h>

#ifndef POLYNOMIALS_HPP
#define POLYNOMIALS_HPP

namespace NTRU{

class RpPolynomial;
class RqPolynomial;
class R2Polynomial;
class Encryption;

RqPolynomial convolutionRq(const R2Polynomial&, const RpPolynomial&);
RqPolynomial convolutionRq(const R2Polynomial&, const RqPolynomial&);
RqPolynomial convolutionRq(const RpPolynomial&, const RqPolynomial&);
RpPolynomial mods_p(RqPolynomial);

class RpPolynomial {								// -Representation of the polynomials in Rp := Zp[x]/(x^N-1)
public:
	enum Z3{_0_ = 0, _1_ = 1, _2_ = 2};
private:
	Z3* coefficients = NULL;

	friend Encryption;
	friend RpPolynomial mods_p(RqPolynomial P);
	friend RqPolynomial convolutionRq(const R2Polynomial&, const RpPolynomial&);
	friend RqPolynomial convolutionRq(const RpPolynomial&, const RqPolynomial&);
public:
	RpPolynomial();								// -Default constructor; initializes the polynomial with zeros
	RpPolynomial(const RpPolynomial& P);
	~RpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
	}
public:
	RpPolynomial& operator = (const RpPolynomial& P);			// Assignment
	int operator[](int i) const;
	int degree() const;							// Returns degree of polynomial
	mpz_class toNumber() const;						// -Interprests the coefficientes as a bese 3 number.
	void print(const char* name = "", bool centered = true, const char* tail = "") const;
	void println(const char* name = "", bool centered = true) const;
};

class R2Polynomial {								// Representation of the polynomials in R2 := Z2[x]/(x^N-1)
public:
	enum Z2 {_0_ = 0, _1_ = 1};						// Integers modulo 2 (binary numbers)
	friend Z2 operator + (Z2 a,Z2 b) {					// Addition modulus 2
		if(a!=b) return _1_;
		return _0_;
	}
	friend Z2 operator - (Z2 a,Z2 b) {					// Addition and subtraction coincide in Z2. This is just for evade problems
		if(a!=b) return _1_;						// with notation
		return _0_;
	}
	friend Z2 operator * (Z2 a, Z2 b) {					// Multiplication modulus 2
		if(a==0) return _0_;
		return  b ;
	}
	friend void operator += (Z2& a, Z2 b) {
		if(a != b) a = _1_;
		else a = _0_;
	}
	friend void operator -= (Z2& a, Z2 b) {
		if(a != b) a = _1_;
		else a = _0_;
	}
private:
	Z2* coefficients = NULL;
	friend Encryption;
	friend RqPolynomial convolutionRq(const R2Polynomial&, const RpPolynomial&);
	friend RqPolynomial convolutionRq(const R2Polynomial&, const RqPolynomial&);
public:
	R2Polynomial();
	R2Polynomial(const R2Polynomial& P);
	R2Polynomial(const RpPolynomial& P);
	~R2Polynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}

	R2Polynomial& operator = (const R2Polynomial& P);
	R2Polynomial& operator = (const RpPolynomial& P);
	R2Polynomial& operator = (Z2 t);
	R2Polynomial  operator + (const R2Polynomial&) const;			// In Z2, addition (+) coincide with subtraction (-)
	R2Polynomial  operator - (const R2Polynomial&) const;			// In Z2, addition (+) coincide with subtraction (-)
	R2Polynomial  operator * (const R2Polynomial&) const;
	void division(const R2Polynomial& P,R2Polynomial res[2]) const;		// Division between this and P, result[2] will save the res[2]. Throws NTRU::MathException
	R2Polynomial gcdXNmns1(R2Polynomial& thisBezout) const;			// Greatest common between this and x^N-1 polynomial. Writing
										// gcd = u·(x^N - 1) + v·this, thisBezout == v
	bool operator == (int t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	bool operator == (const R2Polynomial& P) const;
	bool operator != (int t) const {
		return this->degree() != 0 || this->coefficients[0] != t;
	}
	Z2 operator [] (int i) const;
	int  degree() const;
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
private:
	void negFirstCoeff() {
		if(this->coefficients[0] == 0) this->coefficients[0] = _1_;
		else this->coefficients[0] = _0_;
	}
};

RqPolynomial operator - (int64_t, const RqPolynomial&);				// The intention is to make this function a friend of RqPolynomial

class RqPolynomial {								// Representation of the polynomials in Rq := Zq[x]/(x^N-1)
private:
	int64_t* coefficients = NULL;
	friend Encryption;
	friend RqPolynomial operator - (int64_t, const RqPolynomial&);
	friend RqPolynomial convolutionRq(const R2Polynomial&, const RpPolynomial&);
	friend RqPolynomial convolutionRq(const R2Polynomial&, const RqPolynomial&);
	friend RqPolynomial convolutionRq(const RpPolynomial&, const RqPolynomial&);
public:
	RqPolynomial();
	RqPolynomial(const RqPolynomial& P);
	RqPolynomial(const char data[], int dataLength);			// -Initializing with string of bytes
	~RqPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	RqPolynomial& operator = (const RqPolynomial& P);
	int64_t operator[](int i) const;
	RqPolynomial operator + (const RqPolynomial& P) const;
	RqPolynomial operator * (const RqPolynomial& P) const;
	int degree()  const;							// -Returns degree of polynomial
	bool equalsOne() const;
	void mod_q()  const;
	void mods_q() const;
	int lengthInBytes() const;
	void toBytes(char dest[]) const;					// -Writes the coefficients into an array of bytes. If a certain coefficient is
										//  negative, +=q is applied in order to write a positive number
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
private:
	RqPolynomial& timesThree();						// -Gets a RpPolynomial via the operations 3p + 1
};

}

#endif