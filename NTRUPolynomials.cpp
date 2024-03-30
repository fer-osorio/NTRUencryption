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
    int i = 0;                                                                  // Counting variable. At the end it will contain the length of the origin string
    for(; origin[i] != 0; i++) {dest[i] = origin[i];}                           // Coping element by element
    dest[i] = 0;                                                                // End of string.
    return i;                                                                   // Returning string length
}
inline static int printSpaces(unsigned t) {
	while(t-- > 0) std::cout << ' ' ;
	return 0;
}																				// Functions for printing
inline static int len(char* str) {												// Length of a string
	int l = -1;
	while(str[++l] != 0) {}
	return l;
}

class RandInt {                                                                 // Little class for random integers. Taken from The C++ Programming Language 4th
    public:                                                                     // Edition Bjarne Stroustrup
    RandInt(int low, int high, unsigned seed): re{seed}, dist{low,high} {}
    int operator()() { return dist(re); }                                       // Draw an int
    private:
    std::default_random_engine re;
    std::uniform_int_distribution<> dist;
};

NTRU_ZpPolynomial::PolyModXNmns1::PolyModXNmns1(const NTRU_ZpPolynomial& P,
NTRU_N _N_): N(_N_), p(P.p){
    int upperLimit = min(this->N, P.coeffAmount()), i;
	this->coefficients = new int[this->N];
	for(i = 0; i < upperLimit; i++)
		this->coefficients[i] = P.coefficients[i];
	if(upperLimit < this->N)
		for(; i < this->N; i++)
			this->coefficients[i] = 0;
	else for(; i < upperLimit; i++)
		this->coefficients[i%this->N] =
		Z3addition[this->coefficients[i%this->N]][P.coefficients[i]];
}

