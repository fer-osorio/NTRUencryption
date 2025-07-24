#include<gmpxx.h>
#include"parameters.hpp"

#ifndef POLYNOMIALS_HPP
#define POLYNOMIALS_HPP

namespace NTRU{

struct ZpPolynomial;
struct ZqPolynomial;
struct Z2Polynomial;
class  Encryption;

ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&);
ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);
ZpPolynomial mods_p(ZqPolynomial);

struct ZpPolynomial {								// -Representation of the polynomials in Zp[x]/(x^N-1)
	enum Z3{_0_ = 0, _1_ = 1, _2_ = 2};					// -ZpCenterPolynomial are polynomials with coefficients in {-1, 0, 1}
	private:
	Z3* coefficients = NULL;
	friend Encryption;

	public:
	ZpPolynomial();								// -Default constructor; initializes the polynomial with zeros
	ZpPolynomial(const ZpPolynomial& P);
	~ZpPolynomial() {
		if(this->coefficients != NULL) delete [] this->coefficients;
	}
	static ZpPolynomial randomTernary();

	private: void interchangeZeroFor(Z3 t);					// -Randomly selects a coefficient with value 0 and a coefficient with value t and interchanges them.
	private: void changeZeroForOne();

	public:
	static ZpPolynomial getPosiblePrivateKey();				// -Setting d = N/3, returns a polynomial with d -1's and d+1 1's

	ZpPolynomial& operator = (const ZpPolynomial& P);			// Assignment
	int operator[](int i) const;
	int degree() const;							// Returns degree of polynomial

	friend ZpPolynomial mods_p(ZqPolynomial P);
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
	friend ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);

	mpz_class toNumber() const;						// -Interprests the coefficientes as a bese 3 number.
	void print(const char* name = "", bool centered = true, const char* tail = "") const;
	void println(const char* name = "", bool centered = true) const;
};

struct Z2Polynomial {								// Representation of the polynomials in Z2[x]/(x^N-1)
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

	public:
	Z2Polynomial();
	Z2Polynomial(const Z2Polynomial& P);
	Z2Polynomial(const ZpPolynomial& P);
	~Z2Polynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}

	Z2Polynomial& operator = (const Z2Polynomial& P);
	Z2Polynomial& operator = (const ZpPolynomial& P);
	Z2Polynomial& operator = (Z2 t);
	Z2Polynomial  operator + (const Z2Polynomial&) const;			// In Z2, addition (+) coincide with subtraction (-)
	Z2Polynomial  operator - (const Z2Polynomial&) const;			// In Z2, addition (+) coincide with subtraction (-)
	Z2Polynomial  operator * (const Z2Polynomial&) const;
	void division(const Z2Polynomial& P,Z2Polynomial res[2]) const;		// Division between this and P, result[2] will save the res[2]. Throws NTRU::MathException
	Z2Polynomial gcdXNmns1(Z2Polynomial& thisBezout) const;			// Greatest common between this and x^N-1 polynomial. Writing
										// gcd = u·(x^N - 1) + v·this, thisBezout == v
	bool operator == (int t) const {
		return this->degree() == 0 && this->coefficients[0] == t;
	}
	bool operator == (const Z2Polynomial& P) const;
	bool operator != (int t) const {
		return this->degree() != 0 || this->coefficients[0] != t;
	}
	Z2 operator [] (int i) const;

	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&);

	int  degree() const;
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;

	private:
	void negFirstCoeff() {
		if(this->coefficients[0] == 0) this->coefficients[0] = _1_;
		else this->coefficients[0] = _0_;
	}
};

ZqPolynomial operator - (int64_t, const ZqPolynomial&);				// The intention is to make this function a friend of ZqPolynomial

struct ZqPolynomial {								// Representation of the polynomials in Zq[x]/(x^N-1)
	private:
	int64_t* coefficients = NULL;
	friend Encryption;

	public:
	ZqPolynomial();
	ZqPolynomial(const ZqPolynomial& P);
	ZqPolynomial(const char data[], int dataLength);			// -Initializing with string of bytes
	~ZqPolynomial() {
		if(this->coefficients != NULL) delete[] this->coefficients;
	}
	ZqPolynomial& operator = (const ZqPolynomial& P);
	int64_t operator[](int i) const;
	ZqPolynomial operator + (const ZqPolynomial& P) const;
	ZqPolynomial operator * (const ZqPolynomial& P) const;
	friend ZqPolynomial operator - (int64_t, const ZqPolynomial&);

	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&);
	friend ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&);
	friend ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&);

	int degree()  const;							// -Returns degree of polynomial
	bool equalsOne() const;
	void mod_q()  const;
	void mods_q() const;
	int lengthInBytes() const;
	static int log2(NTRU_q q);

	void toBytes(char dest[]) const;					// -Writes the coefficients into an array of bytes. If a certain coefficient is
										//  negative, +=q is applied in order to write a positive number
	void print(const char* name = "", const char* tail = "") const;
	void println(const char* name = "") const;
	void save(const char* name = NULL, bool saveAsText = false) const;	// -Saving ZqPolynomial in a Binary file. Throws NTRU::FileIOException
	static ZqPolynomial fromFile(const char* fileName);			// -Building a ZqPolynomial from file. Throws NTRU::FileIOException, NTRU::ParameterMismatchException

	private:
	ZqPolynomial& timesThree();						// -Gets a ZpPolynomial via the operations 3p + 1
};

}

#endif