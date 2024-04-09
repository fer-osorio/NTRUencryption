#include"NTRUPolynomials.hpp"
#include<random>
#include<ctime>

const int NTRU_ZpPolynomial::Z3addition[3][3] = {{0, 1, 2},                     // Addition table of the Z3 ring (integers modulo 3)
                                                 {1, 2, 0},                     // ...
                                                 {2, 0, 1}};                    // ...

const int NTRU_ZpPolynomial::Z3subtraction[3][3] = {{0, 2, 1},                  // Addition table of the Z3 ring (integers modulo 3)
                                                    {1, 0, 2},                  // ...
                                                    {2, 1, 0}};                 // ...

const int NTRU_ZpPolynomial::Z3product[3][3] = {{0, 0, 0},                      // Product table of the Z3 ring (integers modulo 3)
                                                {0, 1, 2},                      // ...
                                                {0, 2, 1}};                     // ...
static unsigned _seed_ = (unsigned)time(NULL);

inline static int min(int a, int b) {
	if(a < b) return a;
	return b;
}
inline static int max(int a, int b) {
	if(a < b) return b;
	return a;
}
inline static NTRU_N min_N(NTRU_N a, NTRU_N b) {
	if(a < b) return a;
	return b;
}
inline static NTRU_N max_N(NTRU_N a, NTRU_N b) {
	if(a < b) return b;
	return a;
}
inline static NTRU_q min_q(NTRU_q a, NTRU_q b) {
	if(a < b) return a;
	return b;
}
inline static NTRU_q max_q(NTRU_q a, NTRU_q b) {
	if(a < b) return b;
	return a;
}
static int intToString(int n, char* dest) {                                     // String representation of unsigned integer it returns the length of the string
    int i = 0, j = 0, l = 0;
    char buff = 0;
    if(n < 0) {	dest[i++] = '-'; n = -n; }
    do {
        buff = (char)(n % DECIMAL_BASE);                                        // Taking last current digit
        dest[i++] = buff + 48;                                                  // Saving last current digit
        n -= (int)buff; n /= DECIMAL_BASE;                                 		// Taking out last current digit from the number n
    } while(n > 0);
    l = i;
    dest[i--] = 0;                                                              // Putting a zero at the end and returning one place
    for(; j < i; j++,i--) {                                                     // The number is backwards; reversing the order of the digits
        buff = dest[j];
        dest[j] = dest[i];
        dest[i] = buff;
    }
    return l;                                                                   // Returning length
}
inline static int copyString(const char* origin,char* dest) {
    int i;                                                                      // Counting variable. At the end it will contain the length of the origin string
    for(i = 0; origin[i] != 0; i++) dest[i] = origin[i];                        // Coping element by element
    dest[i] = 0;                                                                // End of string.
    return i;                                                                   // Returning string length
}
inline static int printSpaces(unsigned t) {
	while(t-- > 0) std::cout << ' ' ;
	return 0;
}																				// Functions for printing
inline static int len(const char* str) {										// Length of a string
	int l = -1;
	while(str[++l] != 0) {}
	return l;
}
static unsigned len(int a) {
    unsigned l = 0;
    do { a /= DECIMAL_BASE; l++;
    }while(a > 0);
    return l;
}
static void printArray(int* array, unsigned arrlen, unsigned columnlen, const
char* name = "", const char* tail = "") {
    char buff[10];                                                              // Buffer necessary for the int -> string conversion
    int strLen = 0;                                                             // Start length in characters
    int i=0,j=0;
    int startLen;                                                               // Length of the starting string
    int lastIndex = (int)arrlen - 1;

    startLen = len(name);
    if(startLen > 0) {
        std::cout << name << " = [";
        startLen += 4;
    }
    else {
        std::cout <<  "0   [";
        startLen = 5;
    }
    do {
        if(i != 0 && (i & 15) == 0) {                                           // Since 2^5 = 32, then i&31 = i % 32
            std::cout << '\n'; j++;                                             // New row; increasing number of rows
            if(i != lastIndex) {
                strLen = intToString(j, buff);
                std::cout << buff;                                              // Printing current coefficient
                printSpaces((unsigned)max(startLen - strLen,0));                // Padding with spaces
            }
        }
        strLen = intToString(array[i], buff);                                   // Optimize by returning the length of the string
        std::cout << buff;                                                      // Printing current coefficient
        printSpaces((unsigned)max((int)columnlen - strLen ,0));                 // Padding with spaces
        if(i < lastIndex) std::cout << ',';
    }while(++i < (int)arrlen);
    std::cout << ']';
    std::cout << tail;
}

class RandInt {                                                                 // Little class for random integers. Taken from The C++ Programming Language 4th
    public:                                                                     // Edition Bjarne Stroustrup
    RandInt(int low, int high, unsigned seed): re{seed}, dist{low,high} {}
    int operator()() { return dist(re); }                                       // Draw an int
    private:
    std::default_random_engine re;
    std::uniform_int_distribution<> dist;
};

// ||||||||||||||||| NTRU_ZpPolynomial::ZpPolynomial |||||||||||||||||||||||||||

NTRU_ZpPolynomial::ZpPolynomial::ZpPolynomial(int _degree_, NTRU_p _p_):p(_p_), // Second argument is used only to avoid ambiguity
degree(_degree_) {                                                              // Initializes with zeros
	int _coeffAmount_  = this->coeffAmount(), i;
	this->coefficients = new int[_coeffAmount_];
	for(i = 0; i < _coeffAmount_; i++)
	this->coefficients[i] = 0;
}

