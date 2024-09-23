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
static unsigned len(int a) {
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

ZpPolynomial::ZpPolynomial(const ZpPolynomial& P):N(P.N),p(P.p){
	this->coefficients = new Z3[P.N];
	for(int i=0; i < P.N; i++)
		this->coefficients[i] = P.coefficients[i];
}

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

ZpPolynomial::ZpPolynomial(NTRU_N _N_, NTRU_p _p_, const char data[], int length): N(_N_), p(_p_) {
    int i,j,k,l;

    this->coefficients = new Z3[_N_];
    for(i = 0; i < _N_; i++) this->coefficients[i] = _0_;
    if(length <= 0) return;                                                     // Guarding against negative or null length

    for(i = 0, j = 0; i < length && j < _N_; i++) {                             // i will run through data, j through coefficients
        l = (int)(unsigned char)data[i];
        for(k = 0; k < 5 && j < _N_; k++, l/=3) {                               // Here we're supposing _p_ == 3. Basically we're changing from base 2 to base 3
            switch(l%3) {                                                     // in big endian notation. Notice that the maximum value allowed is 242
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
    }
    if(j < _N_)                                                                 // Padding with zeros (provisional, not intended to be secure)
        for(; j < N; j++) this->coefficients[j++] = _0_;
}

ZpPolynomial ZpPolynomial::operator + (const ZpPolynomial& P) const{
    ZpPolynomial r(max_N(this->N, P.N));                                        // Initializing result in the "biggest polynomial ring"
    const ZpPolynomial *small, *big;
    int i;

    if(this->N < P.N) { small = this; big = &P; }                               // 'small' points to the polynomial with the smallest N, 'big' points to the
	else { small = &P; big = this; }                                            // polynomial with the biggest N

    for(i = 0; i < small->N; i++)
        r.coefficients[i] = this->coefficients[i] + P.coefficients[i];          // Addition element by element till the smallest degree of the arguments
    for(; i < big->N; i++)
        r.coefficients[i] = big->coefficients[i];                               // Just copying, equivalent to filling with zeros the small polynomial
    return r;
}

ZpPolynomial ZpPolynomial::operator - (const ZpPolynomial& P) const{
    const NTRU_N _n_ = min_N(this->N, P.N), _N_ = max_N(this->N, P.N);
    ZpPolynomial r(_N_);                                                        // Initializing result in the "biggest polynomial ring"
    int i;

    for(i = 0; i < _n_; i++)
        r.coefficients[i] = this->coefficients[i] - P.coefficients[i];                                                    // Subtraction element by element till the smallest degree of the arguments
    if(P.N == _N_) for(; i < _N_; i++)                                          // Second argument bigger, virtually filling the first argument with zeros
        r.coefficients[i] = -P.coefficients[i];
    else for(; i < _N_; i++)                                                    // First argument bigger, virtually filling the second argument with zeros
        r.coefficients[i] = this->coefficients[i];
    return r;
}

ZpPolynomial ZpPolynomial::operator - () const{
	ZpPolynomial r = *this;
	int i, _degree_ = this->degree();
	for(i = 0; i <= _degree_; i++)
		r.coefficients[i] = -r.coefficients[i];
	return r;
}

ZpPolynomial ZpPolynomial::operator * (const ZpPolynomial& P) const{
    ZpPolynomial r(max_N(this->N, P.N));                                        // Initializing with zeros
    int i, j, k;
    int this_degree = this->degree();

	for(i = 0; i <= this_degree; i++) {
		if(this->coefficients[i] != 0) {
		    k = this->N - i;
		    for(j = 0; j < k; j++) {                                            // Adding and multiplying while the inequality i+j < N holds
			    if(P.coefficients[j] != 0) {                                    // We expect a big proportion of zeros
			        r.coefficients[i+j] += this->coefficients[i]*P.coefficients[j];
			    }
		    }
		    for(k = 0; k < i; j++, k++) {                                       // Using the definition of convolution polynomial rings
			    if(P.coefficients[j] != 0) {                                    // Notice i+k is congruent with j mod N
			        r.coefficients[k] += this->coefficients[i]*P.coefficients[j];
			    }
		    }
		}
	}
    return r;
}

ZpPolynomial& ZpPolynomial::operator-=(const ZpPolynomial& P) {
    const NTRU_N _n_ = min_N(this->N, P.N);
    int i;
    for(i = 0; i < _n_; i++)
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
    result[0] = ZpPolynomial(this->N);

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
    ZpPolynomial gcd(this->N);                                                  // Initializing result in the "biggest polynomial ring
    ZpPolynomial remainders(this->N);
    ZpPolynomial tmp[2] = {ZpPolynomial(this->N),ZpPolynomial(this->N)};
    ZpPolynomial quoRem[2] = {ZpPolynomial(this->N),ZpPolynomial(this->N)};

    quoRem[0].coefficients[this->N-deg] = leadCoeff;                            // Start of division algorithm between virtual polynomial x^N-1 and this
    for(i = deg-1, j = this->N - 1; i >= 0; i--, j--) {                         // First coefficient of quotient and first subtraction
        quoRem[1].coefficients[j] = _0_ - leadCoeff * this->coefficients[i];
    }
    quoRem[1].coefficients[0] = _2_;                                            // Putting the -1 that is at the end of the polynomial x^N-1
    for(i = this->N-1 - deg, j = this->N-1; j >= deg; i = j - deg) {            // Continuing with division algorithm; i is the place of the next coefficient of
        quoRem[0].coefficients[i] = leadCoeff*quoRem[1].coefficients[j];        // the quotient, j is the degree of the remainders.
        for(k = deg, l = j; k >= 0; k--, l--) {                                 // Multiplication-subtraction step
            quoRem[1].coefficients[l] -= quoRem[0].coefficients[i]*this->coefficients[k];
        }
        while(quoRem[1].coefficients[j] == 0) {j--;};
    }                                                                           // End of division algorithm between virtual polynomial x^N-1 and this

    tmp[1] = -quoRem[0]; thisBezout = _1_;                                        // Initializing values for the execution of the rest of the EEA
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
	if(this->N == P.N && this->p == P.p) {									    // The comparison this->p == P.p is naive, but maybe will be useful in the future
		for(int i = 0; i < this->N; i++)
			if(this->coefficients[i] != P.coefficients[i]) return false;
		return true;
	}
	else return false;
}

bool ZpPolynomial::operator != (const ZpPolynomial& P) const{
	if(this->N == P.N && this->p == P.p) {									    // The comparison this->p != P.p is naive, but maybe will be useful in the future
		for(int i = 0; i < this->N; i++)
			if(this->coefficients[i] != P.coefficients[i])
				return true;
		return false;
	}
	else return true;
}

ZpPolynomial& ZpPolynomial::operator = (const ZpPolynomial& P) {
    if(this != &P) {													        // Guarding against self assignment
        if(this->N != P.N) {                                                    // Just in this case makes sense to delete this->coefficients instead of just
			delete [] this->coefficients;                                       // rewrite it
			this->coefficients = new Z3[P.N];
			this->N = P.N;
		}
		this->p = P.p;
	    for(int i = 0; i < P.N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

ZpPolynomial& ZpPolynomial::operator = (int t) {
    if(this->coefficients == NULL)
		this->coefficients = new Z3[this->N];                                   // Dealing with already initialized object, so this->N is well defined
	if(t < 0) t = -t;
	if(t >= this->p) t %= this->p;
	switch(this->p) {
	    case _3_: this->coefficients[0] = (Z3)t; break;
	}
	for(int i = 1; i < this->N; i++) this->coefficients[i] = _0_;               // Filling with zeros the rest of the array
	return *this;
}

void ZpPolynomial::setPermutation() {                                           // Setting a permutation
    int i, j, k, *tmp = new int[this->N];
    RandInt rn(0, 0x7FFFFFFF);                                                  // Random integers from 0 to the maximum number for and int
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
void ZpPolynomial::permute() {
	int i;
	if(this->permutation == NULL) this->setPermutation();
	if(this->coeffCopy   == NULL) {                                             // If there is no a copy, create the copy
	    this->coeffCopy = new Z3[this->N];
	    for(i = 0; i < this->N; i++) this->coeffCopy[i] = this->coefficients[i];
	}
	for(i = 0; i < this->N; i++)                                                // Permute coefficients
		this->coefficients[i] = this->coeffCopy[this->permutation[i]];
	for(i = 0; i < this->N; i++)                                                // Copy coefficients
		this->coeffCopy[i] = this->coefficients[i];
}

static int64_t multiplyBy_3(int64_t t) {
    return (t << 1) + t;                                                        // -This expression is equivalent to t*2 + t
}

ZqPolynomial ZpPolynomial::encrypt(ZqPolynomial publicKey) const{
    ZqPolynomial encryption(publicKey.get_N(), publicKey.get_q());
    int*  randTernaryTimes_p = new int[this->N];                                // -Will represent the random polynomial needed for encryption
    RandInt ri(0,this->N-1);
    const int d = this->N/3;
    int _d_ = d, i, j, k;
    int q_div_2 = publicKey.zq.get_q() >> 1;
    int neg_qdiv2 = -q_div_2;
    int q_div_2_minus1 = q_div_2 - 1;

    for(i = 0; i < this->N; i++) randTernaryTimes_p[i] = 0;

    while(_d_ > 0) {
        i = ri();                                                               // -Filling with threes. It represent the random polynomial multiplied by p
        if(randTernaryTimes_p[i] == 0) {randTernaryTimes_p[i] =  3; _d_--;}     //  ...
    }
    _d_ = d;
    while(_d_ > 0) {
        i = ri();                                                               // -Filling with negative threes
        if(randTernaryTimes_p[i] == 0) {randTernaryTimes_p[i] = -3; _d_--;}     //  ...
    }
	for(i = 0; i < this->N; i++) {                                              // -Convolution process
	    k = this->N - i;
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
	for(i = 0; i < this->N; i++) {                                              // -Adding this polynomial (adding message)
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
    int i,j,k;
    int N_mod_5 = this->N%5;
    int _N_ = this->N - N_mod_5;
    unsigned s;
    for(i = 0, j = 0; i < _N_; i += 5, j++) {                                   // i will run through dest, j through coefficients
        for(k = 4, s = 0; k >= 0; k--) {                                        // Here we're supposing _p_ == 3. Basically we're changing from base 3 to base 2
            switch(this->coefficients[i+k]) {                                   // Supposing the numbers in base 3 are in big endian notation
                case  _1_:
                    s = s*3 + 1;
                break;
                case  _2_:
                    s = s*3 + 2;
                break;
                default:
                    s *= 3;
            }
        }
        dest[j] = (char)(int)s;
    }
    for(k = N_mod_5-1, s = 0; k >= 0; k--) {
        switch(this->coefficients[i+k]) {                                       // Supposing the numbers in base 3 are in big endian notation
                case  _1_:
                    s = s*3 + 1;
                break;
                case  _2_:
                    s = s*3 + 2;
                break;
                default:
                    s *= 3;
        }
        dest[j] = (char)(int)s;
    }
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

//_______________________________________________________________________ ZpPolynomial ___________________________________________________________________________

//|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ZqPolynomial::Z2Polynomial ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

ZqPolynomial::Z2Polynomial::Z2Polynomial(const Z2Polynomial& P): N(P.N) {
    this->coefficients = new Z2[P.N];
    for(int i = 0; i < P.N; i++) this->coefficients[i] = P.coefficients[i];
}

ZqPolynomial::Z2Polynomial::Z2Polynomial(const ZpPolynomial& P): N(P.get_N()) {
	this->coefficients = new Z2[this->N];
	switch(P.get_p()) {
	    case _3_:
	    for(int i = 0; i < this->N; i++) {
		    if(P[i] != 0) this->coefficients[i] = _1_;                          // Two won't go to zero because it's the additive inverse of one in Z3, therefore
		    else this->coefficients[i] = _0_;                                   // it must go to the additive inverse of one in Z2, which if itself
	    }
	    break;
	}
}

ZqPolynomial::Z2Polynomial::Z2Polynomial(NTRU_N _N_, int ones): N(_N_) {
	int i, j;
	RandInt rn(0, _N_-1);                                                       // Random integers from 0 to N-1
    if(ones < 0) ones = -ones;                                                  // Guarding against invalid values of ones and twos. In particular the
    while(ones >= _N_) ones >>= 1;                                              // Dividing by two till getting inside the allowed range
	this->coefficients = new Z2[_N_];
	for(i = 0; i < _N_; i++) this->coefficients[i] = _0_;
	while(ones > 0) {                                                           // Putting the ones
        j = rn();
        if(this->coefficients[j] == _0_) {
            this->coefficients[j] = _1_;
            ones--;
        }
	}
}

ZqPolynomial::Z2Polynomial& ZqPolynomial::Z2Polynomial::operator = (const Z2Polynomial& P)  {
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

ZqPolynomial::Z2Polynomial& ZqPolynomial::Z2Polynomial::operator = (const ZpPolynomial& P) {
    NTRU_N P_N = P.get_N();
	if(this->N != P_N) {											            // Delete past coefficients array. If this->N != P.N is true, there is no reason
		if(this->coefficients != NULL) delete[] this->coefficients;	            // to delete the array
		this->coefficients = new Z2[P_N];
		this->N = P_N;
	}
	if(this->coefficients == NULL) this->coefficients = new Z2[P_N];            // In case of having an object created with the default (private) constructor
	for(int i = 0; i < P_N; i++) {                                              // Fitting the ZpPolynomial in a Z2Polynomial
	    if(P[i] != 0) this->coefficients[i] = _1_;
	    else this->coefficients[i] = _0_;
	}
	return *this;
}

ZqPolynomial::Z2Polynomial& ZqPolynomial::Z2Polynomial::operator = (Z2 t)  {
	if(this->coefficients==NULL) this->coefficients = new Z2[this->N];          // In case of have been generated from the (private) default constructor
	this->coefficients[0] = t;
	for(int i = 1; i < this->N; i++) this->coefficients[i] = _0_;
	return *this;
}

ZqPolynomial::Z2Polynomial ZqPolynomial::Z2Polynomial::operator + (const Z2Polynomial& P) const {
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

ZqPolynomial::Z2Polynomial ZqPolynomial::Z2Polynomial::operator - (const Z2Polynomial& P) const{
    return *this + P;                                                           // In this polynomial ring, subtraction coincide with addition
}

ZqPolynomial::Z2Polynomial ZqPolynomial::Z2Polynomial::operator * (const Z2Polynomial& P) const{ // Classical polynomial multiplication algorithm
    Z2Polynomial r(max_N(this->N, P.N));
    int i, j, k;
    int this_degree = this->degree();

	for(i = 0; i <= this_degree; i++) {
		if(this->coefficients[i] != 0) {
		    k = this->N - i;
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
    result[0] = Z2Polynomial(this->N);
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
    Z2Polynomial gcd(this->N);                                                  // Initializing result in the "biggest polynomial ring
    Z2Polynomial remainders(this->N);
    Z2Polynomial tmp[2] = {Z2Polynomial(this->N),Z2Polynomial(this->N)};
    int deg = this->degree(), i, j, k, l;                                       // Degree of this and some variables for counting
    Z2 leadCoeff = this->coefficients[deg];                                     // Lead coefficient of this polynomial
    Z2Polynomial quoRem[2]={Z2Polynomial(this->N),Z2Polynomial(this->N)};

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
	if(this->N == P.N) {
		for(int i = 0; i < this->N; i++)
			if(this->coefficients[i] != P.coefficients[i])  return false;
		return true;
	}
    return false;
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

ZqPolynomial::ZqPolynomial(NTRU_N _N_, NTRU_q _q_): N(_N_), zq(_q_) {
    this->coefficients = new int64_t[_N_];
    for(int i = 0; i < this->N; i++) this->coefficients[i] = 0;
}

ZqPolynomial::ZqPolynomial(const ZpPolynomial& P,NTRU_q _q_): N(P.get_N()), zq(_q_) {
	this->coefficients = new int64_t[this->N];
	switch(P.get_p()){
	    case _3_:
	    for(int i = 0; i < this->N; i++) {
	        if(P[i] == 2) this->coefficients[i] = -1;                           // Since (1+2) mod 3 == 0, 2 must be sent to the additive inverse of 1 in Zq;
	        else          this->coefficients[i] = (int64_t)P[i];
	    }
	    break;
	}
}

ZqPolynomial::ZqPolynomial(const ZqPolynomial& P): N(P.N), zq(P.zq) {
    this->coefficients = new int64_t[P.N];
    for(int i = 0; i < P.N; i++) this->coefficients[i] = P.coefficients[i];
}

ZqPolynomial::ZqPolynomial(const Z2Polynomial& P,NTRU_q _q_): N(P.get_N()), zq(_q_) {
    this->coefficients = new int64_t[this->N];
    for(int i = 0; i < this->N; i++) this->coefficients[i] = P[i];
}

ZqPolynomial& ZqPolynomial::operator = (const ZqPolynomial& P) {
	if(this != &P) {														    // Guarding against self assignment
		if(this->N != P.N) {												    // Delete pass coefficients array. If this->N != P.N is true, there is no reason
			if(this->coefficients != NULL) delete[] this->coefficients;		    // to delete the array
			this->coefficients = new int64_t[P.N];
			this->N = P.N;
		}
		if(this->coefficients == NULL) this->coefficients = new int64_t[P.N];
		this->zq = P.zq;
		for(int i = 0; i < this->N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

ZqPolynomial& ZqPolynomial::operator = (const ZpPolynomial& P) {
    NTRU_N P_N = P.get_N();
	if(this->N != P_N) {											            // Delete past coefficients array. If this->N != P.N is true, there is no reason
		if(this->coefficients != NULL) delete[] this->coefficients;	            // to delete the array
		this->coefficients = new int64_t[P_N];
		this->N = P_N;
	}
	if(this->coefficients == NULL) this->coefficients = new int64_t[P_N];       // In case of having an object created with the default (private) constructor
	    switch(P.get_p()){
	        case _3_:
	        for(int i = 0; i < P_N; i++) {
	            if(P[i] == 2) this->coefficients[i] = -1;                       // Since (1+2) mod 3 == 0, 2 must be sent to the additive inverse of 1 in Zq;
	            else          this->coefficients[i] = (int64_t)P[i];
	        }
	    }
	return *this;
}

ZqPolynomial ZqPolynomial::operator + (const ZqPolynomial& P) const{
    ZqPolynomial r(max_N(this->N, P.N),max_q(this->get_q(),P.get_q()));         // Initializing result in the "biggest polynomial ring"
    int i, small_degree, big_degree;                                            // Degree of the polynomials, small_degree <= big_degree

    if(this->degree() < P.degree()) {                                           // Determining the smallest degree
        small_degree = this->degree();
        big_degree = P.degree();
    }
	else {
	    small_degree = P.degree();
	    big_degree = this->degree();
	}

    for(i = 0; i <= small_degree; i++)
        r.coefficients[i] = r.zq.add(this->coefficients[i],P.coefficients[i]);  // Addition element by element till the smallest degree of the arguments
    if(this->degree() == big_degree)
        for(; i <= big_degree; i++) r.coefficients[i] = this->coefficients[i];
    else
        for(; i <= big_degree; i++) r.coefficients[i] = P.coefficients[i];
    return r;
}

ZqPolynomial ZqPolynomial::operator - (const ZqPolynomial& P) const{
    ZqPolynomial r(max_N(this->N, P.N),max_q(this->zq.get_q(),P.zq.get_q()));   // Initializing result in the "biggest polynomial ring
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
        r.coefficients[i] = r.zq.subtract(this->coefficients[i],P.coefficients[i]); // Subtraction element by element till the smallest degree of the arguments
    if(this->degree() == big_degree)
        for(; i <= big_degree; i++) r.coefficients[i] = r.zq.mod_q(this->coefficients[i]);
    else
        for(; i <= big_degree; i++) r.coefficients[i] = r.zq.mod_q(-P.coefficients[i]);
    return r;
}

ZqPolynomial ZqPolynomial::operator * (const ZqPolynomial& P) const{
    ZqPolynomial r(max_N(this->N, P.N),max_q(this->zq.get_q(),P.zq.get_q()));   // Initializing result in the "biggest polynomial ring"
    int i, j, k;
    int this_degree = this->degree();

	for(i = 0; i <= this_degree; i++) {
		if(this->coefficients[i] != 0) {                                        // Taking advantage this polynomials have a big proportion of zeros
		    k = this->N - i;
		    for(j = 0; j < k; j++)                                              // Ensuring we do not get out of the polynomial
			    if(P.coefficients[j] != 0)                                      // Expecting a big proportion of zeros
			        r.coefficients[i+j] += this->coefficients[i] * P.coefficients[j];
		    for(k = 0; k < i; j++, k++)                                         // Using the definition of convolution polynomial ring
			    if(P.coefficients[j] != 0)                                      // Notice i+j = i + (k+N-i), so i+j is congruent with k mod N
			        r.coefficients[k] += this->coefficients[i] * P.coefficients[j];
		}
	}
	for(i = 0; i < r.N; i++) r.coefficients[i] = r.zq.mod_q(r.coefficients[i]); // Applying mod q
	return r;
}

ZqPolynomial ZqPolynomial::operator * (const ZpPolynomial& P) const{
    ZqPolynomial r(max_N(this->N, P.get_N()),this->get_q());                    // Initializing result in the "biggest polynomial ring"
    NTRU_N PN = P.get_N();
    int i, j, k;

	for(i = 0; i < PN; i++) {
		if(P[i] != ZpPolynomial::_0_) {                                         // -Taking advantage this polynomials have a big proportion of zeros
		    if(P[i] == ZpPolynomial::_1_) {                                     // -The other two cases are one, as in this line is showed
		        k = this->N - i;
		        for(j = 0; j < k; j++)
		            r.coefficients[i+j] += this->coefficients[j];
		        for(k = 0; k < i; j++, k++)
		            r.coefficients[k] += this->coefficients[j];
		    } else {                                                            // -The only other case is two, which is interpreted as -1
		        k = this->N - i;
		        for(j = 0; j < k; j++)
		            r.coefficients[i+j] -= this->coefficients[j];
		        for(k = 0; k < i; j++, k++)
		            r.coefficients[k] -= this->coefficients[j];
		    }
		}
	}
	r.mods_q();                                                                 // Computing the center modulus (check the function mods_q)
	return r;
}

ZqPolynomial NTRU::operator - (int64_t t, const ZqPolynomial& P) {
    ZqPolynomial r(P.N, P.get_q());
    r.coefficients[0] = P.zq.subtract(t, P.coefficients[0]);
    for(int i = 1; i < P.N; i++) r.coefficients[i] = r.zq.mod_q(-P.coefficients[i]);
    return r;
}

bool ZqPolynomial::operator == (const Z2Polynomial& P) const {
    if(this->N == P.get_N()) {
        for(int i = 0; i < this->N; i++) if(this->coefficients[i] != P[i]) return false;
        return true;
    }
    return false;
}

ZpPolynomial NTRU::mods_p(ZqPolynomial P) {
    NTRU_N _N_ = P.get_N();
    ZpPolynomial r(_N_);
    for(int i = 0, buff = 0; i < _N_; i++) {
        buff = P[i] % 3;
        if(buff == -2 || buff == 1) r.coefficients[i] = ZpPolynomial::_1_;
        if(buff == -1 || buff == 2) r.coefficients[i] = ZpPolynomial::_2_;
    }
    return r;
}

ZqPolynomial ZqPolynomial::getNTRUpublicKey() {                                 // -Returns public key provided this ZqPolynomial object is the inverse mod q in
    NTRU_q _q_ = this->get_q();                                                 //  Z[x]/X^N-1 of the private key
    ZqPolynomial publicKey(this->N, _q_);
    int* randTernary = new int[this->N];
    RandInt ri(0,this->N- 1);                                                   // -Random integers from 0 to this->N - 1
    const int d_=this->N/3;
    int _d_ = d_;
    int i, j, k;

    for(i = 0; i < this->N; i++) randTernary[i] = 0;

    while(_d_ > 0) {                                                            // -Building ternary polynomial multiplied by p
        i = ri();                                                               // -Filling with p
        if(randTernary[i] == 0) {randTernary[i] =  1; _d_--;}                   // ...
    }
    _d_ = d_;
    while(_d_ > 0) {
        i = ri();                                                               // -Filling with negative p
        if(randTernary[i] == 0) {randTernary[i] = -1; _d_--;}                   // ...
    }
	for(i = 0; i < this->N; i++) {                                              // -Convolution process
	    k = this->N - i;
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
    int const _q_ = this->zq.get_q();
    longlong_to_char ll_c = {0};                                                // ll_c will do the cast from int to char[]
    int _64 = 64;
    long long buff;
    for(buff = _q_ >> 1; buff > 1; buff >>= 1, log2q++) {}                      // Computing log2q using the fact q is a power of two

    for(k = 0, buff = 0; i < this->N;) {
        ll_c._longlong_ >>= (l<<3);                                             // l*8; Ruling out the bits allocated in the last cycle
        while(k < _64 - log2q) {
            buff = this->coefficients[i++];
            if(buff <= 0) buff += _q_;
            ll_c._longlong_ |= buff << k; k += log2q;                           // Allocating log2q bits in ll_c._int_; increasing k
            if(i >= this->N) break;
        }
        for(l = 0; k >= 8; l++, k -= 8) dest[j++] = ll_c._char[l];              // Writing the last two bytes on destination
    }
    if(k > 0) dest[j++] = ll_c._char[l];
}

void ZqPolynomial::print(const char* name,const char* tail) const{
    unsigned len_q = len(this->zq.get_q());
    int coeffAmount = this->degree() + 1;                                       // This three lines is a "casting" from int64_t array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printArray(array, (unsigned)coeffAmount, len_q+1, name, tail);
    delete[] array;
}

void ZqPolynomial::println(const char* name) const{
    this->print(name, "\n");
}

//_______________________________________________________________________ ZqPolynomial ____________________________________________________________________________

// ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| Encryption |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

Encryption::Encryption(NTRU_N _N_,NTRU_q _q_,int _d_,NTRU_p _p_):N(_N_),q(_q_),p(_p_),d(_d_),publicKey(_N_,_q_),privateKey(_N_,_p_),privateKeyInv_p(_N_,_p_) {
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
    int l = -1; while(fstring[++l] != 0) {}
    ZpPolynomial msg(this->N, this->p, fstring, l);
    ZqPolynomial encryptedMsg = msg.encrypt(this->publicKey);
    return encryptedMsg;
}

ZpPolynomial Encryption::decrypt(const ZqPolynomial& e_msg) {
    ZpPolynomial msg = mods_p(e_msg*this->privateKey);
    msg = msg*this->privateKeyInv_p;
    return msg;
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
    ZpPolynomial  Zp_privateKey(this->N, this->d+1, this->d);
    ZpPolynomial  Zp_privateKeyInv(this->N, this->q);
	ZpPolynomial  Zp_gcdXNmns1(this->N, this->q);
	ZqPolynomial  Zq_privateKey(this->N, this->q);
	ZqPolynomial  Zq_privateKeyInv(this->N, this->q);
	ZqPolynomial::Z2Polynomial Z2_privateKey(Zp_privateKey);
	ZqPolynomial::Z2Polynomial Z2_privateKeyInv(this->N, 0);
	ZqPolynomial::Z2Polynomial Z2_gcdXNmns1(this->N, 0);
	int counter, k = 2, l = 1;

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
	        this->d--;                                                          // To much tries, increasing the numbers of non-zero coefficients
            Zp_privateKey = ZpPolynomial(this->N, this->d+1, this->d);          // ...
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
	Zq_privateKeyInv = ZqPolynomial(Z2_privateKeyInv, this->q);

	while(k < this->q) {
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