NTRU_ZpPolynomial::PolyModXNmns1::PolyModXNmns1(NTRU_N _N_,int ones,int twos,
NTRU_p _p_): N(_N_), p(_p_) {
    int i, j;
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

void NTRU_ZpPolynomial::PolyModXNmns1::setPermutation() {                       // Naive way of setting a permutation
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
void NTRU_ZpPolynomial::PolyModXNmns1::permute() {
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

NTRU_ZpPolynomial::PolyModXNmns1 NTRU_ZpPolynomial::PolyModXNmns1::operator +
(const PolyModXNmns1& P) const{
    PolyModXNmns1 r(max_N(this->N, P.N));                                      // Initializing result in the "biggest polynomial ring"
    const PolyModXNmns1 *small, *big;
    int i;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                            // polynomial with the biggest N

    for(i = 0; i < small->N; i++)
        r.coefficients[i]=Z3addition[this->coefficients[i]][P.coefficients[i]]; // Addition element by element till the smallest degree of the arguments
    for(; i < big->N; i++)
        r.coefficients[i] = big->coefficients[i];                               // Just copying, equivalent to filling with zeros the small polynomial
    return r;
}

NTRU_ZpPolynomial::PolyModXNmns1 NTRU_ZpPolynomial::PolyModXNmns1::operator -
(const PolyModXNmns1& P) const{
    const NTRU_N _n_ = min_N(this->N, P.N), _N_ = max_N(this->N, P.N);
    PolyModXNmns1 r(_N_);                                                      // Initializing result in the "biggest polynomial ring"
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

NTRU_ZpPolynomial::PolyModXNmns1 NTRU_ZpPolynomial::PolyModXNmns1::operator *
(const PolyModXNmns1& P) const{
    PolyModXNmns1 r(max_N(this->N, P.N));                                      // Initializing with zeros
    const PolyModXNmns1 *small, *big;
    int i, j, k;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                            // polynomial with the biggest N

    for(i = 0; i < r.N; i++) {                                                  // Convolution process
        k = min(i, small->N);
        for(j = 0; j <= k; j++)
            r.coefficients[i] = Z3addition[ r.coefficients[i] ]
            [ Z3product[ small->coefficients[j] ][ big->coefficients[i-j] ] ];
        for(; j < small->N; j++)
            r.coefficients[i] = Z3addition[ r.coefficients[i] ]
            [Z3product[small->coefficients[j] ][ big->coefficients[r.N+i-j]]];
    }
    return r;
}

NTRU_ZpPolynomial::PolyModXNmns1& NTRU_ZpPolynomial::PolyModXNmns1::operator -=
(const PolyModXNmns1& P) {
    const NTRU_N _n_ = min_N(this->N, P.N);
    int i;
    for(i = 0; i < _n_; i++)
        this->coefficients[i] =                                                 // Subtraction element by element till the smallest degree of the arguments
        Z3subtraction[this->coefficients[i]][P.coefficients[i]];
    return *this;
}

NTRU_ZpPolynomial::NTRU_ZpPolynomial(int zeros, int ones, int twos):
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

NTRU_ZpPolynomial& NTRU_ZpPolynomial::operator = (const NTRU_ZpPolynomial& P) {
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

NTRU_ZpPolynomial NTRU_ZpPolynomial::operator + (const NTRU_ZpPolynomial& P)
const{
    NTRU_ZpPolynomial r(max(this->degree, P.degree),_3_);                       // Initializing result in the "biggest polynomial ring"
    const NTRU_ZpPolynomial *small, *big;
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

NTRU_ZpPolynomial NTRU_ZpPolynomial::operator - (const NTRU_ZpPolynomial& P)
const{
	NTRU_ZpPolynomial r(max(this->degree, P.degree), _3_);                      // Initializing result with zeros
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

NTRU_ZpPolynomial NTRU_ZpPolynomial::operator * (const NTRU_ZpPolynomial& P)
const{
    NTRU_ZpPolynomial r(this->degree + P.degree, _3_);
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

void NTRU_ZpPolynomial::division(const NTRU_ZpPolynomial& P, NTRU_ZpPolynomial
result[2]) const{
    if(P == 0) {
        throw "\nIn NTRUencryption.cpp, function void NTRU_ZpPolynomial::"
        "division(const NTRU_ZpPolynomial& P, NTRU_ZpPolynomial result[2])"
        "Division by zero...\n";
    }
    if(*this == 0) {                                                            // Case zero divided by anything
        result[0] = NTRU_ZpPolynomial(0,_3_);                                       // Zero polynomial
        result[1] = NTRU_ZpPolynomial(0,_3_);                                       // Zero polynomial
        return;
    }

    const int dividendDegree = this->degree;
    const int divisorDegree  = P.degree;
    int degreeDiff;                                                             // Difference between degrees
    int remDeg;                                                                 // Remainder degree
    const int leadCoeffDivsrInv = P.coefficients[divisorDegree];                // In Z3, each element is its own inverse
    int i;                                                                      // For counting

    if(dividendDegree < divisorDegree) {                                        // Case dividend has smaller degree than divisor
        result[0] = 0; result[1] = *this;
        return;
    }                                                                           // At this point we know leading coefficient has an inverse in Zq

    degreeDiff = dividendDegree - divisorDegree;                                // At this point we know degreeDiff >= 0
    remDeg = dividendDegree;
    result[1] = *this;                                                          // Initializing remainder with dividend (this)
    result[0] = NTRU_ZpPolynomial(degreeDiff, this->p);

    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        result[0].coefficients[degreeDiff] = Z3product[
        leadCoeffDivsrInv][result[1].coefficients[remDeg]];                     // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] =
            Z3subtraction[result[1].coefficients[i]][Z3product[
            result[0].coefficients[degreeDiff]][P.coefficients[i-degreeDiff]]];
        }

        if(result[1].coefficients[remDeg] != 0){                                 // No congruence with 0 mod q, throwing exception
            result[1].println("result[1]");
            result[0].println("result[0]");
            throw "\nIn NTRUencryption.cpp, function void NTRU_ZpPolynomial::"
            "division(const NTRU_ZpPolynomial& P, NTRU_ZpPolynomial result[2]"
            "). result[1].coefficients[remDeg] != 0\n";}                         // At this point we know result[1].coefficients[remDeg] = 0

        while(remDeg >= 0 && result[1].coefficients[remDeg] == 0) remDeg--;     // Updating value of the degree of the remainder
    }
    if(remDeg < 0) remDeg = 0;
    result[1].degree = remDeg;                                                  // Adjusting the polynomial in order to have no zeros to the left
}

NTRU_ZpPolynomial NTRU_ZpPolynomial::PolyModXNmns1::gcdXNmns1(PolyModXNmns1&
thisBezout) const{                                                              // EEA will mean Extended Euclidean Algorithm
    NTRU_ZpPolynomial gcd, _thisBezout_;                                        // Initializing result in the "biggest polynomial ring
    NTRU_ZpPolynomial remainders;
    NTRU_ZpPolynomial tmp[2];
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    int leadCoeff = this->coefficients[deg];                                    // Lead coefficient of this polynomial
    NTRU_ZpPolynomial quoRem[2] = { NTRU_ZpPolynomial(this->N-deg,this->p),
                                    NTRU_ZpPolynomial(this->N-1,  this->p) };

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
    }
    quoRem[1].correctDegree();                                                  // End of division algorithm between virtual polynomial x^N-1 and this

    /*NTRU_ZpPolynomial XNmns1(this->N,_3_);                                    // Debugging purposes
    XNmns1.coefficients[this->N] = 1;                                           // ...
    XNmns1.coefficients[0] = 2;                                                 // ...
    if(quoRem[0]*(*this) + quoRem[1] == XNmns1)                                 // ...
        std::cout << "\nFirst step successful...\n";                            // ...
    else                                                                        // ...
        std::cout << "\nSomething went wrong...\n";*/                           // ...

    tmp[1] = -quoRem[0]; _thisBezout_ = 1;                                      // Initializing values for the execution of the rest of the EEA
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
        tmp[1] = _thisBezout_ - quoRem[0]*tmp[0];                               // u[k+2] = u[k] - q[k+2]*u[k+1]
        _thisBezout_ = tmp[0];                                                  // Updating values
        tmp[0] = tmp[1];                                                        // ...
        gcd = remainders;                                                       // ...
        remainders = quoRem[1];                                                 // ...
	}
	thisBezout = _thisBezout_;
	return gcd;
}

NTRU_ZpPolynomial::PolyModXNmns1& NTRU_ZpPolynomial::PolyModXNmns1::operator =
(const NTRU_ZpPolynomial::PolyModXNmns1& P) {
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

NTRU_ZpPolynomial::PolyModXNmns1& NTRU_ZpPolynomial::PolyModXNmns1::operator =
(const NTRU_ZpPolynomial& P) {
    int upperLimit = min(this->N, P.coeffAmount()), i;
	if(this->coefficients == NULL)
		this->coefficients = new int[this->N];
	for(i = 0; i < upperLimit; i++)
		this->coefficients[i] = P.coefficients[i];
	if(upperLimit < this->N)
		for(; i < this->N; i++)
			this->coefficients[i] = 0;
	else for(; i < upperLimit; i++)
		this->coefficients[i%this->N] =
		Z3addition[this->coefficients[i%this->N]][P.coefficients[i]];
	this->p = P.p;
	return *this;
}

void NTRU_ZpPolynomial::PolyModXNmns1::print(const char* name) const{
    char start[] = "0   [";                                                     // Start of the string will be printed
    char numBuf[10];                                                            // Buffer necessary for the int -> string conversion
    int qlen, strLen;                                                           // q length in characters, start length in characters
    int i = 0, j = 0;
    int deg = this->degree();
    int startLen;

    intToString((int)this->p, numBuf);                                          // Conversion from number to string
    qlen   = len(numBuf) + 1;
    startLen = len(start);

    std::cout << name << " = \n";
    std::cout << start;
    do {
        if(i != 0 && (i & 31) == 0 && i != deg) {                               // Since 2^5 = 32, then i&31 = i % 32
            std::cout << '\n';
            j++; intToString(j, numBuf);
            std::cout << numBuf;                                                // Printing current coefficient
            strLen = len(numBuf);
            printSpaces((unsigned)max(startLen - strLen,0));                    // Padding with spaces
        }
        intToString(this->coefficients[i], numBuf);                             // Optimize by returning the length of the string
        std::cout << numBuf;                                                    // Printing current coefficient
        strLen = len(numBuf);
        printSpaces((unsigned)max(qlen - strLen ,0));                           // Padding with spaces
        if(i < deg) std::cout << ',';
    }while(++i <= deg);
    std::cout << ']';
}

void NTRU_ZpPolynomial::print(const char* name) const{
    char start[] = "0   [";                                                     // Start of the string will be printed
    char numBuf[10];                                                            // Buffer necessary for the int -> string conversion
    int qlen, strLen;                                                           // q length in characters, start length in characters
    int i = 0, j = 0;
    int deg = this->degree;
    int startLen;

    intToString((int)this->p, numBuf);                                          // Conversion from number to string
    qlen   = len(numBuf) + 1;
    startLen = len(start);

    std::cout << name << " = \n";
    std::cout << start;
    do {
        if(i != 0 && (i & 31) == 0 && i != deg) {                               // Since 2^5 = 32, then i&31 = i % 32
            std::cout << '\n';
            j++; intToString(j, numBuf);
            std::cout << numBuf;                                                // Printing current coefficient
            strLen = len(numBuf);
            printSpaces((unsigned)max(startLen - strLen,0));                    // Padding with spaces
        }
        intToString(this->coefficients[i], numBuf);                             // Optimize by returning the length of the string
        std::cout << numBuf;                                                    // Printing current coefficient
        strLen = len(numBuf);
        printSpaces((unsigned)max(qlen - strLen ,0));                           // Padding with spaces
        if(i < deg) std::cout << ',';
    }while(++i <= deg);
    std::cout << ']';
}

void NTRU_ZpPolynomial::PolyModXNmns1::test(int d) {
    NTRU_ZpPolynomial Np0(d,d+1,d);
	NTRU_ZpPolynomial Np1(d, d, d);
	NTRU_ZpPolynomial quorem[2];
	NTRU_ZpPolynomial gcd;
	NTRU_ZpPolynomial::PolyModXNmns1 Bezout;
	NTRU_ZpPolynomial::PolyModXNmns1 Np2(Np0,this->N);

	std::cout << "\n::::"
	"NTRUPolynomials testing start ..........................................."
	"........................................................................."
	<< '\n';
	/*Np0.println("\nNp0");
	Np1.println("\nNp1");*/

    try { Np0.division(Np1, quorem); }
	catch(const char* exp) { std::cout << exp; }

    //quorem[0].println("\nquotient");
	//quorem[1].println("\nremainder");

	if( Np1*quorem[0] + quorem[1] == Np0 && Np1.degree > quorem[1].degree &&
	    quorem[1].degree >= 0) std::cout << "\nSuccesful division.\n";

	try{ gcd = Np2.gcdXNmns1(Bezout); }
	catch(const char* exp) { std::cout << exp; }
	gcd.println("gcd");
	//Bezout.println("Bezout");
	//Np2.println("Np2");
	(Np2*Bezout).println("Np2*Bezout");

	std::cout << "\n::::"
	"NTRUPolynomials testing end ............................................."
	"........................................................................."
	<< "\n\n";
}