NTRU_ZpPolynomial::ZpPolynomial::ZpPolynomial(int zeros, int ones, int twos):
degree(1) {
    int i, j, _coeffAmount_;

    if(zeros < 0) zeros = -zeros;
    if(ones < 0) ones = -ones;                                                  // Guarding against invalid values of ones and twos. In particular the
    if(twos < 0) twos = -twos;                                                  // inequality ones + twos < N must follow

    this->degree = zeros + ones + twos;
    _coeffAmount_ = this->coeffAmount();                                        // degree + 1
    RandInt rn{0, this->degree, _seed_++};                                      // Random integers from 0 to N-1
	this->coefficients = new int[_coeffAmount_];

	for(i = 0; i < _coeffAmount_; i++) this->coefficients[i] = 0;

	while(ones > 0) {                                                           // Putting the ones first
        j = rn();
        if(this->coefficients[j] == 0) {
            this->coefficients[j] = 1;
            ones--;
        }
	}
	while(twos > 0) {                                                           // Then the twos
        j = rn();
        if(this->coefficients[j] == 0) {
            this->coefficients[j] = 2;                                          // Two is congruent with -1 modulo 3
            twos--;
        }
	}
	this->correctDegree();
}

NTRU_ZpPolynomial::ZpPolynomial::ZpPolynomial(const ZpPolynomial& P): p(P.p),
degree(P.degree) {
	int _coeffAmount_  = this->coeffAmount(), i;
	this->coefficients = new int[_coeffAmount_];
	for(i = 0; i < _coeffAmount_; i++)
		this->coefficients[i] = P.coefficients[i];
}

NTRU_ZpPolynomial::ZpPolynomial::ZpPolynomial(const NTRU_ZpPolynomial& P):
p(P.get_p()), degree(P.degree()){
    int _coeffAmount_  = this->coeffAmount(), i;
	this->coefficients = new int[_coeffAmount_];
	for(i = 0; i < _coeffAmount_; i++)  this->coefficients[i] = P[i];
}

