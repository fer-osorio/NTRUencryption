#include<fstream>
#include<random>
#include<ctime>
#include"NTRUencryption.hpp"

#define DECIMAL_BASE 10

union longlong_to_char {                                                        // Allows to cast from an long long to an array of four bytes (char)
    long long  _longlong_;
    char       _char[8];
};

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
inline static NTRU_p max_p(NTRU_p a, NTRU_p b) {
	if(a < b) return b;
	return a;
}
static int intToString(int n, char* dest) {                                     // String representation of unsigned integer it returns the length of the string
    int i = 0, j = 0, l = 0;
    char buff = 0;
    if(n < 0) {	dest[i++] = '-'; n = -n; j = 1; }
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
static int printSpaces(unsigned t) {
	while(t-- > 0) std::cout << ' ' ;
	return 0;
}																				// Functions for printing
static int len(const char* str) {										        // Length of a string
	int l = -1;
	while(str[++l] != 0) {}
	return l;
}
static unsigned len(int a) {                                                    // -Returns the number of decimal characters for the number a
    unsigned l = 0;
    do { a /= DECIMAL_BASE; l++;
    }while(a > 0);
    return l;
}
static void printArray(int* array, unsigned arrlen, unsigned columnlen, const char* name = "", const char* tail = "") {
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
        printSpaces((unsigned)max((int)columnlen - strLen + 1,0));              // Padding with spaces. The +1 is for alignment with negative numbers
        std::cout << buff;                                                      // Printing current coefficient
        if(i < lastIndex) std::cout << ',';
    }while(++i < (int)arrlen);
    std::cout << ']';
    std::cout << tail;
}

static bool compareStrings(const char* str1, const char* str2) {
    int i = 0;
    if(str1 == NULL || str2 == NULL) return false;
    while(str1[i] == str2[i] && str1[i] != 0) { i++; }
    return str1[i] == str2[i];
}

static bool copyStrings(const char* origin, char* destination) {
    if(origin == NULL || destination == NULL) return false;
    for(int i = 0; origin[i] != 0; i++) destination[i] = origin[i];
    return true;
}

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| NTRU Parameters |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

struct NTRU_Parameters {                                                        // -The intention of this structure is to protect the NTRU parameters against
    private: NTRU_N N = _1499_;                                                 //  not intentional modifications.
    private: NTRU_p p = _3_;
    public:  NTRU_N get_N() const{ return this->N; }
    public:  NTRU_p get_p() const{ return this->p; }
    class Zq {																	// Representation of the group of integers modulus q: Zq
		NTRU_q 	q;
		int64_t q_1;															// Will hold q-1, this will help with mod q operation
		int64_t negq_1;															// This will help in the centering process. Set as ~q_1
		int64_t q_div_2;

		public: Zq(NTRU_q _q_): q(_q_), q_1((int64_t)_q_-1), negq_1(~q_1), q_div_2(q>>1) {}
		public: NTRU_q get_q() const{ return q; }

		int64_t mod_q(int64_t t) const{											// operation t % q
    		if(t >= 0)	return t & this->q_1;									// Equivalent to t % q since q is a power of 2
    		else 		return (t | this->negq_1);								// Computing q - (-t%q) because -t = -(Q-1)q + (q-r) 0 <= r < q
		}
		int64_t mods_q(int64_t a) const{
    		int64_t r;
    		if(a >= 0) r = a & this->q_1;										// Equivalent to a % q since q is a power of 2
    		else r = (a | this->negq_1) & this->q_1;							// Computing q - (-a%q) because -t = -(Q-1)q + (q-r) 0 <= r < q
    		if(r < this->q_div_2) return r;										// At this point we know 0 <= r < q
    		else return r | this->negq_1;										// This is equivalent to r - this->q when r < q
		}
		int64_t add(int64_t a, int64_t b) const{
			return this->mod_q(a+b);
		}
		void addEqual(int64_t& a, int64_t b) const{
			a = this->mod_q(a+b);
		}
		int64_t subtract(int64_t a, int64_t b) const{
			return this->mod_q(a-b);
		}
		int64_t product(int64_t a, int64_t b) const{
			return this->mod_q(a*b);
		}
		friend void NTRU::Encryption::setNTRUparameters(NTRU_N, NTRU_q) const;
	};
	friend void NTRU::Encryption::setNTRUparameters(NTRU_N, NTRU_q) const;
};

static NTRU_Parameters NTRUparameters;                                          // -This unique instance puts every polynomial "inside the same polynomial ring"
static NTRU_Parameters::Zq zq(_8192_);                                          // -Arithmetic for the operation modulo q

// ____________________________________________________________________ NTRU Parameters ___________________________________________________________________________

class RandInt {                                                                 // Little class for random integers. Taken from The C++ Programming Language 4th
    static unsigned _seed_;
    public:                                                                     // Edition Bjarne Stroustrup
    RandInt(int low, int high): re(this->_seed_), dist(low,high) {}
    int operator()() { return dist(re); }                                       // Draw an int
    ~RandInt() { this->_seed_++; }
    private:
    std::default_random_engine re;
    std::uniform_int_distribution<> dist;
};
unsigned RandInt::_seed_ = (unsigned)time(NULL);
static RandInt randomIntegersN(0, NTRUparameters.get_N() - 1);                  // -Random integers from 0 to N-1.


using namespace NTRU;

const ZpPolynomial::Z3 ZpPolynomial::Z3add [3][3] = {{_0_,_1_,_2_},             // Addition table of the Z3 ring (integers modulo 3)
                                                     {_1_,_2_,_0_},             // ...
                                                     {_2_,_0_,_1_}};            // ...

const ZpPolynomial::Z3 ZpPolynomial::Z3subt[3][3] = {{_0_,_2_,_1_},             // Addition table of the Z3 ring (integers modulo 3)
                                                     {_1_,_0_,_2_},             // ...
                                                     {_2_,_1_,_0_}};            // ...

const ZpPolynomial::Z3 ZpPolynomial::Z3prod[3][3] = {{_0_,_0_,_0_},             // Product table of the Z3 ring (integers modulo 3)
                                                     {_0_,_1_,_2_},             // ...
                                                     {_0_,_2_,_1_}};            // ...

const ZpPolynomial::Z3 ZpPolynomial::Z3neg[3]     =  {_0_,_2_,_1_};


// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ZpPolynomial |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

ZpPolynomial::ZpPolynomial() {
    const NTRU_N N = NTRUparameters.get_N();
	this->coefficients = new Z3[N];
	for(int i = 0; i < N; i++) this->coefficients[i] = _0_;
}

ZpPolynomial::ZpPolynomial(const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
	this->coefficients = new Z3[N];
	for(int i = 0; i < N; i++) this->coefficients[i] = P.coefficients[i];
}
/*
ZpPolynomial::ZpPolynomial(NTRU_N _N_,int ones,int twos,NTRU_p _p_): N(_N_), p(_p_) {
    int i, j;
    RandInt rn(0, _N_-1);                                                       // Random integers from 0 to N-1
    if(ones < 0) ones = -ones;                                                  // Guarding against invalid values of ones and twos. In particular the
    if(twos < 0) twos = -twos;                                                  // inequality ones + twos < N must follow
    while(ones + twos >= this->N) {                                             // Dividing by two till getting inside the allowed range
        ones <<= 1;                                                             // ...
        twos <<= 1;                                                             // ...
    }
	this->coefficients = new Z3[_N_];
	for(i = 0; i < _N_; i++) this->coefficients[i] = _0_;
	while(ones > 0) {                                                           // Putting the ones first
        j = rn();
        if(this->coefficients[j] == _0_) {
            this->coefficients[j] = _1_;
            ones--;
        }
	}
	while(twos > 0) {                                                           // Then the negative ones
        j = rn();
        if(this->coefficients[j] == _0_) {
            this->coefficients[j] = _2_;                                          // Two is congruent with -1 modulo 3
            twos--;
        }
	}
}                                                                               // Maybe is some room for optimization using JV theorem
*/

ZpPolynomial::ZpPolynomial(const char data[], int dataLength) {
    int i,j,k,l;
    const NTRU_N N = NTRUparameters.get_N();

    this->coefficients = new Z3[N];
    for(i = 0; i < N; i++) this->coefficients[i] = _0_;
    if(dataLength <= 0) return;                                                 // Guarding against negative or null dataLength

    for(i = 0, j = 0; i < dataLength && j < N; i++) {                           // i will run through data, j through coefficients
        l = (int)(unsigned char)data[i];
        for(k = 0; k < 5 && j < N; k++, l/=3) {                                 // Here we're supposing _p_ == 3. Basically we're changing from base 2 to base 3
            switch(l%3) {                                                       // in big endian notation. Notice that the maximum value allowed is 242
                case  1:
                    this->coefficients[j++] = _1_;
                break;
                case  2:
                    this->coefficients[j++] = _2_;
                break;
                default:
                    this->coefficients[j++] = _0_;
            }
        }
    }                                                                           // -The rest of the coefficients will be left as zero
}

ZpPolynomial ZpPolynomial::randomTernary() {
    const NTRU_N N = NTRUparameters.get_N();
    const int d = N/3;
    int i, j, dd = d;
    ZpPolynomial r;

	r.coefficients = new Z3[N];
	for(i = 0; i < N; i++) r.coefficients[i] = _0_;
	while(dd > 0) {                                                             // Putting the ones first
        j = randomIntegersN();
        if(r.coefficients[j] == _0_) {
            r.coefficients[j] = _1_;
            dd--;
        }
	}
	dd = d;
	while(dd > 0) {                                                             // Then the negative ones
        j = randomIntegersN();
        if(r.coefficients[j] == _0_) {
            r.coefficients[j] = _2_;                                            // Two is congruent with -1 modulo 3
            dd--;
        }
	}
	return r;
}

void ZpPolynomial::changeAzeroForAone() {
    int i = randomIntegersN();
    while(this->coefficients[i] != _0_) i = randomIntegersN();                  // -Looking for a zero in a random position
    this->coefficients[i] = _1_;                                                // -Changing zero for one
}

ZpPolynomial ZpPolynomial::getPosiblePrivateKey() {
    ZpPolynomial r = ZpPolynomial::randomTernary();                             // -Setting d = N/3, r is a polynomial with d 1's and d -1's
    r.changeAzeroForAone();                                                     // -Adding one more one
    return r;
}

ZpPolynomial ZpPolynomial::operator + (const ZpPolynomial& P) const{
    ZpPolynomial r;                                                             // Initializing result
    const NTRU_N N = NTRUparameters.get_N();
    for(int i = 0; i < N; i++)
        r.coefficients[i] = this->coefficients[i] + P.coefficients[i];          // -Addition element by element till the smallest degree of the arguments
    return r;
}

ZpPolynomial ZpPolynomial::operator - (const ZpPolynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
    ZpPolynomial r;                                                             // -Initializing result in the "biggest polynomial ring"
    for(int i = 0; i < N; i++)
        r.coefficients[i] = this->coefficients[i] - P.coefficients[i];          // -Subtraction element by element till the smallest degree of the arguments
    return r;
}

ZpPolynomial ZpPolynomial::operator - () const{
	ZpPolynomial r = *this;
	NTRU_N N = NTRUparameters.get_N();
	for(int i = 0; i < N; i++)
		r.coefficients[i] = -r.coefficients[i];
	return r;
}

ZpPolynomial ZpPolynomial::operator * (const ZpPolynomial& P) const{            // -Remember, this can be optimized ##############################################
    ZpPolynomial r;                                                             // Initializing with zeros
    int i, j, k;
    const NTRU_N N = NTRUparameters.get_N();

	for(i = 0; i < N; i++) {
		if(this->coefficients[i] != 0) {
		    k = N - i;
		    for(j = 0; j < k; j++)                                              // Adding and multiplying while the inequality i+j < N holds
			    if(P.coefficients[j] != 0)                                      // We expect a big proportion of zeros
			        r.coefficients[i+j] += this->coefficients[i]*P.coefficients[j];
		    for(k = 0; k < i; j++, k++)                                         // Using the definition of convolution polynomial rings
			    if(P.coefficients[j] != 0)                                      // Notice i+k is congruent with j mod N
			        r.coefficients[k] += this->coefficients[i]*P.coefficients[j];
		}
	}
    return r;
}

