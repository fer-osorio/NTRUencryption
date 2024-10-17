#include<fstream>
#include<random>
#include<chrono>
#include<ctime>
#include"NTRUencryption.hpp"

#define DECIMAL_BASE 10

union int64_to_char {                                                        // Allows to cast from an int64_t to an array of four bytes (char)
    int64_t int64;
    char    chars[8];
};

inline static int max(int a, int b) {
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
static int lengthString(const char* str) {										        // Length of a string
	int l = -1;
	while(str[++l] != 0) {}
	return l;
}
static unsigned lengthDecimalInt(int a) {                                       // -Returns the number of decimal characters for the number a
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

    startLen = lengthString(name);
    if(startLen > 0) {
        std::cout << name << " = [ ";
        startLen += 4;
    }
    else {
        std::cout <<  "0   [";
        startLen = 5;
    }
    do {
        if(i != 0 && (i & 15) == 0) {                                           // Since 2^4 = 16, then i&15 = i % 16
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

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| NTRU Parameters |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

static void setNTRUparameters(NTRU_N, NTRU_q);

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
		friend void setNTRUparameters(NTRU_N, NTRU_q);
	};
	friend void setNTRUparameters(NTRU_N, NTRU_q);
};

static NTRU_Parameters NTRUparameters;                                          // -This unique instance puts every polynomial "inside the same polynomial ring"
static NTRU_Parameters::Zq zq(_8192_);                                          // -Arithmetic for the operation modulo q

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

void setNTRUparameters(NTRU_N N, NTRU_q q) {
    NTRUparameters.N = N;
    zq.q = q;
    randomIntegersN = RandInt(0,N - 1);
}

static NTRU_N N_backup = NTRUparameters.get_N();							    // -In case of encrypting or decrypting using keys from an external file, this
static NTRU_q q_backup = zq.get_q();                                            //  files will save the original internal values

// ____________________________________________________________________ NTRU Parameters ___________________________________________________________________________

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

void ZpPolynomial::changeAzeroForAtwo() {
    int i = randomIntegersN();
    while(this->coefficients[i] != _0_) i = randomIntegersN();                  // -Looking for a zero in a random position
    this->coefficients[i] = _2_;                                                // -Changing zero for one
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
		if(this->coefficients[i] != _0_) {
		    k = N - i;
		    if(this->coefficients[i] == _1_) {
		        for(j = 0; j < k; j++)                                          // Adding and multiplying while the inequality i+j < N holds
			        if(P.coefficients[j] != _0_)                                // We expect a big proportion of zeros
			            r.coefficients[i+j] += P.coefficients[j];
		        for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial rings
			        if(P.coefficients[j] != _0_)                                // Notice i+k is congruent with j mod N
			            r.coefficients[k] += P.coefficients[j];
			} else {
			    for(j = 0; j < k; j++)                                          // Subtracting and multiplying while the inequality i+j < N holds
			        if(P.coefficients[j] != _0_)                                // We expect a big proportion of zeros
			            r.coefficients[i+j] -= P.coefficients[j];
		        for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial rings
			        if(P.coefficients[j] != _0_)                                // Notice i+k is congruent with j mod N
			            r.coefficients[k] -= P.coefficients[j];
			}
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
    if(P == _0_) throw "\nvoid ZpPolynomial::ZpPolynomial::division(const ZpPolynomial& P,ZpPolynomial result[2]) const. Division by zero...\n";
    if(*this == 0) {                                                            // -Case zero divided by anything
        result[0] = _0_;                                                        // -Zero polynomial
        result[1] = _0_;                                                        // -Zero polynomial
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

	while(remainders != _0_) {                                                  // EEA implementation (continuation)
        try{ gcd.division(remainders, quoRem); }
        catch(const char* exp) {
            std::cerr << "\nIn NTRUencryption.cpp; function ZpPolynomial ZpPolynomial::PolyModXNmns1::gcdXNmns1(PolyModXNmns1& thisBezout)\n";
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

ZqPolynomial ZpPolynomial::encrypt(ZqPolynomial publicKey, bool showEncryptionTime) const{
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    if(showEncryptionTime) begin = std::chrono::steady_clock::now();
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
        i = randomIntegersN();                                                  // -Filling with threes. It represent the random polynomial multiplied by p
        if(randTernaryTimes_p[i] == 0) {randTernaryTimes_p[i] =  3; _d_--;}     //  ...
    }
    _d_ = d;
    while(_d_ > 0) {
        i = randomIntegersN();                                                  // -Filling with negative threes
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
	if(showEncryptionTime) {
	    end = std::chrono::steady_clock::now();
        std::cout << "\nMessage was encrypted in " << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "[µs]\n" << std::endl;
    }
	return encryption;
}

void ZpPolynomial::toBytes(char dest[]) const{
    NTRU_N N = NTRUparameters.get_N();
    int i,j,k;
    int N_mod_5 = N%5;                                                          // -Since N is a prime number, N_mod_5 is always bigger than 0
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
    NTRU_q q = zq.get_q();
    int byteArrSize = N / 5 + 1;                                                // -this->N is necessarily prime, so this->N % 5 > 0 holds true
    char* byteArr = NULL;
    const char ntrup[] = "NTRUp";                                               // -This will indicate the binary file is saving a NTRU (NTRU) polynomial with
    std::ofstream file;                                                         //  coefficients in Zp (p)
    if(name == NULL) file.open("ZpPolynomial.ntrup");
    else             file.open(name);
    if(file.is_open()) {
        file.write(ntrup, 5);                                                   // -The first five bytes are for the letters 'N' 'T' 'R' 'U' 'p'
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the degree of the polynomial
        byteArr = new char[byteArrSize];                                        // -The following bytes are for the polynomials coefficients
        this->toBytes(byteArr);
        file.write(byteArr, byteArrSize);
    }
    if(byteArr != NULL) delete[] byteArr;
}

//_______________________________________________________________________ ZpPolynomial ___________________________________________________________________________

//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Z2Polynomial ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

Z2Polynomial::Z2Polynomial() {
    const NTRU_N N = NTRUparameters.get_N();
	this->coefficients = new Z2[N];
	for(int i = 0; i < N; i++) this->coefficients[i] = _0_;
}

Z2Polynomial::Z2Polynomial(const Z2Polynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
    this->coefficients = new Z2[N];
    for(int i = 0; i < N; i++) this->coefficients[i] = P.coefficients[i];
}

Z2Polynomial::Z2Polynomial(const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
	this->coefficients = new Z2[N];
	for(int i = 0; i < N; i++) {
	    if(P[i] != 0) this->coefficients[i] = _1_;                              // Two won't go to zero because it's the additive inverse of one in Z3, therefore
	    else this->coefficients[i] = _0_;                                       // it must go to the additive inverse of one in Z2, which if itself
    }
}

Z2Polynomial& Z2Polynomial::operator = (const Z2Polynomial& P)  {
	if(this != &P) {													        // Guarding against self assignment
	    const NTRU_N N = NTRUparameters.get_N();
		if(this->coefficients == NULL) this->coefficients = new Z2[N];
		for(int i = 0; i < N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

Z2Polynomial& Z2Polynomial::operator = (const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
	if(this->coefficients == NULL) this->coefficients = new Z2[N];              // In case of having an object created with the default (private) constructor
	for(int i = 0; i < N; i++) {                                                // Fitting the ZpPolynomial in a Z2Polynomial
	    if(P[i] != 0) this->coefficients[i] = _1_;
	    else this->coefficients[i] = _0_;
	}
	return *this;
}

Z2Polynomial& Z2Polynomial::operator = (Z2 t)  {
    const NTRU_N N = NTRUparameters.get_N();
	if(this->coefficients == NULL) this->coefficients = new Z2[N];              // In case of have been generated from the (private) default constructor
	this->coefficients[0] = t;
	for(int i = 1; i < N; i++) this->coefficients[i] = _0_;
	return *this;
}

Z2Polynomial Z2Polynomial::operator + (const Z2Polynomial& P) const {
    const NTRU_N N = NTRUparameters.get_N();
    Z2Polynomial r;                                                             // -Initializing result with zeros
    for(int i = 0; i < N; i++)
        r.coefficients[i] = this->coefficients[i] + P.coefficients[i];          // -Addition element by element till the smallest degree of the arguments
    return r;
}

Z2Polynomial Z2Polynomial::operator - (const Z2Polynomial& P) const{
    return *this + P;                                                           // In this polynomial ring, subtraction coincide with addition
}

Z2Polynomial Z2Polynomial::operator * (const Z2Polynomial& P) const{ // Classical polynomial multiplication algorithm
    const NTRU_N N = NTRUparameters.get_N();
    Z2Polynomial r;                                                             // -Initializing with zeros
    int i, j, k, l;

	for(i = 0; i < N; i++) {
		if(this->coefficients[i] != _0_) {                                      // -Polynomial over binary field, here we know  this->coefficients[i] is 1
		    k = N - i;
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

void Z2Polynomial::division(const Z2Polynomial& P, Z2Polynomial result[2]) const{
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

Z2Polynomial Z2Polynomial::gcdXNmns1(Z2Polynomial& thisBezout) const{
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
    for(i = N-1 - deg, j = N-1; j >= deg; i = j - deg) {                        // Continuing with division algorithm; i is the place of the next coefficient of
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
            std::cerr << "\nNTRU_Z2Polynomial ZqPolynomial::Z2Polynomial::gcdXNmns1(Z2Polynomial& thisBezout) const\n";
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

bool Z2Polynomial::operator == (const Z2Polynomial& P) const{
    const NTRU_N N = NTRUparameters.get_N();
	for(int i = 0; i < N; i++) if(this->coefficients[i] != P.coefficients[i])  return false;
	return true;
}

Z2Polynomial::Z2 Z2Polynomial::operator[](int i) const{
    const NTRU_N N = NTRUparameters.get_N();
	if(i < 0) i = -i;
	if(i >= N) i %= N;
	return this->coefficients[i];
}

int Z2Polynomial::degree() const{												// Returns degree of polynomial
	int deg = NTRUparameters.get_N();
	while(this->coefficients[--deg] == _0_ && deg > 0) {}
	return deg;
}

void Z2Polynomial::print(const char* name,const char* tail) const{
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from Z2 array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printArray(array, (unsigned)coeffAmount, 2, name, tail);
    delete[] array;
}

void Z2Polynomial::println(const char* name) const{
    this->print(name, "\n");
}

//________________________________________________________________________Z2Polynomial_____________________________________________________________________________

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ZqPolynomial ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

ZqPolynomial::ZqPolynomial() {
    const NTRU_N N = NTRUparameters.get_N();
    this->coefficients = new int64_t[N];
    for(int i = 0; i < N; i++) this->coefficients[i] = 0;
}

ZqPolynomial::ZqPolynomial(const ZpPolynomial& P) {
    const NTRU_N N = NTRUparameters.get_N();
	this->coefficients = new int64_t[N];
	for(int i = 0; i < N; i++) {
	    if(P[i] == 2) this->coefficients[i] = -1;                               // Since (1+2) mod 3 == 0, 2 must be sent to the additive inverse of 1 in Zq;
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

ZqPolynomial::ZqPolynomial(const char data[], int dataLength) {
    NTRU_N  N = NTRUparameters.get_N();
    const int log2q = log2(zq.get_q());
    const int q = zq.get_q();
    const int64_t q_1    = q - 1;                                               // -Essentially, the first log2q bits are 1's
    const int64_t qdiv2  = q >> 1;
    const int64_t negq_1 = ~q_1;
    int bitsOcupiedInBuff = 0;
    int i , j, dataByteIndex;
    int offset = 0;
    int64_to_char aux;
    uint64_t buff = 0;

    this->coefficients = new int64_t[N];

    for(i = 0, dataByteIndex = 0; dataByteIndex < dataLength && i < N ;) {
        aux.int64 = 0;
        for(j = 0; bitsOcupiedInBuff <= 56 && dataByteIndex < dataLength; bitsOcupiedInBuff += 8) {// -If we are using at most 56 bits of the buffer, we can
            aux.chars[j++] = data[dataByteIndex++];                          //  still allocate one more byte
        }
        buff |= (uint64_t)aux.int64 << offset;
        for(; bitsOcupiedInBuff >= log2q; bitsOcupiedInBuff -= log2q, i++) {
            this->coefficients[i] = (int64_t)buff & q_1;                        // -Taking the first log2q bits of buff. The result will be positive
            if(this->coefficients[i] >= qdiv2) this->coefficients[i] |= negq_1; // This is equivalent to r - this->q when r < q
            buff >>= log2q;
        }
        offset = bitsOcupiedInBuff;
    }
    if(bitsOcupiedInBuff > 0) {
        if(i < N) {
            this->coefficients[i] = (int64_t)buff & q_1;                        // -Taking the first log2q bits of buff. The result will be positive
            if(this->coefficients[i] >= qdiv2) this->coefficients[i] |= negq_1; // This is equivalent to r - this->q when r < q
            buff >>= log2q;
            i++;
        }
    }
    for(;i < N; i++) this->coefficients[i] = 0;                                 // -Padding the rest of the polynomial with zeros
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
	if(this->coefficients == NULL) this->coefficients = new int64_t[N];         // In case of having an object created with the default (private) constructor
	for(int i = 0; i < N; i++) {
	    if(P[i] == 2) this->coefficients[i] = -1;                               // Since (1+2) mod 3 == 0, 2 must be sent to the additive inverse of 1 in Zq;
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
		//if(this->coefficients[i] != 0) {                                      // Taking advantage this polynomials have a big proportion of zeros
		    k = N - i;
		    for(j = 0; j < k; j++)                                              // Ensuring we do not get out of the polynomial
			    //if(P.coefficients[j] != 0)                                      // Expecting a big proportion of zeros
			        r.coefficients[i+j] += this->coefficients[i] * P.coefficients[j];
		    for(k = 0; k < i; j++, k++)                                         // Using the definition of convolution polynomial ring
			    //if(P.coefficients[j] != 0)                                    // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
			        r.coefficients[k] += this->coefficients[i] * P.coefficients[j];
		//}
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
    for(int i = 1; i < N; i++) r.coefficients[i] = -P.coefficients[i];          //zq.mod_q(-P.coefficients[i]);
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

ZqPolynomial NTRU::convolutionZq(const Z2Polynomial& z2P, const ZpPolynomial& zpP) {
    const NTRU_N N = NTRUparameters.get_N();
    ZqPolynomial r;
    int i, j, k;

    for(i = 0; i < N; i++) {
        if(z2P.coefficients[i] != Z2Polynomial::Z2::_0_) {
            k = N - i;
		    for(j = 0; j < k; j++)
		        if(zpP.coefficients[j] != 0) {
		            if(zpP.coefficients[j] == 1) ++r.coefficients[i+j];
		            else --r.coefficients[i+j];
		        }
		    for(k = 0; k < i; j++, k++)
		        if(zpP.coefficients[j] != 0) {
		            if(zpP.coefficients[j] == 1) ++r.coefficients[k];
		            else --r.coefficients[k];
		        };
        }
    }
    r.mods_q();                                                                 // Applying mods q
    return r;
}

ZqPolynomial NTRU::convolutionZq(const Z2Polynomial& z2P, const ZqPolynomial& zqP) {
    const NTRU_N N = NTRUparameters.get_N();
    ZqPolynomial r;
    int i, j, k;

    for(i = 0; i < N; i++) {
        if(z2P.coefficients[i] != Z2Polynomial::Z2::_0_) {
            k = N - i;
		    for(j = 0; j < k; j++)
		        r.coefficients[i+j] += zqP.coefficients[j];
		    for(k = 0; k < i; j++, k++)
		        r.coefficients[k] += zqP.coefficients[j];
        }
    }
    r.mods_q();                                                                 // Applying mods q
    return r;
}

int ZqPolynomial::degree() const{											    // -Returns degree of polynomial
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

int ZqPolynomial::log2(NTRU_q q) {                                                            // -Returns logarithm base 2 of a NTRU_q value
    int log2q = 0, qq = q;
    for(; qq > 1; qq >>= 1, log2q++) {}
    return log2q;
}

int ZqPolynomial::lengthInBytes() const{
    return NTRUparameters.get_N()*log2(zq.get_q())/8 + 1;
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
        i = randomIntegersN();                                                  // -Filling with p
        if(randTernary[i] == 0) {randTernary[i] =  1; _d_--;}                   // ...
    }
    _d_ = d_;
    while(_d_ > 0) {
        i = randomIntegersN();                                                  // -Filling with negative p
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
	        } else {
	            for(j = 0; j < k; j++)                                          // Ensuring we do not get out of the polynomial
                    publicKey.coefficients[i+j] -= this->coefficients[j];
	            for(k = 0; k < i; j++, k++)                                     // Using the definition of convolution polynomial ring
	    	        publicKey.coefficients[k] -= this->coefficients[j];         // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
	        }
	    }
	}
	delete[] randTernary;
	publicKey.mods_q();                                                         // -Applying mods q
	return publicKey;
}

void ZqPolynomial::toBytes(char dest[]) const{                                  // -Supposing dest is pointing to a suitable memory location
    const NTRU_N N = NTRUparameters.get_N();
    const int buffBitsSize = 64;
    const int q = zq.get_q();
    int i = 0, j = 0, log2q = log2(zq.get_q());                                 // -log2q will hold the logarithm base two of q. Here we are assuming q < 2^32)
    int bitsAllocInBuff = 0;                                                    // -Amount of bits allocated (copied from coefficients array) in buffer
    int bytesAllocInDest = 0;
    int64_to_char buffer = {0};                                                 // -buffer will do the cast from int to char[]
    int64_t aux;

    //std::cout << "\nlog2q = " << log2q << '\n';                                 // -Debugging purposes

    for(bitsAllocInBuff = 0, aux = 0; i < N;) {
        buffer.int64 >>= (bytesAllocInDest << 3);                               // -l*8; Ruling out the bits allocated in the last cycle
        while(bitsAllocInBuff < buffBitsSize - log2q) {                         // -Allocating bits from coefficients to buffer
            aux = this->coefficients[i++];
            if(aux < 0) aux += q;
            buffer.int64 |= aux << bitsAllocInBuff;                             // -Allocating log2q bits in buffer._int_;
            bitsAllocInBuff += log2q;                                           // -increasing amount of bits allocated in buffer
            if(i >= N) break;
        }
        for(bytesAllocInDest = 0; bitsAllocInBuff >= 8; bytesAllocInDest++, bitsAllocInBuff -= 8)
            dest[j++] = buffer.chars[bytesAllocInDest];                         // -Writing buffer in destination (as long there are at least 8 bits in buffer)
    }
    if(bitsAllocInBuff > 0) dest[j++] = buffer.chars[bytesAllocInDest];         // -The bits that remain in buffer, we allocate them in the last byte
}

void ZqPolynomial::print(const char* name,const char* tail) const{
    unsigned len_q = lengthDecimalInt(zq.get_q());
    int coeffAmount = this->degree() + 1;                                       // -This three lines is a "casting" from int64_t array to int array
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
    const short q = zq.get_q();
    int log2q = log2(zq.get_q());
    int byteArrSize = N*log2q/8 + 1;                                            // -For values 8 < log2q < 16, this expression is valid
    char* byteArr = NULL;
    const char ntruq[] = "NTRUq";                                               // -This will indicate the binary file is saving a Zq NTRU (NTRU) polynomial

    std::ofstream file;                                                         //  coefficients in Zp (p)
    if(name == NULL) file.open("ZqPolynomial.ntrup");
    else             file.open(name);
    if(file.is_open()) {
        file.write(ntruq, 5);
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the q value
        byteArr = new char[byteArrSize];
        this->toBytes(byteArr);
        file.write(byteArr, byteArrSize);
    }
    if(byteArr != NULL) delete[] byteArr;
}

//_______________________________________________________________________ ZqPolynomial ____________________________________________________________________________

// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Encryption |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

/*Encryption::Encryption() {
	try {
	    this->setKeys(true);
	}catch(const char* exp) {
	    std::cerr << "\nIn file NTRUencryption.cpp, function Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_): N(_N_),q(_q_), p(_p_), d(_d_), "
        "privateKey(_N_, _p_), privateKeyInv_p(_N_, _p_), publicKey(_N_, _q_)\n";
        std::cerr << exp;
	}
}*/

Encryption::Encryption(NTRU_N n, NTRU_q Q): N(n), q(Q) {
    setNTRUparameters(this->N, this->q);
    this->validPrivateKey = true;
	try {
	    this->setKeys(true);
	}catch(const char* exp) {
	    std::cerr << "\nIn file NTRUencryption.cpp, function Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_): N(_N_),q(_q_), p(_p_), d(_d_), "
        "privateKey(_N_, _p_), privateKeyInv_p(_N_, _p_), publicKey(_N_, _q_)\n";
        std::cerr << exp;
        this->validPrivateKey = false;
	}
}

Encryption::Encryption(const char* NTRUkeyFile): N(_1499_), q(_8192_) {
    const char NTRUpublicKey[] = "NTRUpublicKey";                               // -This will indicate the binary file is saving a NTRU public key
    const char NTRUprivatKey[] = "NTRUprivatKey";                               // -This will indicate the binary file is saving a NTRU private key
    char* coeffBytes = NULL;
    char* fileHeader = NULL;
    int   sz = 0, headerSz = lengthString("NTRUpublicKey");                     // -Notice how "NTRUpublicKey" and "NTRUprivatKey" strings have the same length
    bool  isPrivateKey = false;
    std::ifstream file;
    file.open(NTRUkeyFile, std::ios::binary);
    if(file.is_open()) {
        fileHeader = new char[headerSz + 1];
        file.read(fileHeader, headerSz);                                        // -Reading the firsts bytes hoping "NTRUpublicKey" or "NTRUprivatKey" apears
        fileHeader[headerSz] = 0;                                               // -End of string
        if(compareStrings(fileHeader, NTRUprivatKey) || compareStrings(fileHeader, NTRUpublicKey)) { // -Testing if file saves a NTRU private key
            file.read((char*)&this->N, 2);
            file.read((char*)&this->q, 2);
            //std::cout << "N = " << this->N << ", q = " << this->q << "\n";    // -Debugging purposes
            setNTRUparameters(this->N, this->q);
            if(compareStrings(fileHeader, NTRUprivatKey)) {
                isPrivateKey = true;
                sz = this->privateKeySizeInBytes();
                coeffBytes = new char[sz];
                file.read(coeffBytes, sz);                                          // -Reading the coefficients of the polynomial
                this->privateKey = ZpPolynomial(coeffBytes, sz);
                try {
                    this->setKeysFromPrivKey();
                } catch(const char* str) {
                    delete[] coeffBytes; coeffBytes = NULL;
                    delete[] fileHeader; fileHeader = NULL;
                    std::cerr << "\nIn file NTRUencryption.cpp, function void Encryption::Encryption(const char* NTRUKeyFile)\n";
                    throw;
                }
            } else {
                isPrivateKey = false;
                sz = this->publicKeySizeInBytes();
                coeffBytes = new char[sz];
                file.read(coeffBytes, sz);                                      // -Reading the coefficients of the polynomial
                //std::cout << "publicKeySizeInBytes = " << sz << '\n';         // -Debugging purposes
                this->publicKey = ZqPolynomial(coeffBytes, sz);
            }
        } else {
            delete[] fileHeader; fileHeader = NULL;
            throw "Not a valid ntruPrivateKey file.";
        }
    } else {
        throw "File could not be opened/created.";
    }
    if(coeffBytes != NULL) delete[] coeffBytes;
    if(fileHeader != NULL) delete[] fileHeader;
    this->validPrivateKey = isPrivateKey;                                       // -Only encryption if we got just the public key
}

int Encryption::plainTextMaxSizeInBytes() const{ return this->N/5 + 1; }        // -Notice the fact maximum size for plain text and the size of the private key
int Encryption::cipherTextSizeInBytes()   const{ return this->N*ZqPolynomial::log2(this->q)/8 + 1; }//  (in bytes) is the same, and that is because they are
int Encryption::privateKeySizeInBytes()   const{ return this->N/5 + 1; }        //  both ZpPolynomials. The same is true for the cipher text and the public key
int Encryption::publicKeySizeInBytes()    const{ return this->N*ZqPolynomial::log2(this->q)/8 + 1; }

void Encryption::saveKeys(const char publicKeyName[], const char privateKeyName[]) const{
    if(this->N != NTRUparameters.get_N() || this->q != zq.get_q()) {
        setNTRUparameters(this->N, this->q);
    }
    NTRU_N N = NTRUparameters.get_N();
    NTRU_q q = zq.get_q();
    int publicKeySize  = this->publicKeySizeInBytes();
    int privateKeySize = this->privateKeySizeInBytes();
    char* publicKeyBytes  = NULL;
    char* privateKeyBytes = NULL;
    const char NTRUpublicKey[]  = "NTRUpublicKey";                              // -This will indicate the binary file is saving a NTRU public key
    const char NTRUprivateKey[] = "NTRUprivatKey";                              // -This will indicate the binary file is saving a NTRU private key
    std::ofstream file;
    if(publicKeyName == NULL) file.open("Encryption.ntruPublicKey", std::ios::binary);
    else                      file.open(publicKeyName, std::ios::binary);
    if(file.is_open()) {
        file.write((char*)NTRUpublicKey, lengthString(NTRUpublicKey));          // -Initiating the file with the string "NTRUpublicKey"
        file.write((char*)&N, 2);                                               // -A short int for the degree of the polynomial
        file.write((char*)&q, 2);                                               // -A short int for the degree of the polynomial
        publicKeyBytes = new char[publicKeySize];                               // -The following bytes are for the polynomials coefficients
        this->publicKey.toBytes(publicKeyBytes);
        file.write(publicKeyBytes, publicKeySize);
        delete[] publicKeyBytes;
        publicKeyBytes = NULL;
    }
    file.close();
    if(this->validPrivateKey) {                                                 // -If the object is in only encryption mode, private key will not be saved,
        if(privateKeyName == NULL) file.open("Encryption.ntruPrivateKey", std::ios::binary);      //  because there is no valid private key
        else                       file.open(privateKeyName, std::ios::binary);
        if(file.is_open()) {
            file.write((char*)NTRUprivateKey, lengthString(NTRUprivateKey));    // -Initiating the file with the string "NTRUprivateKey"
            file.write((char*)&N, 2);                                           // -A short int for the degree of the polynomial
            file.write((char*)&q, 2);                                           // -A short int for the degree of the polynomial
            privateKeyBytes = new char[privateKeySize];                         // -The following bytes are for the polynomials coefficients
            this->privateKey.toBytes(privateKeyBytes);
            file.write(privateKeyBytes, privateKeySize);
            delete[] privateKeyBytes;
            privateKeyBytes = NULL;
        }
    }
    if(publicKeyBytes  != NULL) delete[] publicKeyBytes;
    if(privateKeyBytes != NULL) delete[] privateKeyBytes;
}

// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Encryption keys |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

void Encryption::setKeys(bool showKeyCreationTime) {
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    if(showKeyCreationTime) begin = std::chrono::steady_clock::now();

    if(this->N != NTRUparameters.get_N() || this->q != zq.get_q()) {
        setNTRUparameters(this->N, this->q);
    }
	ZpPolynomial Zp_gcdXNmns1;
	Z2Polynomial Z2_privateKeyInv;
	Z2Polynomial Z2_gcdXNmns1;
	Z2Polynomial Z2_privateKey(this->privateKey);
	int counter, k = 2, l = 1;

	try{
        Zp_gcdXNmns1 = this->privateKey.gcdXNmns1(this->privateKeyInv_p);
    }catch(const char* exp) {
        std::cerr << "\nIn file NTRUencryption.cpp, function void Encryption::setKeys()\n";
        throw;
    }
    try{
        Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv);
    }catch(const char* exp) {
        std::cerr << "\nIn file NTRUencryption.cpp, function void Encryption::setKeys()\n";
        throw;
    }

	counter = 1;
    while(Zp_gcdXNmns1 != 1 || Z2_gcdXNmns1 != 1) {
        if((counter & 31) != 0) {                                               // If we have not tried to much times, just permute the coefficients
            this->privateKey.permute();                                         // counter & 3 == counter % 4
            Z2_privateKey = this->privateKey;                                   // ...
        } else {
            this->privateKey.changeAzeroForAone();                              // To much tries, increasing the numbers of non-zero coefficients
            std::cout << "Increasing number of non zero coefficients in posible private key. Counter = " << counter << "\n";
            Z2_privateKey = this->privateKey;                                   // ...
        }
        try{ Zp_gcdXNmns1 = this->privateKey.gcdXNmns1(this->privateKeyInv_p); }
        catch(const char* exp) {
            std::cerr << "\nIn file NTRUencryption.cpp, function void NTRUencryption::setKeys()\n";
            throw;
        }
        try{ Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv); }
        catch(const char* exp) {
            std::cerr << "\nIn file NTRUencryption.cpp, function void NTRUencryption::setKeys()\n";
            throw;
    	}
        counter++;
    }

    this->publicKey = convolutionZq(Z2_privateKeyInv, 2 - convolutionZq(Z2_privateKeyInv, this->privateKey));
    k <<= l; l <<= 1;

	while(k < this->q) {
        this->publicKey = this->publicKey*(2 - this->publicKey*this->privateKey);
        k <<= l; l <<= 1;
    }                                                                           // -At this line, we have just created the private key and its inverse

    /*if(this->publicKey*Zp_privateKey == 1) {
    if(Zp_privateKey*this->privateKeyInv_p == 1) {
        if(counter > 1)
            std::cout << "Private key was found after " <<counter<< " attempts.\n";
        else
                std::cout << "Private key was found after "<< counter << " attempt.\n";
        } else {
            (Zp_privateKey*this->privateKeyInv_p).println("this->privateKey*this->privateKeyInv_p");
            throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zp[x]/(x^N-1) ring not found.\n";
        }
    } else {
        (Zp_privateKey*this->privateKeyInv_p).println("this->publicKey*this->privateKey");
        throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zq[x]/(x^N-1) ring not found\n";
    }*/

	if(showKeyCreationTime) {
	    end = std::chrono::steady_clock::now();
        std::cout << "\nPrivate and public keys generation took "<< std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()<<"[µs]\n"<<std::endl;
        if(counter > 1) std::cout << "Private key was found after " <<counter<< " attempts.\n";
        else std::cout << "Private key was found after "<< counter << " attempt.\n";
    }
}

void Encryption::setKeysFromPrivKey() {                                         // -In this function we're supposing private key polynomial has inverse.
    if(this->N != NTRUparameters.get_N() || this->q != zq.get_q()) {
        setNTRUparameters(this->N, this->q);
    }
    Z2Polynomial Z2_privateKey(this->privateKey);
    Z2Polynomial Z2_privateKeyInv;
    Z2Polynomial Z2_gcdXNmns1;
    ZpPolynomial Zp_gcdXNmns1;
    int k = 2, l = 1;

    try{
        Zp_gcdXNmns1 = this->privateKey.gcdXNmns1(this->privateKeyInv_p);
    }catch(const char* exp) {
        std::cerr<<"\nIn file NTRUencryption.cpp, function void Encryption::setKeys()\n";
        throw;
	}
    if(this->privateKey*this->privateKeyInv_p != 1) {
        throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeysFromPrivKey(). Private key has no inverse in Zp[x]/(x^N-1).\n";
    }
    try{
        Z2_gcdXNmns1 = Z2_privateKey.gcdXNmns1(Z2_privateKeyInv);
    }catch(const char* exp) {
        std::cerr<<"\nIn file NTRUencryption.cpp, function void Encryption::setKeys()\n";
        throw;
    }
    this->publicKey = convolutionZq(Z2_privateKeyInv, 2 - convolutionZq(Z2_privateKeyInv, this->privateKey));
    k <<= l; l <<= 1;

	while(k < this->q) {
        this->publicKey = this->publicKey*(2 - this->publicKey*this->privateKey);
        k <<= l; l <<= 1;
    }                                                                           // -At this line, we have just created the private key and its inverse
    if(this->privateKeyInv_p*this->privateKey != 1) {
        (this->privateKeyInv_p*this->privateKey).println("Zq_privateKeyInv*this->privateKey");
        throw "\nIn file NTRUencryption.cpp, function void Encryption::setKeys(). Private key inverse in Zq[x]/(x^N-1) ring not found\n";
    }
}

// ____________________________________________________________________ Encryption keys ___________________________________________________________________________

ZqPolynomial Encryption::encrypt(const char bytes[], int size, bool showEncryptionTime) const{
    if(this->N != NTRUparameters.get_N() || this->q != zq.get_q()) {
        setNTRUparameters(this->N, this->q);
    }
    ZpPolynomial msg(bytes, size);
    ZqPolynomial encryptedMsg = msg.encrypt(this->publicKey, showEncryptionTime);
    return encryptedMsg;
}

ZqPolynomial Encryption::encrypt(const ZpPolynomial& msg, bool showEncryptionTime) const{
    if(this->N != NTRUparameters.get_N() || this->q != zq.get_q()) setNTRUparameters(this->N, this->q);
    ZqPolynomial encryptedMsg = msg.encrypt(publicKey, showEncryptionTime);
    return encryptedMsg;
}

ZpPolynomial Encryption::decrypt(const ZqPolynomial& e_msg, bool showDecryptionTime) const{
    if(!this->validPrivateKey) {
        std::cerr << "\nThis object has no valid private key, therefore is only capable of encryption. Returning zero ZpPolynomial.\n";
        return ZpPolynomial();
    }
    if(this->N != NTRUparameters.get_N() || this->q != zq.get_q()) {
        setNTRUparameters(this->N, this->q);
    }
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    if(showDecryptionTime) begin = std::chrono::steady_clock::now();

    ZpPolynomial msg = mods_p(e_msg*privateKey);
    msg = msg*privateKeyInv_p;

    if(showDecryptionTime) {
        end = std::chrono::steady_clock::now();
        std::cout << "\nMessage was decrypted in " << std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count() << "[µs]\n" << std::endl;
    }
    return msg;
}

//________________________________________________________________________ Encryption _____________________________________________________________________________