NTRU_ZpPolynomial::ZpPolynomial& NTRU_ZpPolynomial::ZpPolynomial::operator =
(const ZpPolynomial& P) {
    if(this != &P) {														    // Guarding against self assignment
		int _coeffAmount_ = P.coeffAmount(), i;
		if(this->degree != P.degree) {
			delete[] this->coefficients;
			this->coefficients = new int[_coeffAmount_];
			this->degree = P.degree;
		}
		this->p = P.p;
		for(i = 0; i < _coeffAmount_; i++)
			this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

NTRU_ZpPolynomial::ZpPolynomial& NTRU_ZpPolynomial::ZpPolynomial::operator =
(int t) {
    if(this->coefficients == NULL)
		this->coefficients = new int[1];
	if(t < 0) t = -t;
	if(t >= this->p) t %= this->p;
	this->coefficients[0] = t;
    this->degree = 0;
	return *this;
}

NTRU_ZpPolynomial::ZpPolynomial NTRU_ZpPolynomial::ZpPolynomial::operator +
(const ZpPolynomial& P) const{
    ZpPolynomial r(max(this->degree, P.degree),_3_);                            // Initializing result in the "biggest polynomial ring"
    const ZpPolynomial *small, *big;
    int i;

    if(this->degree < P.degree) { small = this; big = &P; }                     // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                            // polynomial with the biggest N

    for(i = 0; i <= small->degree; i++)
        r.coefficients[i]=Z3addition[this->coefficients[i]][P.coefficients[i]]; // Addition element by element till the smallest degree of the arguments
    for(; i <= big->degree; i++)
        r.coefficients[i] = big->coefficients[i];                               // Just copying, equivalent to filling with zeros the small polynomial
    r.correctDegree();
    return r;
}

NTRU_ZpPolynomial::ZpPolynomial NTRU_ZpPolynomial::ZpPolynomial::operator -
(const ZpPolynomial& P) const{
	ZpPolynomial r(max(this->degree, P.degree), _3_);                           // Initializing result with zeros
	int deg = min(this->degree,P.degree), DEG = max(this->degree,P.degree), i;

    for(i = 0; i <= deg; i++)
    	r.coefficients[i] =
    	Z3subtraction[this->coefficients[i]][P.coefficients[i]];    	        // Subtraction element by element till the smallest degree of the arguments
    if(P.degree == DEG) for(; i <= DEG; i++)
    	r.coefficients[i] = Z3subtraction[0][P.coefficients[i]];                // Second argument bigger, virtually filling the first argument with zeros
    else for(; i <= DEG; i++)
    	r.coefficients[i] = this->coefficients[i];                              // First argument bigger, virtually filling the second argument with zeros

    while(r.coefficients[r.degree] == 0 && r.degree > 0) r.degree--;            // Correcting degree

    return r;
}

NTRU_ZpPolynomial::ZpPolynomial NTRU_ZpPolynomial::ZpPolynomial::operator - ()
const {
	ZpPolynomial r = *this;
	for(int i = 0; i <= r.degree; i++)
		r.coefficients[i] = Z3subtraction[0][r.coefficients[i]];
	return r;
}

NTRU_ZpPolynomial::ZpPolynomial NTRU_ZpPolynomial::ZpPolynomial::operator *
(const ZpPolynomial& P) const{
    ZpPolynomial r(this->degree + P.degree, _3_);
	int i,j;
	for(i = 0; i <= this->degree; i++) {
		if(this->coefficients[i] != 0)
		for(j = 0; j <= P.degree; j++) {
			if(P.coefficients[j] != 0)
			r.coefficients[i+j] = Z3addition[r.coefficients[i+j]]
			[Z3product[this->coefficients[i]][P.coefficients[j]]];
		}
	}
	return r;
}

bool NTRU_ZpPolynomial::ZpPolynomial::operator == (const ZpPolynomial& P){
	if(this->degree == P.degree && this->p == P.p) {                            // The comparison this->p == P.p is naive, but maybe will be useful in the future
		for(int i = 0; i <= this->degree; i++)
			if(this->coefficients[i] != P.coefficients[i])  return false;
		return true;
	}
	else return false;
}

void NTRU_ZpPolynomial::ZpPolynomial::print(const char* name, const char* tail)
const {
    printArray(this->coefficients,(unsigned)this->degree + 1,len(this->p) + 1,
    name, tail);
}

void NTRU_ZpPolynomial::ZpPolynomial::println(const char* name) const{
    this->print(name, "\n");
}

//__________________ NTRU_ZpPolynomial::ZpPolynomial ___________________________

// |||||||||||||||||||||||||| NTRU_ZpPolynomial ||||||||||||||||||||||||||||||||

NTRU_ZpPolynomial::NTRU_ZpPolynomial(const NTRU_ZpPolynomial& P):N(P.N),p(P.p){
	this->coefficients = new int[P.N];
	for(int i=0; i<P.N; i++)
		this->coefficients[i] = P.coefficients[i];
}

NTRU_ZpPolynomial::NTRU_ZpPolynomial(const ZpPolynomial& P, NTRU_N _N_):
N(_N_), p(P.get_p()){
    int upperLimit = min(this->N, P.coeffAmount()), i;
	this->coefficients = new int[this->N];
	for(i = 0; i < upperLimit; i++)	this->coefficients[i] = P[i];
	if(upperLimit < this->N) for(; i < this->N; i++) this->coefficients[i] = 0;
	else for(; i < upperLimit; i++)                                             // This step is equivalent to apply mod x^N - 1
		this->coefficients[i%this->N] =
		Z3addition[this->coefficients[i%this->N]][P[i]];
}

NTRU_ZpPolynomial::NTRU_ZpPolynomial(NTRU_N _N_,int ones,int twos,NTRU_p _p_):
N(_N_), p(_p_) { int i, j;
    RandInt rn{0, _N_-1, _seed_++};                                             // Random integers from 0 to N-1
    if(ones < 0) ones = -ones;                                                  // Guarding against invalid values of ones and twos. In particular the
    if(twos < 0) twos = -twos;                                                  // inequality ones + twos < N must follow
    while(ones + twos >= this->N) {                                             // Dividing by two till getting inside the allowed range
        ones <<= 1;                                                             // ...
        twos <<= 1;                                                             // ...
    }
	this->coefficients = new int[_N_];
	for(i = 0; i < _N_; i++) this->coefficients[i] = 0;
	while(ones > 0) {                                                           // Putting the ones first
        j = rn();
        if(this->coefficients[j] == 0) {
            this->coefficients[j] = 1;
            ones--;
        }
	}
	while(twos > 0) {                                                           // Then the negative ones
        j = rn();
        if(this->coefficients[j] == 0) {
            this->coefficients[j] = 2;                                          // Two is congruent with -1 modulo 3
            twos--;
        }
	}
}                                                                               // Maybe is some room for optimization using JV theorem

NTRU_ZpPolynomial NTRU_ZpPolynomial::operator + (const NTRU_ZpPolynomial& P)
const{
    NTRU_ZpPolynomial r(max_N(this->N, P.N));                                      // Initializing result in the "biggest polynomial ring"
    const NTRU_ZpPolynomial *small, *big;
    int i;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                            // polynomial with the biggest N

    for(i = 0; i < small->N; i++)
        r.coefficients[i]=Z3addition[this->coefficients[i]][P.coefficients[i]]; // Addition element by element till the smallest degree of the arguments
    for(; i < big->N; i++)
        r.coefficients[i] = big->coefficients[i];                               // Just copying, equivalent to filling with zeros the small polynomial
    return r;
}

NTRU_ZpPolynomial NTRU_ZpPolynomial::operator - (const NTRU_ZpPolynomial& P)
const{
    const NTRU_N _n_ = min_N(this->N, P.N), _N_ = max_N(this->N, P.N);
    NTRU_ZpPolynomial r(_N_);                                                   // Initializing result in the "biggest polynomial ring"
    int i;

    for(i = 0; i < _n_; i++)
        r.coefficients[i] =                                                     // Subtraction element by element till the smallest degree of the arguments
        Z3subtraction[this->coefficients[i]][P.coefficients[i]];
    if(P.N == _N_) for(; i < _N_; i++)                                          // Second argument bigger, virtually filling the first argument with zeros
        r.coefficients[i] = Z3subtraction[0][P.coefficients[i]];
    else for(; i < _N_; i++)                                                    // First argument bigger, virtually filling the second argument with zeros
        r.coefficients[i] = this->coefficients[i];
    return r;
}

NTRU_ZpPolynomial NTRU_ZpPolynomial::operator - () const{
	NTRU_ZpPolynomial r = *this;
	int i, _degree_ = this->degree();
	for(i = 0; i <= _degree_; i++)
		r.coefficients[i] = Z3subtraction[0][r.coefficients[i]];
	return r;
}

NTRU_ZpPolynomial NTRU_ZpPolynomial::operator * (const NTRU_ZpPolynomial& P)
const{
    NTRU_ZpPolynomial r(max_N(this->N, P.N));                                   // Initializing with zeros
    int i, j, k;
    int this_degree = this->degree();
    int P_degree    = P.degree();

	for(i = 0; i <= this_degree; i++) {
		if(this->coefficients[i] != 0)
		for(j = 0; j <= P_degree; j++) {
			if(P.coefficients[j] != 0) {
			    if((k = i + j) < this->N)
			        r.coefficients[k] = Z3addition[r.coefficients[k]]
    			    [Z3product[this->coefficients[i]][P.coefficients[j]]];
			    else
			        r.coefficients[k-this->N] =
			        Z3addition[r.coefficients[k-this->N]]
			        [Z3product[this->coefficients[i]][P.coefficients[j]]];
			}
		}
	}
    return r;
}

NTRU_ZpPolynomial& NTRU_ZpPolynomial::operator-=(const NTRU_ZpPolynomial& P) {
    const NTRU_N _n_ = min_N(this->N, P.N);
    int i;
    for(i = 0; i < _n_; i++)
        this->coefficients[i] =                                                 // Subtraction element by element till the smallest degree of the arguments
        Z3subtraction[this->coefficients[i]][P.coefficients[i]];
    return *this;
}

void NTRU_ZpPolynomial::division(const NTRU_ZpPolynomial& P, NTRU_ZpPolynomial
result[2]) const{
    if(P == 0) {
        throw "\nvoid NTRU_ZpPolynomial::ZpPolynomial::division(const Zp"
        "Polynomial& P,ZpPolynomial result[2]) const. Division by zero...\n";
    }
    if(*this == 0) {                                                            // Case zero divided by anything
        result[0] = 0;                                                          // Zero polynomial
        result[1] = 0;                                                          // Zero polynomial
        return;
    }

    const int dividendDegree = this->degree();
    const int divisorDegree  = P.degree();
    const int leadCoeffDivsrInv = P.coefficients[divisorDegree];                // In Z3, each element is its own inverse
    int degreeDiff;                                                             // Difference between degrees
    int remDeg;                                                                 // Remainder degree
    int i;                                                                      // For counting

    if(dividendDegree < divisorDegree) {                                        // Case dividend has smaller degree than divisor
        result[0] = 0; result[1] = *this;
        return;
    }                                                                           // At this point we know leading coefficient has an inverse in Zq

    degreeDiff = dividendDegree - divisorDegree;                                // At this point we know degreeDiff >= 0
    remDeg = dividendDegree;
    result[1] = *this;                                                          // Initializing remainder with dividend (this)
    result[0] = ZpPolynomial(this->N);

    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        result[0].coefficients[degreeDiff] = Z3product[
        leadCoeffDivsrInv][result[1].coefficients[remDeg]];                     // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] =
            Z3subtraction[result[1].coefficients[i]][Z3product[
            result[0].coefficients[degreeDiff]][P.coefficients[i-degreeDiff]]];
        }

        if(result[1].coefficients[remDeg] != 0) {                               // No congruence with 0 mod q, throwing exception
            throw "\nvoid NTRU_ZpPolynomial::ZpPolynomial::division(const Zp"
            "Polynomial& P,ZpPolynomial result[2]) const. result[1]."
            "coefficients[remDeg] != 0\n";
        }                                                                       // At this point we know result[1].coefficients[remDeg] = 0
        while(remDeg >= 0 && result[1].coefficients[remDeg] == 0) remDeg--;     // Updating value of the degree of the remainder
    }
}