ZpPolynomial& ZpPolynomial::operator -= (const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
    for(int i = 0; i < N; i++)
        this->coefficients[i] = this->coefficients[i] - P.coefficients[i];      // Subtraction element by element till the smallest degree of the arguments
    return *this;
}

void ZpPolynomial::division(const ZpPolynomial& P, ZpPolynomial result[2]) const{
    if(P == _0_) {
        throw "\nvoid ZpPolynomial::ZpPolynomial::division(const Zp"
        "Polynomial& P,ZpPolynomial result[2]) const. Division by zero...\n";
    }
    if(*this == 0) {                                                            // Case zero divided by anything
        result[0] = _0_;                                                          // Zero polynomial
        result[1] = _0_;                                                          // Zero polynomial
        return;
    }

    const int dividendDegree = this->degree();
    const int divisorDegree  = P.degree();
    const Z3  leadCoeffDivsrInv = P.coefficients[divisorDegree];                // In Z3, each element is its own inverse
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
    result[0] = ZpPolynomial();

    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        result[0].coefficients[degreeDiff] = leadCoeffDivsrInv*result[1].coefficients[remDeg];  // Putting new coefficient in the quotient
        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] -= result[0].coefficients[degreeDiff]*P.coefficients[i-degreeDiff];
        }
        if(result[1].coefficients[remDeg] != _0_) {                             // No congruence with 0 mod p, throwing exception
            throw "\nvoid ZpPolynomial::ZpPolynomial::division(const ZpPolynomial& P,ZpPolynomial result[2]) const. result[1]. coefficients[remDeg] != 0\n";
        }                                                                       // At this point we know result[1].coefficients[remDeg] = 0
        while(remDeg >= 0 && result[1].coefficients[remDeg] == _0_) remDeg--;   // Updating value of the degree of the remainder
    }
}

ZpPolynomial ZpPolynomial::gcdXNmns1(ZpPolynomial& thisBezout) const{           // EEA will mean Extended Euclidean Algorithm
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    Z3 leadCoeff = this->coefficients[deg];                                     // Lead coefficient of this polynomial
    NTRU_N N = NTRUparameters.get_N();
    ZpPolynomial gcd;                                                           // Initializing result in the "biggest polynomial ring
    ZpPolynomial remainders;
    ZpPolynomial tmp[2] = {ZpPolynomial(),ZpPolynomial()};
    ZpPolynomial quoRem[2] = {ZpPolynomial(),ZpPolynomial()};

    quoRem[0].coefficients[N-deg] = leadCoeff;                                  // Start of division algorithm between virtual polynomial x^N-1 and this
    for(i = deg-1, j = N - 1; i >= 0; i--, j--) {                               // First coefficient of quotient and first subtraction
        quoRem[1].coefficients[j] = _0_ - leadCoeff * this->coefficients[i];
    }
    quoRem[1].coefficients[0] = _2_;                                            // Putting the -1 that is at the end of the polynomial x^N-1
    for(i = N-1 - deg, j = N-1; j >= deg; i = j - deg) {                        // Continuing with division algorithm; i is the place of the next coefficient of
        quoRem[0].coefficients[i] = leadCoeff*quoRem[1].coefficients[j];        // the quotient, j is the degree of the remainders.
        for(k = deg, l = j; k >= 0; k--, l--) {                                 // Multiplication-subtraction step
            quoRem[1].coefficients[l] -= quoRem[0].coefficients[i]*this->coefficients[k];
        }
        while(quoRem[1].coefficients[j] == 0) {j--;};
    }                                                                           // End of division algorithm between virtual polynomial x^N-1 and this

    tmp[1] = -quoRem[0]; thisBezout = _1_;                                      // Initializing values for the execution of the rest of the EEA
    tmp[0] = tmp[1];                                                            // v[-1] = 0, v[0] = 1 ==> v[1] = v[-1] - q[1]*v[0] = - q[1]
    gcd = *this;                                                                // ...
    remainders = quoRem[1];                                                     // ...

	while(remainders != _0_) {                                                    // EEA implementation (continuation)
        try{ gcd.division(remainders, quoRem); }
        catch(const char* exp) {
            std::cout << "\nIn NTRUencryption.cpp; function ZpPolynomial "
            "ZpPolynomial::PolyModXNmns1::gcdXNmns1(PolyModXNmns1& this"
            "Bezout)\n";
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

bool ZpPolynomial::operator == (const ZpPolynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
	for(int i = 0; i < N; i++) if(this->coefficients[i] != P.coefficients[i]) return false;
	return true;
}

bool ZpPolynomial::operator != (const ZpPolynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
	for(int i = 0; i < N; i++) if(this->coefficients[i] != P.coefficients[i]) return true;
	return false;
}

ZpPolynomial& ZpPolynomial::operator = (const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
    if(this != &P) {													        // Guarding against self assignment
        if(this->coefficients == NULL) this->coefficients = new Z3[N];
	    for(int i = 0; i < N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

ZpPolynomial& ZpPolynomial::operator = (int t) {
    const NTRU_N N = NTRUparameters.get_N();
    if(this->coefficients == NULL) this->coefficients = new Z3[N];              // Dealing with already initialized object, so this->N is well defined
	if(t < 0) t = -t;
	if(t >= 3) t %= 3;
	this->coefficients[0] = (Z3)t;
	for(int i = 1; i < N; i++) this->coefficients[i] = _0_;                     // Filling with zeros the rest of the array
	return *this;
}

int ZpPolynomial::operator [] (int i) const{
    const NTRU_N N = NTRUparameters.get_N();
	if(i < 0) i = -i;
	if(i > N) i %= N;
	return this->coefficients[i];
}

void ZpPolynomial::setPermutation() {                                           // Setting a permutation
    const NTRU_N N = NTRUparameters.get_N();
    int i, j, k, *tmp = new int[N];
    RandInt rn(0, 0x7FFFFFFF);                                                  // Random integers from 0 to the maximum number for and int
    if(this->permutation==NULL) this->permutation = new int[N];
    for(i = 0; i < N; i++)  tmp[i] = i;
    for(i = 0, j = N; i < N; i++, j--) {
        k = rn()%j;
        this->permutation[i] = tmp[k];
        tmp[k] = tmp[j-1];
    }
    delete[] tmp;
}
                                                                                // Implementation of the permutation
void ZpPolynomial::permute() {
	int i;
	const NTRU_N N = NTRUparameters.get_N();
	if(this->permutation == NULL) this->setPermutation();
	if(this->coeffCopy   == NULL) {                                             // If there is no a copy, create the copy
	    this->coeffCopy = new Z3[N];
	    for(i = 0; i < N; i++) this->coeffCopy[i] = this->coefficients[i];
	}
	for(i = 0; i < N; i++) this->coefficients[i] = this->coeffCopy[this->permutation[i]]; // Permute coefficients
	for(i = 0; i < N; i++) this->coeffCopy[i] = this->coefficients[i];          // Copy coefficients
}

int ZpPolynomial::degree() const{
    int deg = NTRUparameters.get_N();
	while(this->coefficients[--deg] == 0 && deg > 0) {}
	return deg;
}

static int64_t multiplyBy_3(int64_t t) {
    return (t << 1) + t;                                                        // -This expression is equivalent to t*2 + t
}

ZqPolynomial ZpPolynomial::encrypt(ZqPolynomial publicKey) const{
    NTRU_N N = NTRUparameters.get_N();
    ZqPolynomial encryption;
    int*  randTernaryTimes_p = new int[N];                                      // -Will represent the random polynomial needed for encryption
    const int d = N/3;
    int _d_ = d, i, j, k;
    int q_div_2 = zq.get_q() >> 1;
    int neg_qdiv2 = -q_div_2;
    int q_div_2_minus1 = q_div_2 - 1;

    for(i = 0; i < N; i++) randTernaryTimes_p[i] = 0;

    while(_d_ > 0) {
        i = randomIntegersN();                                                               // -Filling with threes. It represent the random polynomial multiplied by p
        if(randTernaryTimes_p[i] == 0) {randTernaryTimes_p[i] =  3; _d_--;}     //  ...
    }
    _d_ = d;
    while(_d_ > 0) {
        i = randomIntegersN();                                                               // -Filling with negative threes
        if(randTernaryTimes_p[i] == 0) {randTernaryTimes_p[i] = -3; _d_--;}     //  ...
    }
	for(i = 0; i < N; i++) {                                                    // -Convolution process
	    k = N - i;
	    if(randTernaryTimes_p[i] != 0) {
	        if(randTernaryTimes_p[i] == 3) {
	            for(j = 0; j < k; j++)                                          // -Ensuring we do not get out of the polynomial
                    encryption.coefficients[i+j] += multiplyBy_3(publicKey[j]);
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        encryption.coefficients[k] += multiplyBy_3(publicKey[j]);   // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
	        }
	        if(randTernaryTimes_p[i] == -3) {
	            for(j = 0; j < k; j++)                                          // Ensuring we do not get out of the polynomial
                    encryption.coefficients[i+j] -= multiplyBy_3(publicKey[j]);
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        encryption.coefficients[k] -= multiplyBy_3(publicKey[j]);   // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
	        }
	    }
	}
	encryption.mods_q();                                                        // -Obtaining center modulus for each coefficient
	for(i = 0; i < N; i++) {                                                    // -Adding this polynomial (adding message)
	    if(this->coefficients[i] == _1_)
	        if((++encryption.coefficients[i]) == q_div_2)                       // -Possible case: e[i] = q/2, but the -q/2 <= e[i] < q/2 must bet met, so
	            encryption.coefficients[i] = neg_qdiv2;                         //  e[i]<-e[i]-q = q/2 -(q/2 + q/2) = -q/2
	    if(this->coefficients[i] == _2_)
	        if((--encryption.coefficients[i]) < neg_qdiv2)                      // -Possible case: e[i] = -q/2-1, but the -q/2 <= e[i] < q/2 must bet met, so
	            encryption.coefficients[i] = q_div_2_minus1;                    //  e[i]<-e[i]+q = (-q/2-1) + (q/2 + q/2) = q/2 - 1
	}
	delete[] randTernaryTimes_p;
	return encryption;
}

void ZpPolynomial::toBytes(char dest[]) const{
    NTRU_N N = NTRUparameters.get_N();
    int i,j,k;
    int N_mod_5 = N%5;                                                    // -Since N is a prime number, N_mod_5 is always bigger than 0
    int _N_ = N - N_mod_5;
    int buff;
    for(i = 0, j = 0; i < _N_; i += 5, j++) {                                   // i will run through dest, j through coefficients
        for(k = 4, buff = 0; k >= 0; k--) buff = buff*3 + this->coefficients[i+k];// Here we're supposing _p_ == 3. Basically we're changing from base 3 to base 2
        dest[j] = (char)buff;                                                   // Supposing the numbers in base 3 are in big endian notation
    }
    for(k = N_mod_5-1, buff = 0; k >= 0; k--) buff = buff*3 + this->coefficients[i+k];// Supposing the numbers in base 3 are in big endian notation
    dest[j] = (char)buff;
}

void ZpPolynomial::print(const char* name, const char* tail) const{
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printArray(array, (unsigned)coeffAmount, 2, name, tail);
    delete[] array;
}

void ZpPolynomial::println(const char* name) const{
	this->print(name, "\n");
}

void ZpPolynomial::save(const char* name) const{
    NTRU_N N = NTRUparameters.get_N();
    int byteArrSize = N / 5 + 1;                                          // -this->N is necessarily prime, so this->N % 5 > 0 holds true
    char* byteArr = NULL;
    const char ntrup[] = "NTRUp";                                               // -This will indicate the binary file is saving a NTRU (NTRU) polynomial with
    std::ofstream file;                                                         //  coefficients in Zp (p)
    if(name == NULL) file.open("ZpPolynomial.ntrup");
    else             file.open(name);
    if(file.is_open()) {
        file.write(ntrup, 5);
        file.write((char*)&N, 2);                                         // -A short int for the degree of the polynomial
        byteArr = new char[byteArrSize];
        this->toBytes(byteArr);
        file.write(byteArr, byteArrSize);
    }
    if(byteArr != NULL) delete[] byteArr;
}

//_______________________________________________________________________ ZpPolynomial ___________________________________________________________________________

//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ZqPolynomial::Z2Polynomial ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

ZqPolynomial::Z2Polynomial::Z2Polynomial() {
    const NTRU_N N = NTRUparameters.get_N();
	this->coefficients = new Z2[N];
	for(int i = 0; i < N; i++) this->coefficients[i] = _0_;
}

ZqPolynomial::Z2Polynomial::Z2Polynomial(const Z2Polynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
    this->coefficients = new Z2[N];
    for(int i = 0; i < N; i++) this->coefficients[i] = P.coefficients[i];
}

ZqPolynomial::Z2Polynomial::Z2Polynomial(const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
	this->coefficients = new Z2[N];
	for(int i = 0; i < N; i++) {
	    if(P[i] != 0) this->coefficients[i] = _1_;                          // Two won't go to zero because it's the additive inverse of one in Z3, therefore
	    else this->coefficients[i] = _0_;                                   // it must go to the additive inverse of one in Z2, which if itself
    }
}

/*ZqPolynomial::Z2Polynomial::Z2Polynomial(int ones) {
	const NTRU_N N = NTRUparameters.get_N();
	int i, j;
	RandInt rn(0, N-1);                                                         // Random integers from 0 to N-1
    if(ones < 0) ones = -ones;                                                  // Guarding against invalid values of ones and twos. In particular the
    while(ones >= N) ones >>= 1;                                                // Dividing by two till getting inside the allowed range
	this->coefficients = new Z2[N];
	for(i = 0; i < N; i++) this->coefficients[i] = _0_;
	while(ones > 0) {                                                           // Putting the ones
        j = rn();
        if(this->coefficients[j] == _0_) {
            this->coefficients[j] = _1_;
            ones--;
        }
	}
}*/

ZqPolynomial::Z2Polynomial& ZqPolynomial::Z2Polynomial::operator = (const Z2Polynomial& P)  {
	if(this != &P) {													        // Guarding against self assignment
	    const NTRU_N N = NTRUparameters.get_N();
		if(this->coefficients == NULL) this->coefficients = new Z2[N];
		for(int i = 0; i < N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

ZqPolynomial::Z2Polynomial& ZqPolynomial::Z2Polynomial::operator = (const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
	if(this->coefficients == NULL) this->coefficients = new Z2[N];              // In case of having an object created with the default (private) constructor
	for(int i = 0; i < N; i++) {                                              // Fitting the ZpPolynomial in a Z2Polynomial
	    if(P[i] != 0) this->coefficients[i] = _1_;
	    else this->coefficients[i] = _0_;
	}
	return *this;
}

ZqPolynomial::Z2Polynomial& ZqPolynomial::Z2Polynomial::operator = (Z2 t)  {
    const NTRU_N N = NTRUparameters.get_N();
	if(this->coefficients == NULL) this->coefficients = new Z2[N];              // In case of have been generated from the (private) default constructor
	this->coefficients[0] = t;
	for(int i = 1; i < N; i++) this->coefficients[i] = _0_;
	return *this;
}

ZqPolynomial::Z2Polynomial ZqPolynomial::Z2Polynomial::operator + (const Z2Polynomial& P) const {
    const NTRU_N N = NTRUparameters.get_N();
    Z2Polynomial r;                                                             // -Initializing result with zeros
    for(int i = 0; i < N; i++)
        r.coefficients[i] = this->coefficients[i] + P.coefficients[i];          // -Addition element by element till the smallest degree of the arguments
    return r;
}

ZqPolynomial::Z2Polynomial ZqPolynomial::Z2Polynomial::operator - (const Z2Polynomial& P) const{
    return *this + P;                                                           // In this polynomial ring, subtraction coincide with addition
}

ZqPolynomial::Z2Polynomial ZqPolynomial::Z2Polynomial::operator * (const Z2Polynomial& P) const{ // Classical polynomial multiplication algorithm
    const NTRU_N N = NTRUparameters.get_N();
    Z2Polynomial r;                                                             // -Initializing with zeros
    int i, j, k;

	for(i = 0; i < N; i++) {
		if(this->coefficients[i] != 0) {
		    k = N - i;
		    for(j = 0; j < k; j++) {                                            // Adding and multiplying while the inequality i+j < N holds
			    if(P.coefficients[j] != 0) {                                    // We expect a big proportion of zeros
			        r.coefficients[i+j] += this->coefficients[i]*P.coefficients[j];
			    }
		    }
		    for(k = 0; k < i; j++, k++) {                                       // Using the definition of convolution polynomial rings
			    if(P.coefficients[j] != 0) {                                    // Notice i+j = i +(k+N-i), so i+j is congruent with k mod N
			        r.coefficients[k] += this->coefficients[i]*P.coefficients[j];
			    }
		    }
		}
	}
	return r;
}

void ZqPolynomial::Z2Polynomial::division(const Z2Polynomial& P, Z2Polynomial result[2]) const{
    if(P == _0_) {
        throw "\nvoid ZpPolynomial::ZpPolynomial::division(const Zp"
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
    result[0] = Z2Polynomial();
    for(;degreeDiff >= 0; degreeDiff = remDeg - divisorDegree) {
        result[0].coefficients[degreeDiff] =
        leadCoeffDivsrInv * result[1].coefficients[remDeg];                     // Putting new coefficient in the quotient

        for(i = remDeg; i >= degreeDiff; i--) {                                 // Updating remainder
            result[1].coefficients[i] -=
            result[0].coefficients[degreeDiff]*P.coefficients[i-degreeDiff];
        }

        if(result[1].coefficients[remDeg] != _0_) {                             // No congruence with 0 mod q, throwing exception
            throw "\nvoid ZpPolynomial::ZpPolynomial::division(const Zp"
            "Polynomial& P,ZpPolynomial result[2]) const. result[1]."
            "coefficients[remDeg] != 0\n";
        }                                                                       // At this point we know result[1].coefficients[remDeg] = 0
        while(remDeg >= 0 && result[1].coefficients[remDeg] == _0_) remDeg--;   // Updating value of the degree of the remainder
    }
}

ZqPolynomial::Z2Polynomial ZqPolynomial::Z2Polynomial::gcdXNmns1(Z2Polynomial& thisBezout) const{
    const NTRU_N N = NTRUparameters.get_N();
    Z2Polynomial gcd;                                                           // Initializing result with zeros
    Z2Polynomial remainders;
    Z2Polynomial tmp[2] = {Z2Polynomial(), Z2Polynomial()};
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    Z2 leadCoeff = this->coefficients[deg];                                     // Lead coefficient of this polynomial
    Z2Polynomial quoRem[2]={Z2Polynomial(), Z2Polynomial()};

    quoRem[0].coefficients[N-deg] = leadCoeff;                                  // Start of division algorithm between virtual polynomial x^N-1 and this
    for(i = deg-1, j = N - 1; i >= 0; i--, j--) {                               // First coefficient of quotient and first subtraction
        quoRem[1].coefficients[j] = this->coefficients[i];                      // All x in Z2, -x = x
    }
    quoRem[1].coefficients[0] = _1_;                                            // Putting the -1 that is at the end of the polynomial x^N-1. 1 == -1 in Z2
    for(i = N-1 - deg, j = N-1; j >= deg; i = j - deg) {                  // Continuing with division algorithm; i is the place of the next coefficient of
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
            std::cout << "\nNTRU_ZqPolynomial::Z2Polynomial ZqPolynomial"
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

bool ZqPolynomial::Z2Polynomial::operator == (const Z2Polynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
	for(int i = 0; i < N; i++) if(this->coefficients[i] != P.coefficients[i])  return false;
	return true;
}

ZqPolynomial::Z2Polynomial::Z2 ZqPolynomial::Z2Polynomial::operator[](int i) const{
    const NTRU_N N = NTRUparameters.get_N();
	if(i < 0) i = -i;
	if(i >= N) i %= N;
	return this->coefficients[i];
}

int ZqPolynomial::Z2Polynomial::degree() const{												// Returns degree of polynomial
	int deg = NTRUparameters.get_N();
	while(this->coefficients[--deg] == _0_ && deg > 0) {}
	return deg;
}

void ZqPolynomial::Z2Polynomial::print(const char* name,const char* tail) const{
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printArray(array, (unsigned)coeffAmount, 2, name, tail);
    delete[] array;
}

void ZqPolynomial::Z2Polynomial::println(const char* name) const{
    this->print(name, "\n");
}

//______________________________________________________________NTRU_ZqPolynomial::Z2Polynomial___________________________________________________________________

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ZqPolynomial |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

ZqPolynomial::ZqPolynomial() {
    const NTRU_N N = NTRUparameters.get_N();
    this->coefficients = new int64_t[N];
    for(int i = 0; i < N; i++) this->coefficients[i] = 0;
}

ZqPolynomial::ZqPolynomial(const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
	this->coefficients = new int64_t[N];
	for(int i = 0; i < N; i++) {
	    if(P[i] == 2) this->coefficients[i] = -1;                           // Since (1+2) mod 3 == 0, 2 must be sent to the additive inverse of 1 in Zq;
	    else          this->coefficients[i] = (int64_t)P[i];
	}
}

ZqPolynomial::ZqPolynomial(const ZqPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
    this->coefficients = new int64_t[N];
    for(int i = 0; i < N; i++) this->coefficients[i] = P.coefficients[i];
}

ZqPolynomial::ZqPolynomial(const Z2Polynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
    this->coefficients = new int64_t[N];
    for(int i = 0; i < N; i++) this->coefficients[i] = P[i];
}

ZqPolynomial& ZqPolynomial::operator = (const ZqPolynomial& P) {
	if(this != &P) {														    // Guarding against self assignment
	    const NTRU_N N = NTRUparameters.get_N();
		if(this->coefficients == NULL) this->coefficients = new int64_t[N];
		for(int i = 0; i < N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

ZqPolynomial& ZqPolynomial::operator = (const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
	if(this->coefficients == NULL) this->coefficients = new int64_t[N];       // In case of having an object created with the default (private) constructor
	for(int i = 0; i < N; i++) {
	    if(P[i] == 2) this->coefficients[i] = -1;                       // Since (1+2) mod 3 == 0, 2 must be sent to the additive inverse of 1 in Zq;
	    else          this->coefficients[i] = (int64_t)P[i];
	}
	return *this;
}

int64_t ZqPolynomial::operator [] (int i) const{
    const NTRU_N N = NTRUparameters.get_N();
	if(i < 0) i = -i;
	if(i > N) i %= N;
	return this->coefficients[i];
}

ZqPolynomial ZqPolynomial::operator + (const ZqPolynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
    ZqPolynomial r;                                                             // -Initializing with the zero polynomial
    for(int i = 0; i < N; i++)
        r.coefficients[i] = zq.add(this->coefficients[i],P.coefficients[i]);    // -Addition element by element till the smallest degree of the arguments
    return r;
}

ZqPolynomial ZqPolynomial::operator - (const ZqPolynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
    ZqPolynomial r;
    for(int i = 0; i < N; i++)
        r.coefficients[i] = zq.subtract(this->coefficients[i],P.coefficients[i]); // Subtraction element by element till the smallest degree of the arguments
    return r;
}

ZqPolynomial ZqPolynomial::operator * (const ZqPolynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
    ZqPolynomial r;
    int i, j, k;

	for(i = 0; i < N; i++) {
		if(this->coefficients[i] != 0) {                                        // Taking advantage this polynomials have a big proportion of zeros
		    k = N - i;
		    for(j = 0; j < k; j++)                                              // Ensuring we do not get out of the polynomial
			    if(P.coefficients[j] != 0)                                      // Expecting a big proportion of zeros
			        r.coefficients[i+j] += this->coefficients[i] * P.coefficients[j];
		    for(k = 0; k < i; j++, k++)                                         // Using the definition of convolution polynomial ring
			    if(P.coefficients[j] != 0)                                      // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
			        r.coefficients[k] += this->coefficients[i] * P.coefficients[j];
		}
	}
	r.mod_q();                                                                  // Applying mod q
	return r;
}

ZqPolynomial ZqPolynomial::operator * (const ZpPolynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
    ZqPolynomial r;
    int i, j, k;

	for(i = 0; i < N; i++) {
		if(P[i] != ZpPolynomial::_0_) {                                         // -Taking advantage this polynomials have a big proportion of zeros
		    if(P[i] == ZpPolynomial::_1_) {                                     // -The other two cases are one, as in this line is showed
		        k = N - i;
		        for(j = 0; j < k; j++)
		            r.coefficients[i+j] += this->coefficients[j];
		        for(k = 0; k < i; j++, k++)
		            r.coefficients[k] += this->coefficients[j];
		    } else {                                                            // -The only other case is two, which is interpreted as -1
		        k = N - i;
		        for(j = 0; j < k; j++)
		            r.coefficients[i+j] -= this->coefficients[j];
		        for(k = 0; k < i; j++, k++)
		            r.coefficients[k] -= this->coefficients[j];
		    }
		}
	}
	r.mods_q();                                                                 // Applying mods q
	return r;
}

ZqPolynomial NTRU::operator - (int64_t t, const ZqPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
    ZqPolynomial r;
    r.coefficients[0] = zq.subtract(t, P.coefficients[0]);
    for(int i = 1; i < N; i++) r.coefficients[i] = zq.mod_q(-P.coefficients[i]);
    return r;
}

bool ZqPolynomial::operator == (const Z2Polynomial& P) const {
    const NTRU_N N = NTRUparameters.get_N();
    for(int i = 0; i < N; i++) if(this->coefficients[i] != P[i]) return false;
    return true;
}

ZpPolynomial NTRU::mods_p(ZqPolynomial P) {
    const NTRU_N N = NTRUparameters.get_N();
    ZpPolynomial r;
    for(int i = 0, buff = 0; i < N; i++) {
        buff = P[i] % 3;
        if(buff == -2 || buff == 1) r.coefficients[i] = ZpPolynomial::_1_;
        if(buff == -1 || buff == 2) r.coefficients[i] = ZpPolynomial::_2_;
    }
    return r;
}

int ZqPolynomial::degree() const{													// -Returns degree of polynomial
	int deg = NTRUparameters.get_N();
	while(this->coefficients[--deg] == 0 && deg > 0) {}
	return deg;
}

void ZqPolynomial::mod_q() const{
    const NTRU_N N = NTRUparameters.get_N();
    for(int i = 0; i < N; i++) this->coefficients[i] = zq.mod_q(this->coefficients[i]);
}

void ZqPolynomial::mods_q() const{
    const NTRU_N N = NTRUparameters.get_N();
    for(int i = 0; i < N; i++) this->coefficients[i] = zq.mods_q(this->coefficients[i]);
}

ZqPolynomial ZqPolynomial::getNTRUpublicKey() {                                 // -Returns public key provided this ZqPolynomial object is the inverse mod q in
    const NTRU_N N = NTRUparameters.get_N();                                    //  Z[x]/X^N-1 of the private key
    ZqPolynomial publicKey;
    int* randTernary = new int[N];
    const int d_ = N/3;
    int _d_ = d_;
    int i, j, k;

    for(i = 0; i < N; i++) randTernary[i] = 0;

    while(_d_ > 0) {                                                            // -Building ternary polynomial multiplied by p
        i = randomIntegersN();                                                               // -Filling with p
        if(randTernary[i] == 0) {randTernary[i] =  1; _d_--;}                   // ...
    }
    _d_ = d_;
    while(_d_ > 0) {
        i = randomIntegersN();                                                               // -Filling with negative p
        if(randTernary[i] == 0) {randTernary[i] = -1; _d_--;}                   // ...
    }
	for(i = 0; i < N; i++) {                                                    // -Convolution process
	    k = N - i;
	    if(randTernary[i] != 0) {
	        if(randTernary[i] == 1) {
	            for(j = 0; j < k; j++)                                          // Ensuring we do not get out of the polynomial
                    publicKey.coefficients[i+j] += this->coefficients[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        publicKey.coefficients[k] += this->coefficients[j];         // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
	        }
	        if(randTernary[i] == -1) {
	            for(j = 0; j < k; j++)                                          // Ensuring we do not get out of the polynomial
                    publicKey.coefficients[i+j] -= this->coefficients[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        publicKey.coefficients[k] -= this->coefficients[j];         // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
	        }
	    }
	}
	delete[] randTernary;
	publicKey.mods_q();                                                         // Applying mods q
	return publicKey;
}

void ZqPolynomial::toBytes(char* dest) const{                                   // Supposing dest is pointing to a suitable memory location
    int i = 0, j = 0, k = 0, l = 0, log2q = 0;                                  // log2q will hold the logarithm base two of q. Here we are assuming q < 2^32
    int const q = zq.get_q();
    longlong_to_char ll_c = {0};                                                // ll_c will do the cast from int to char[]
    int _64 = 64;
    long long buff;
    const NTRU_N N = NTRUparameters.get_N();

    for(buff = q; buff > 1; buff >>= 1, log2q++) {}                           // Computing log2q using the fact q is a power of two

    std::cout << "\nlog2q = " << log2q << '\n';                                 // -Debugging purposes

    for(k = 0, buff = 0; i < N;) {
        ll_c._longlong_ >>= (l<<3);                                             // l*8; Ruling out the bits allocated in the last cycle
        while(k < _64 - log2q) {
            buff = this->coefficients[i++];
            if(buff <= 0) buff += q;
            ll_c._longlong_ |= buff << k; k += log2q;                           // Allocating log2q bits in ll_c._int_; increasing k
            if(i >= N) break;
        }
        for(l = 0; k >= 8; l++, k -= 8) dest[j++] = ll_c._char[l];              // Writing the last two bytes on destination
    }
    if(k > 0) dest[j++] = ll_c._char[l];
}

void ZqPolynomial::print(const char* name,const char* tail) const{
    unsigned len_q = len(zq.get_q());
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from int64_t array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printArray(array, (unsigned)coeffAmount, len_q+1, name, tail);
    delete[] array;
}

void ZqPolynomial::println(const char* name) const{
    this->print(name, "\n");
}

void ZqPolynomial::save(const char* name) const{
    const NTRU_N N = NTRUparameters.get_N();
    const int _q_ = zq.get_q();
    int byteArrSize, aux, log2q;
    char* byteArr = NULL;
    const char ntruq[] = "NTRUq";                                               // -This will indicate the binary file is saving a Zq NTRU (NTRU) polynomial
    for(aux = _q_, log2q = 0; aux > 1; aux >>= 1, log2q++) {}                   //  Computing log2q using the fact q is a power of two
    byteArrSize = N*log2q/8 + 1;                                                // -For values 8 < log2q < 16, this expression is valid
    std::ofstream file;                                                         //  coefficients in Zp (p)
    if(name == NULL) file.open("ZpPolynomial.ntrup");
    else             file.open(name);
    if(file.is_open()) {
        file.write(ntruq, 5);
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        byteArr = new char[byteArrSize];
        this->toBytes(byteArr);
        file.write(byteArr, byteArrSize);
    }
    if(byteArr != NULL) delete[] byteArr;
}

//_______________________________________________________________________ ZqPolynomial ____________________________________________________________________________

// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Encryption |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

Encryption::Encryption() {
	try {
	    this->setKeys();
	}catch(const char* exp) {
	    std::cout <<
	    "\nIn file NTRUencryption.cpp, function "
	    "Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_): N(_N_),q(_q_), p(_p_), d(_d_), "
        "privateKey(_N_, _p_), privateKeyInv_p(_N_, _p_), publicKey(_N_, _q_)\n";
        std::cout << exp;
	}
}

Encryption::Encryption(NTRU_N N, NTRU_q q) {
    this->setNTRUparameters(N, q);
	try {
	    this->setKeys();
	}catch(const char* exp) {
	    std::cout <<
	    "\nIn file NTRUencryption.cpp, function "
	    "Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_): N(_N_),q(_q_), p(_p_), d(_d_), "
        "privateKey(_N_, _p_), privateKeyInv_p(_N_, _p_), publicKey(_N_, _q_)\n";
        std::cout << exp;
	}
}

ZqPolynomial Encryption::encrypt(const ZpPolynomial& msg) {
    ZqPolynomial encryptedMsg = msg.encrypt(this->publicKey);
    return encryptedMsg;
}

ZqPolynomial Encryption::encrypt(const char fstring[]) {
    int fstringLen = -1;
    while(fstring[++fstringLen] != 0) {}
    ZpPolynomial msg(fstring, fstringLen);
    ZqPolynomial encryptedMsg = msg.encrypt(this->publicKey);
    return encryptedMsg;
}

ZpPolynomial Encryption::decrypt(const ZqPolynomial& e_msg) {
    ZpPolynomial msg = mods_p(e_msg*this->privateKey);
    msg = msg*this->privateKeyInv_p;
    return msg;
}

NTRU_N Encryption::get_N() const{ return NTRUparameters.get_N(); }
NTRU_q Encryption::get_q() const{ return zq.get_q(); }

void Encryption::setNTRUparameters(NTRU_N N, NTRU_q q) const{                   // -Sets the value of NTRUencryption algorithm
    NTRUparameters.N = N;
    zq.q = q;
}

void Encryption::savePrivateKey_txt() {
    char buff[301];
    std::ofstream file;
    buff[300] = 0;

    this->privateKey.toBytes(buff);
    file.open("PrivateKey.txt");
    if(file.is_open()) {
        file.write(buff, 301);
        file.close();
    } else {
        throw "File could not be written.";
    }
}

void Encryption::setKeys() {
    ZpPolynomial  Zp_privateKeyInv;
	ZpPolynomial  Zp_gcdXNmns1;
	ZqPolynomial  Zq_privateKey;
	ZqPolynomial  Zq_privateKeyInv;
	ZqPolynomial::Z2Polynomial Z2_privateKeyInv;
	ZqPolynomial::Z2Polynomial Z2_gcdXNmns1;
	ZpPolynomial  Zp_privateKey = ZpPolynomial::getPosiblePrivateKey();         // -Initializing private key with random ternary polynomial
	ZqPolynomial::Z2Polynomial Z2_privateKey(Zp_privateKey);
	int counter, k = 2, l = 1, q = zq.get_q();

	try{
	    Zp_gcdXNmns1 = Zp_privateKey.gcdXNmns1(Zp_privateKeyInv);
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption::setKeys()\n";
        throw;
	}
	try{
	    Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv);
	}catch(const char* exp) {
        std::cout<<"\nIn file Encryption.cpp, function void Encryption::setKeys()\n";
        throw;
	}

	counter = 1;
    while(Zp_gcdXNmns1 != 1 || Z2_gcdXNmns1 != 1) {
        if((counter & 3) != 0) {                                                // If we have not tried to much times, just permute the coefficients
            Zp_privateKey.permute();                                            // counter & 3 == counter % 4
	        Z2_privateKey = Zp_privateKey;                                      // ...
        } else {
            Zp_privateKey.changeAzeroForAone();                                 // To much tries, increasing the numbers of non-zero coefficients
            Z2_privateKey = Zp_privateKey;                                      // ...
	    }
	    try{ Zp_gcdXNmns1=Zp_privateKey.gcdXNmns1(Zp_privateKeyInv); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRUencryption::setKeys()\n";
            throw;
	    }
	    try{ Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv); }
	    catch(const char* exp) {
            std::cout<<"\nIn file Encryption.cpp, function void NTRUencryption::setKeys()\n";
            throw;
    	}
	    counter++;
	}

    Zq_privateKey    = Zp_privateKey;
	Zq_privateKeyInv = ZqPolynomial(Z2_privateKeyInv);

	while(k < q) {
	    Zq_privateKeyInv = Zq_privateKeyInv*(2 - Zq_privateKey*Zq_privateKeyInv);
        k <<= l; l <<= 1;
	}                                                                           // -At this line, we have just created the private key and its inverse
	if(Zq_privateKeyInv*Zp_privateKey == 1) {
	    if(Zp_privateKey*Zp_privateKeyInv == 1) {
	        if(counter > 1)
	            std::cout << "Private key was found after " <<counter<< " attempts.\n";
	        else
	            std::cout << "Private key was found after "<< counter << " attempt.\n";
	    } else {
	        (Zp_privateKey*Zp_privateKeyInv).println("this->privateKey*this->privateKeyInv_p");
	        throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zp[x]/(x^N-1) ring not found.\n";
	    }
	} else {
	    (Zp_privateKey*Zp_privateKeyInv).println("Zq_privateKeyInv*this->privateKey");
	    throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zq[x]/(x^N-1) ring not found\n";
	}
	this->privateKey      = Zp_privateKey;
	this->privateKeyInv_p = Zp_privateKeyInv;
	this->publicKey       = Zq_privateKeyInv.getNTRUpublicKey();
}
//________________________________________________________________________ Encryption _____________________________________________________________________________