NTRU_ZpPolynomial NTRU_ZpPolynomial::gcdXNmns1(NTRU_ZpPolynomial& thisBezout)
const{                                                                          // EEA will mean Extended Euclidean Algorithm
    NTRU_ZpPolynomial gcd;                                                      // Initializing result in the "biggest polynomial ring
    NTRU_ZpPolynomial remainders;
    NTRU_ZpPolynomial tmp[2];
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    int leadCoeff = this->coefficients[deg];                                    // Lead coefficient of this polynomial
    NTRU_ZpPolynomial quoRem[2] = { NTRU_ZpPolynomial(this->N),
                                    NTRU_ZpPolynomial(this->N) };

    quoRem[0].coefficients[this->N-deg] = leadCoeff;                            // Start of division algorithm between virtual polynomial x^N-1 and this
    for(i = deg-1, j = this->N - 1; i >= 0; i--, j--) {                         // First coefficient of quotient and first subtraction
        quoRem[1].coefficients[j] =
        Z3subtraction[0][Z3product[ leadCoeff ][ this->coefficients[i]] ];
    }
    quoRem[1].coefficients[0] = 2;                                              // Putting the -1 that is at the end of the polynomial x^N-1
    for(i = this->N-1 - deg, j = this->N-1; j >= deg; i = j - deg) {            // Continuing with division algorithm; i is the place of the next coefficient of
        quoRem[0].coefficients[i] =                                             // the quotient, j is the degree of the remainders.
        Z3product[leadCoeff][quoRem[1].coefficients[j]];
        for(k = deg, l = j; k >= 0; k--, l--) {                                 // Multiplication-subtraction step
            quoRem[1].coefficients[l] =
            Z3subtraction[ quoRem[1].coefficients[l] ]
            [Z3product[ quoRem[0].coefficients[i] ][ this->coefficients[k] ]];
        }
        while(quoRem[1].coefficients[j] == 0) {j--;};
    }                                                                           // End of division algorithm between virtual polynomial x^N-1 and this

    tmp[1] = -quoRem[0]; thisBezout = 1;                                        // Initializing values for the execution of the rest of the EEA
    tmp[0] = tmp[1];                                                            // v[-1] = 0, v[0] = 1 ==> v[1] = v[-1] - q[1]*v[0] = - q[1]
    gcd = *this;                                                                // ...
    remainders = quoRem[1];                                                     // ...

	while(remainders != 0) {                                                    // EEA implementation (continuation)
        try{ gcd.division(remainders, quoRem); }
        catch(const char* exp) {
            std::cout << "\nIn NTRUencryption.cpp; function NTRU_ZpPolynomial "
            "NTRU_ZpPolynomial::PolyModXNmns1::gcdXNmns1(PolyModXNmns1& this"
            "Bezout)\n";
            throw;
        }
        /*
        std::cout <<      "tmp[0].degree = " <<    tmp[0].degree;
        std::cout << ", quoRem[0].degree = " << quoRem[0].degree;
        std::cout << ", quoRem[1].degree = " << quoRem[1].degree << '\n';
        */
        tmp[1] = thisBezout - quoRem[0]*tmp[0];                                 // u[k+2] = u[k] - q[k+2]*u[k+1]
        thisBezout = tmp[0];                                                    // Updating values
        tmp[0] = tmp[1];                                                        // ...
        gcd = remainders;                                                       // ...
        remainders = quoRem[1];                                                 // ...
	}
	thisBezout = thisBezout;
	return gcd;
}

bool NTRU_ZpPolynomial::operator == (const NTRU_ZpPolynomial& P) const{
	if(this->N == P.N && this->p == P.p) {									    // The comparison this->p == P.p is naive, but maybe will be useful in the future
		for(int i = 0; i < this->N; i++)
			if(this->coefficients[i]!=P.coefficients[i])
				return false;
		return true;
	}
	else return false;
}

bool NTRU_ZpPolynomial::operator != (const NTRU_ZpPolynomial& P) const{
	if(this->N == P.N && this->p == P.p) {									    // The comparison this->p != P.p is naive, but maybe will be useful in the future
		for(int i = 0; i < this->N; i++)
			if(this->coefficients[i] != P.coefficients[i])
				return true;
		return false;
	}
	else return true;
}

NTRU_ZpPolynomial& NTRU_ZpPolynomial::operator = (const NTRU_ZpPolynomial& P) {
    if(this != &P) {													        // Guarding against self assignment
		if(this->coefficients == NULL) {
			this->coefficients = new int[P.N];
			this->N = P.N;
		} else if(this->N != P.N) {
			delete [] this->coefficients;
			this->coefficients = new int[P.N];
			this->N = P.N;
		}
	    for(int i = 0; i < P.N; i++)
	    	this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

NTRU_ZpPolynomial& NTRU_ZpPolynomial::operator = (const ZpPolynomial& P) {
    int upperLimit = min(this->N, P.coeffAmount()), i;
	if(this->coefficients == NULL)
		this->coefficients = new int[this->N];
	for(i = 0; i < upperLimit; i++)
		this->coefficients[i] = P[i];
	if(upperLimit < this->N)
		for(; i < this->N; i++)
			this->coefficients[i] = 0;
	else for(; i < upperLimit; i++)
		this->coefficients[i%this->N] =
		Z3addition[this->coefficients[i%this->N]][P[i]];
	this->p = P.get_p();
	return *this;
}

NTRU_ZpPolynomial& NTRU_ZpPolynomial::operator = (int t) {
    if(this->coefficients == NULL)
		this->coefficients = new int[this->N];                                  // Dealing with already initialized object, so this->N is well defined
	if(t < 0) t = -t;
	if(t >= this->p) t %= this->p;
	this->coefficients[0] = t;
	for(int i=1; i < this->N; i++) this->coefficients[i] = 0;                   // Filling with zeros the rest of the array
	return *this;
}

void NTRU_ZpPolynomial::setPermutation() {                                      // Setting a permutation
    int i, j, k, *tmp = new int[this->N];
    RandInt rn{0, 0x7FFFFFFF, _seed_++};                                        // Random integers from 0 to the maximum number for and int
    if(this->permutation==NULL) this->permutation = new int[this->N];
    for(i = 0; i < this->N; i++)  tmp[i] = i;
    for(i = 0, j = this->N; i < this->N; i++, j--) {
        k = rn()%j;
        this->permutation[i] = tmp[k];
        tmp[k] = tmp[j-1];
    }
    delete[] tmp;
}
                                                                                // Implementation of the permutation
void NTRU_ZpPolynomial::permute() {
	int i;
	if(this->permutation == NULL) this->setPermutation();
	if(this->coeffCopy   == NULL) {                                             // If there is no a copy, create the copy
	    this->coeffCopy = new int[this->N];
	    for(i = 0; i < this->N; i++)
	        this->coeffCopy[i]=this->coefficients[i];
	}
	for(i = 0; i < this->N; i++)                                                // Permute coefficients
		this->coefficients[i] = this->coeffCopy[permutation[i]];
	for(i = 0; i < this->N; i++)                                                // Copy coefficients
		this->coeffCopy[i] = this->coefficients[i];
}

void NTRU_ZpPolynomial::print(const char* name, const char* tail) const{
    printArray(this->coefficients, (unsigned)this->degree() + 1,
    len(this->p) + 1, name, tail);
}

void NTRU_ZpPolynomial::println(const char* name) const{
	this->print(name, "\n");
}

void NTRU_ZpPolynomial::test(NTRU_N _N_, int d) const{
    NTRU_ZpPolynomial Np0(_N_, d, d+1);
	NTRU_ZpPolynomial Np1(_N_, d, d);
	NTRU_ZpPolynomial quorem[2];
	NTRU_ZpPolynomial gcd;
	NTRU_ZpPolynomial Bezout;
	NTRU_ZpPolynomial Np2(Np0,this->N);

	std::cout << "\n::::"
	"NTRU_ZpPolynomial testing start ........................................."
	"........................."
	<< '\n';

    try { Np0.division(Np1, quorem); }
	catch(const char* exp) { std::cout << exp; }

	if( Np1*quorem[0] + quorem[1] == Np0 && Np1.degree() > quorem[1].degree())
	    std::cout << "\nSuccesful division.\n";

	try{ gcd = Np2.gcdXNmns1(Bezout); }
	catch(const char* exp) { std::cout << exp; }
	gcd.println("gcd");
	//Bezout.println("Bezout");
	//Np2.println("Np2");
	(Np2*Bezout).println("Np2*Bezout");

	std::cout << "\n::::"
	"NTRU_ZpPolynomial testing end ..........................................."
	".........................."
	<< "\n\n";
}

//___________________________ NTRU_ZpPolynomial ________________________________

//|||||||||||||||||||| NTRU_ZqPolynomial::Z2Polynomial |||||||||||||||||||||||||

NTRU_ZqPolynomial::Z2Polynomial::Z2Polynomial(const Z2Polynomial& P): N(P.N) {
    this->coefficients = new Z2[P.N];
    for(int i = 0; i < P.N; i++) this->coefficients[i] = P.coefficients[i];
}

NTRU_ZqPolynomial::Z2Polynomial::Z2Polynomial(const NTRU_ZpPolynomial& P):
N(P.get_N()) {
	this->coefficients = new Z2[this->N];
	for(int i = 0; i < this->N; i++) {
		if(P[i] == 1) this->coefficients[i] = _1_;
		else this->coefficients[i] = _0_;
	}
	std::cout << "\n";
}

NTRU_ZqPolynomial::Z2Polynomial::Z2Polynomial(NTRU_N _N_, int ones): N(_N_) {
	RandInt rn{0, _N_-1, _seed_++};                                             // Random integers from 0 to N-1
	int i, j;
    if(ones < 0) ones = -ones;                                                  // Guarding against invalid values of ones and twos. In particular the
    while(ones >= _N_) ones <<= 1;                                              // Dividing by two till getting inside the allowed range
	this->coefficients = new Z2[_N_];
	for(i = 0; i < _N_; i++) this->coefficients[i] = _0_;
	while(ones > 0) {                                                           // Putting the ones first
        j = rn();
        if(this->coefficients[j] == _0_) {
            this->coefficients[j] = _1_;
            ones--;
        }
	}
}

NTRU_ZqPolynomial::Z2Polynomial& NTRU_ZqPolynomial::Z2Polynomial::operator =
(const Z2Polynomial& P)  {
	if(this != &P) {													        // Guarding against self assignment
		if(this->N != P.N) {											        // Delete pass coefficients array. If this->N != P.N is true, there is no reason
			if(this->coefficients != NULL) delete[] this->coefficients;	        // to delete the array
			this->coefficients = new Z2[P.N];
			this->N = P.N;
		}
		if(this->coefficients == NULL) this->coefficients = new Z2[P.N];
		for(int i = 0; i < P.N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

NTRU_ZqPolynomial::Z2Polynomial& NTRU_ZqPolynomial::Z2Polynomial::operator =
(const NTRU_ZpPolynomial& P) {
    NTRU_N P_N = P.get_N();
	if(this->N != P_N) {											            // Delete past coefficients array. If this->N != P.N is true, there is no reason
		if(this->coefficients != NULL) delete[] this->coefficients;	            // to delete the array
		this->coefficients = new Z2[P_N];
		this->N = P_N;
	}
	if(this->coefficients == NULL) this->coefficients = new Z2[P_N];            // In case of having an object created with the default (private) constructor
	for(int i = 0; i < P_N; i++) {                                              // Fitting the ZpPolynomial in a Z2Polynomial
	    if(P[i] == 1) this->coefficients[i] = _1_;
	    else this->coefficients[i] = _0_;
	}
	return *this;
}

NTRU_ZqPolynomial::Z2Polynomial& NTRU_ZqPolynomial::Z2Polynomial::operator =
(Z2 t)  {
	if(this->coefficients==NULL) this->coefficients = new Z2[this->N];          // In case of have been generated from the (private) default constructor
	this->coefficients[0] = t;
	for(int i = 1; i < this->N; i++) this->coefficients[i] = _0_;
	return *this;
}

NTRU_ZqPolynomial::Z2Polynomial NTRU_ZqPolynomial::Z2Polynomial::operator +
(const Z2Polynomial& P) const {
    Z2Polynomial r(max_N(this->N, P.N));                                        // Initializing result in the "biggest polynomial ring"
    const Z2Polynomial* big;                                                    // Pointer to the 'biggest' polynomial
    int i, small_degree, big_degree;                                            // Degree of the polynomials, small_degree <= big_degree

    if(this->degree() < P.degree()) {
        big = &P;
        small_degree = this->degree();
    }
	else {
	    big = this;
	    small_degree = P.degree();
	}
	big_degree   = big->degree();

    for(i = 0; i <= small_degree; i++)
        r.coefficients[i] = this->coefficients[i] + P.coefficients[i];          // Subtraction element by element till the smallest degree of the arguments
    for(; i <= big_degree; i++)
        r.coefficients[i] = big->coefficients[i];                               // Just copying, equivalent to filling with zeros the small polynomial
    return r;
}

NTRU_ZqPolynomial::Z2Polynomial NTRU_ZqPolynomial::Z2Polynomial::operator -
(const Z2Polynomial& P) const {
    return *this + P;                                                           // In this polynomial ring, subtraction coincide with addition
}

NTRU_ZqPolynomial::Z2Polynomial NTRU_ZqPolynomial::Z2Polynomial::operator *
(const Z2Polynomial& P) const{                                                  // Classical polynomial multiplication algorithm
    Z2Polynomial r(max_N(this->N, P.N));
    int i, j, k;
    int this_degree = this->degree();
    int P_degree    = P.degree();

	for(i = 0; i <= this_degree; i++) {
		if(this->coefficients[i] != _0_)                                        // Taking advantage this polynomials have a big proportion of zeros
		    for(j = 0; j <= P_degree; j++) {
			    if(P.coefficients[j] != _0_) {
			        if((k = i + j) < r.N)
			            r.coefficients[k] +=
			            this->coefficients[i]*P.coefficients[j];
			        else
			            r.coefficients[k-r.N] +=
			            this->coefficients[i]*P.coefficients[j];
			    }
		    }
	}
	return r;
}

void NTRU_ZqPolynomial::Z2Polynomial::division(const Z2Polynomial& P,
Z2Polynomial result[2]) const{
    if(P == _0_) {
        throw "\nvoid NTRU_ZpPolynomial::ZpPolynomial::division(const Zp"
        "Polynomial& P,ZpPolynomial result[2]) const. Division by zero...\n";
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
    result[0] = Z2Polynomial(this->N);

    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        result[0].coefficients[degreeDiff] =
        leadCoeffDivsrInv * result[1].coefficients[remDeg];                     // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] -=
            result[0].coefficients[degreeDiff]*P.coefficients[i-degreeDiff];
        }

        if(result[1].coefficients[remDeg] != _0_) {                             // No congruence with 0 mod q, throwing exception
            throw "\nvoid NTRU_ZpPolynomial::ZpPolynomial::division(const Zp"
            "Polynomial& P,ZpPolynomial result[2]) const. result[1]."
            "coefficients[remDeg] != 0\n";
        }                                                                       // At this point we know result[1].coefficients[remDeg] = 0
        while(remDeg >= 0 && result[1].coefficients[remDeg] == _0_) remDeg--;   // Updating value of the degree of the remainder
    }
}

NTRU_ZqPolynomial::Z2Polynomial NTRU_ZqPolynomial::Z2Polynomial::gcdXNmns1(
Z2Polynomial& thisBezout) const{
    Z2Polynomial gcd;                                                           // Initializing result in the "biggest polynomial ring
    Z2Polynomial remainders;
    Z2Polynomial tmp[2];
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    Z2 leadCoeff = this->coefficients[deg];                                     // Lead coefficient of this polynomial
    Z2Polynomial quoRem[2]={Z2Polynomial(this->N),
                            Z2Polynomial(this->N)};

    quoRem[0].coefficients[this->N-deg] = leadCoeff;                            // Start of division algorithm between virtual polynomial x^N-1 and this
    for(i = deg-1, j = this->N - 1; i >= 0; i--, j--) {                         // First coefficient of quotient and first subtraction
        quoRem[1].coefficients[j] = this->coefficients[i];                      // All x in Z2, -x = x
    }
    quoRem[1].coefficients[0] = _1_;                                            // Putting the -1 that is at the end of the polynomial x^N-1. 1 == -1 in Z2
    for(i = this->N-1 - deg, j = this->N-1; j >= deg; i = j - deg) {            // Continuing with division algorithm; i is the place of the next coefficient of
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
        catch(const char* exp) {
            std::cout << "\nNTRU_ZqPolynomial::Z2Polynomial NTRU_ZqPolynomial"
            "::Z2Polynomial::gcdXNmns1(Z2Polynomial& thisBezout) const\n";
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

bool NTRU_ZqPolynomial::Z2Polynomial::operator == (const Z2Polynomial& P)const{
	if(this->N == P.N) {
		for(int i = 0; i < this->N; i++)
			if(this->coefficients[i] != P.coefficients[i])
				return false;
		return true;
	}
	else return false;
}

void NTRU_ZqPolynomial::Z2Polynomial::print(const char* name,const char* tail)
const{
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printArray(array, (unsigned)coeffAmount, 2, name, tail);
    delete[] array;
}

void NTRU_ZqPolynomial::Z2Polynomial::println(const char* name) const{
    this->print(name, "\n");
}

void NTRU_ZqPolynomial::Z2Polynomial::test(NTRU_N _N_, int d) const{
    Z2Polynomial Np0(_N_, d);
	Z2Polynomial Np1(_N_, d);
	Z2Polynomial quorem[2];
	Z2Polynomial gcd;
	Z2Polynomial Bezout;
	Z2Polynomial Np2(Np0);

	std::cout << "\n::::"
	"NTRU_ZqPolynomial::Z2Polynomial testing start ..........................."
	"........................."
	<< '\n';
    try { Np0.division(Np1, quorem); }
	catch(const char* exp) { std::cout << exp; }

	if( Np1*quorem[0] + quorem[1] == Np0 && Np1.degree() > quorem[1].degree())
	    std::cout << "\nSuccesful division...\n";
	else
	    std::cout << "\nDivision filure...\n";

	try{ gcd = Np2.gcdXNmns1(Bezout); }
	catch(const char* exp) { std::cout << exp; }
	gcd.print("gcd", "\n");
	//Bezout.println("Bezout");
	//Np2.println("Np2");
	(Np2*Bezout).println("Np2*Bezout");

	std::cout << "\n::::"
	"NTRU_ZqPolynomial::Z2Polynomial testing end ............................."
	".........................."
	<< "\n\n";
}

//_____________________NTRU_ZqPolynomial::Z2Polynomial__________________________

// |||||||||||||||||||||||||| NTRU_ZqPolynomial ||||||||||||||||||||||||||||||||

NTRU_ZqPolynomial::NTRU_ZqPolynomial(NTRU_N _N_, NTRU_q _q_):N(_N_),_Zq_(_q_) {
    this->coefficients = new int[_N_];
    for(int i = 0; i < this->N; i++) this->coefficients[i] = 0;
}

NTRU_ZqPolynomial::NTRU_ZqPolynomial(const NTRU_ZpPolynomial& P,NTRU_q _q_):
N(P.get_N()), _Zq_(_q_) {
	this->coefficients = new int[this->N];
	for(int i = 0; i < this->N; i++) this->coefficients[i] = P[i];
}

NTRU_ZqPolynomial::NTRU_ZqPolynomial(const NTRU_ZqPolynomial& P): N(P.N),
_Zq_(P._Zq_) {
    this->coefficients = new int[P.N];
    for(int i = 0; i < P.N; i++) this->coefficients[i] = P.coefficients[i];
}

NTRU_ZqPolynomial::NTRU_ZqPolynomial(const Z2Polynomial& P,NTRU_q _q_):
N(P.get_N()), _Zq_(_q_) {
    this->coefficients = new int[this->N];
    for(int i = 0; i < this->N; i++) this->coefficients[i] = P[i];
}

NTRU_ZqPolynomial& NTRU_ZqPolynomial::operator = (const NTRU_ZqPolynomial& P) {
	if(this != &P) {														    // Guarding against self assignment
		if(this->N != P.N) {												    // Delete pass coefficients array. If this->N != P.N is true, there is no reason
			if(this->coefficients != NULL) delete[] this->coefficients;		    // to delete the array
			this->coefficients = new int[P.N];
			this->N = P.N;
		}
		if(this->coefficients == NULL) this->coefficients = new int[P.N];
		this->_Zq_ = P._Zq_;
		for(int i = 0; i < this->N; i++)
			this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

NTRU_ZqPolynomial& NTRU_ZqPolynomial::operator = (const NTRU_ZpPolynomial& P) {
    NTRU_N P_N = P.get_N();
	if(this->N != P_N) {											            // Delete past coefficients array. If this->N != P.N is true, there is no reason
		if(this->coefficients != NULL) delete[] this->coefficients;	            // to delete the array
		this->coefficients = new int[P_N];
		this->N = P_N;
	}
	if(this->coefficients == NULL) this->coefficients = new int[P_N];           // In case of having an object created with the default (private) constructor
	for(int i = 0; i < P_N; i++) this->coefficients[i] = P[i];                  // Fitting the ZpPolynomial in a ZqPolynomial
	return *this;
}

NTRU_ZqPolynomial NTRU_ZqPolynomial::operator - (const NTRU_ZqPolynomial& P)
const{
    NTRU_ZqPolynomial r(max_N(this->N, P.N),
                        max_q(this->_Zq_.get_q(),P._Zq_.get_q()) );             // Initializing result in the "biggest polynomial ring"
    int i, small_degree, big_degree;                                            // Degree of the polynomials, small_degree <= big_degree

    if(this->degree() < P.degree()) {
        small_degree = this->degree();
        big_degree = P.degree();
    }
	else {
	    small_degree = P.degree();
	    big_degree = this->degree();
	}

    for(i = 0; i <= small_degree; i++)
        r.coefficients[i] =
        r._Zq_.subtract(this->coefficients[i],P.coefficients[i]);               // Subtraction element by element till the smallest degree of the arguments
    if(this->degree() == big_degree)
        for(; i <= big_degree; i++)
            r.coefficients[i] = this->coefficients[i];
    else
        for(; i <= big_degree; i++)
            r.coefficients[i] = r._Zq_.negative(this->coefficients[i]);
    return r;
}

NTRU_ZqPolynomial NTRU_ZqPolynomial::operator * (const NTRU_ZqPolynomial& P)
const{
    NTRU_ZqPolynomial r(max_N(this->N, P.N),
                        max_q(this->_Zq_.get_q(),P._Zq_.get_q()));              // Initializing result in the "biggest polynomial ring"
    int i, j, k;
    int this_degree = this->degree();
    int P_degree    = P.degree();

	for(i = 0; i <= this_degree; i++) {
		if(this->coefficients[i] != 0)                                          // Taking advantage this polynomials have a big proportion of zeros
		    for(j = 0; j <= P_degree; j++) {
			    if(P.coefficients[j] != 0) {
			        if((k = i + j) < r.N) {
			            r.coefficients[k] += r._Zq_.product(
			            this->coefficients[i], P.coefficients[j]);
			        } else {
			            k -= r.N;
			            r.coefficients[k] += r._Zq_.product(
			            this->coefficients[i], P.coefficients[j]);
			        }
			        r.coefficients[k] = r._Zq_.mod_q(r.coefficients[k]);
			    }
		    }
	}
	return r;
}

NTRU_ZqPolynomial operator - (int t, const NTRU_ZqPolynomial& P) {
    NTRU_ZqPolynomial r(P.N, P.get_q());
    r.coefficients[0] = P._Zq_.subtract(t, P.coefficients[0]);
    for(int i = 1; i < P.N; i++)
        r.coefficients[i] = P._Zq_.negative(P.coefficients[i]);
    return r;
}

void NTRU_ZqPolynomial::print(const char* name,const char* tail)
const{
    unsigned len_q = len(this->_Zq_.get_q());
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printArray(array, (unsigned)coeffAmount, len_q+1, name, tail);
    delete[] array;
}

void NTRU_ZqPolynomial::println(const char* name) const{
    this->print(name, "\n");
}

const int CenteredPolynomial::ZpCentered::Z3[3] =          { 0, 1,-1};          // Centered Z3

const int CenteredPolynomial::ZpCentered::Z3add [3][3] = { { 0, 1,-1},          // Addition table Z3 centered
                                                           { 1,-1, 0},
                                                           {-1, 0, 1} };

const int CenteredPolynomial::ZpCentered::Z3subs[3][3] = { { 0,-1, 1},          // Subtractiontion table Z3 centered
                                                           { 1, 0,-1},
                                                           {-1, 1, 0} };

int CenteredPolynomial::ZpCentered::addition(int a, int b) const{               // We're assuming a and b are in {-p/2 - 1,...,0,..,p/2 - 1}
    if(a == -1) a = 2;
    if(b == -1) b = 2;
    return this->Z3add[a][b];
}

int CenteredPolynomial::ZpCentered::subtraction(int a, int b) const{            // We're assuming a and b are in {-p/2 - 1,...,0,..,p/2 - 1}
    if(a == -1) a = 2;
    if(b == -1) b = 2;
    return this->Z3subs[a][b];
}

int CenteredPolynomial::ZpCentered::product(int a, int b) const{                // We're assuming a and b are in {-p/2 - 1,...,0,..,p/2 - 1}
    if(a == 0) return 0;
    if(b == 0) return 0;
    if(a == -1) if(b == -1) return 1;
    return 1;
}

int CenteredPolynomial::ZqCentered::addition(int a, int b) const{
    int r = a+b;
    if(r < 0) r += this->q;
    r &= this->q_1;
    if(r > (this->q >> 1)) return r-this->q;
    return r;
}

CenteredPolynomial::CenteredPolynomial(const NTRU_ZpPolynomial& P, NTRU_q _q_):
N(P.get_N()), _Zp_(P.get_p()), _Zq_(_q_) {
    int _p_ = _Zp_.get_p();
    int p_div_2 = _p_ >> 1, i;                                              // Dividing by two. Possible optimization through a case by case function
    this->coefficients = new int[this->N];
    switch(_p_) {
    case _3_:
        for(i = 0; i < this->N; i++) this->coefficients[i]=ZpCentered::Z3[P[i]];
    }
}

CenteredPolynomial::CenteredPolynomial(const NTRU_ZqPolynomial& P, NTRU_p _p_):
N(P.get_N()), _Zp_(_p_), _Zq_(P.get_q()) {
    int _q_ = _Zq_.get_q();
    int q_div_2 = _q_ >> 1;                                                 // Dividing by two. Possible optimization through a case by case function
    this->coefficients = new int[this->N];
    for(int i = 0; i < this->N; i++) {                                          // Centering coefficients
        if((int)P[i] > q_div_2) this->coefficients[i] = (int)(P[i] - _q_);
        else this->coefficients[i] = (int)P[i];
    }
}

CenteredPolynomial::~CenteredPolynomial() {
    if(this->coefficients != NULL) delete[] this->coefficients;
}

/*CenteredPolynomial CenteredPolynomial::operator*(const CenteredPolynomial& P)
const {

}*/

//___________________________ NTRU_ZqPolynomial ________________________________
