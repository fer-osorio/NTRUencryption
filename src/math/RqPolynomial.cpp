#include"../../include/ntru/polynomials.hpp"
#include"../core/parameter_validation.hpp"
#include"../utils/print.hpp"

using namespace NTRU;

RqPolynomial::RqPolynomial() {
    this->coefficients = new int64_t[NTRU_N];
    for(int i = 0; i < NTRU_N; i++) this->coefficients[i] = 0;
}

RqPolynomial::RqPolynomial(const RqPolynomial& P) {
    this->coefficients = new int64_t[NTRU_N];
    for(int i = 0; i < NTRU_N; i++) this->coefficients[i] = P.coefficients[i];
}

RqPolynomial::RqPolynomial(const char data[], int dataLength) {
    int bitsOcupiedInBuff = 0;
    int i , j, dataByteIndex;
    int offset = 0;
    int64_to_char aux;
    uint64_t buff = 0;
    this->coefficients = new int64_t[NTRU_N];
    for(i = 0, dataByteIndex = 0; dataByteIndex < dataLength && i < NTRU_N ;) {
        aux.int64 = 0;
        for(j = 0; bitsOcupiedInBuff <= 56 && dataByteIndex < dataLength; bitsOcupiedInBuff += 8) {// -If we are using at most 56 bits of the buffer, we can
            aux.chars[j++] = data[dataByteIndex++];                             //  still allocate one more byte
        }
        buff |= (uint64_t)aux.int64 << offset;
        for(; bitsOcupiedInBuff >= log2q; bitsOcupiedInBuff -= log2q, i++) {
            this->coefficients[i] = (int64_t)buff & q_1;                        // -Taking the first log2q bits of buff. The result will be positive
            if(this->coefficients[i] >= q_div_2) this->coefficients[i] |= negq_1; // This is equivalent to r - this->q when r < q
            buff >>= log2q;
        }
        offset = bitsOcupiedInBuff;
    }
    if(bitsOcupiedInBuff > 0) {
        if(i < NTRU_N) {
            this->coefficients[i] = (int64_t)buff & q_1;                        // -Taking the first log2q bits of buff. The result will be positive
            if(this->coefficients[i] >= q_div_2) this->coefficients[i] |= negq_1;// This is equivalent to r - this->q when r < q
            buff >>= log2q;
            i++;
        }
    }
    for(;i < NTRU_N; i++) this->coefficients[i] = 0;                               // -Padding the rest of the polynomial with zeros
}

RqPolynomial& RqPolynomial::operator = (const RqPolynomial& P) {
	if(this != &P) {														    // Guarding against self assignment
		if(this->coefficients != NULL) delete[] this->coefficients;
		this->coefficients = new int64_t[NTRU_N];
		for(int i = 0; i < NTRU_N; i++) this->coefficients[i] = P.coefficients[i];
	}
	return *this;
}

int64_t RqPolynomial::operator [] (int i) const{
	if(i < 0) i = -i;
	if(i > NTRU_N) i %= NTRU_N;
	return this->coefficients[i];
}

RqPolynomial RqPolynomial::operator + (const RqPolynomial& P) const{
    RqPolynomial r;                                                             // -Initializing with the zero polynomial
    for(int i = 0; i < NTRU_N; i++)
        r.coefficients[i] = modq(this->coefficients[i] + P.coefficients[i]);          // -Addition element by element till the smallest degree of the arguments
    return r;
}

RqPolynomial RqPolynomial::operator * (const RqPolynomial& P) const{
    RqPolynomial r;
    int i, j, k;
	for(i = 0; i < NTRU_N; i++) {
		k = NTRU_N - i;
	    for(j = 0; j < k; j++)                                                  // Ensuring we do not get out of the polynomial
		    r.coefficients[i+j] += this->coefficients[i] * P.coefficients[j];
	    for(k = 0; k < i; j++, k++)                                             // Using the definition of convolution polynomial ring
		    r.coefficients[k] += this->coefficients[i] * P.coefficients[j];
	}
	r.mod_q();                                                                  // Applying mod q
	return r;
}

RqPolynomial NTRU::operator - (int64_t t, const RqPolynomial& P) {
    RqPolynomial r;
    r.coefficients[0] = modq(t - P.coefficients[0]);
    for(int i = 1; i < NTRU_N; i++) r.coefficients[i] = -P.coefficients[i];
    return r;
}

RpPolynomial NTRU::mods_p(RqPolynomial P) {
    RpPolynomial r;
    for(int i = 0, buff = 0; i < NTRU_N; i++) {
        buff = P[i] % 3;
        if(buff == -2 || buff == 1) r.coefficients[i] = RpPolynomial::_1_;
        if(buff == -1 || buff == 2) r.coefficients[i] = RpPolynomial::_2_;
    }
    return r;
}

RqPolynomial NTRU::convolutionRq(const R2Polynomial& z2P, const RpPolynomial& zpP) {
    RqPolynomial r;
    int i, j, k;
    for(i = 0; i < NTRU_N; i++) {
        if(z2P.coefficients[i] != R2Polynomial::Z2::_0_) {
            k = NTRU_N - i;
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
    r.mod_q();                                                                  // Applying mods q
    return r;
}

RqPolynomial NTRU::convolutionRq(const R2Polynomial& z2P, const RqPolynomial& zqP) {
    RqPolynomial r;
    int i, j, k;
    for(i = 0; i < NTRU_N; i++) {
        if(z2P.coefficients[i] != R2Polynomial::Z2::_0_) {
            k = NTRU_N - i;
		    for(j = 0; j < k; j++)
		        r.coefficients[i+j] += zqP.coefficients[j];
		    for(k = 0; k < i; j++, k++)
		        r.coefficients[k] += zqP.coefficients[j];
        }
    }
    r.mod_q();                                                                  // Applying mods q
    return r;
}

RqPolynomial NTRU::convolutionRq(const RpPolynomial& p1, const RqPolynomial& p2) {
    RqPolynomial r;
    int i, j, k;
	for(i = 0; i < NTRU_N; i++) {
		if(p1.coefficients[i] != RpPolynomial::_0_) {                           // -Taking advantage this polynomials have a big proportion of zeros
		    if(p1.coefficients[i] == RpPolynomial::_1_) {                       // -The other two cases are one, as in this line is showed
		        k = NTRU_N - i;
		        for(j = 0; j < k; j++)
		            r.coefficients[i+j] += p2.coefficients[j];
		        for(k = 0; k < i; j++, k++)
		            r.coefficients[k] += p2.coefficients[j];
		    } else {                                                            // -The only other case is two, which is interpreted as -1
		        k = NTRU_N - i;
		        for(j = 0; j < k; j++)
		            r.coefficients[i+j] -= p2.coefficients[j];
		        for(k = 0; k < i; j++, k++)
		            r.coefficients[k] -= p2.coefficients[j];
		    }
		}
	}
	r.mod_q();                                                                 // Applying mods q
	return r;
}

int RqPolynomial::degree() const{											    // -Returns degree of polynomial
	int deg = NTRU_N;
	while(this->coefficients[--deg] == 0 && deg > 0) {}
	return deg;
}

bool RqPolynomial::equalsOne() const{
    if(this->coefficients[0] != 1) return false;
    for(size_t i = 1; i < NTRU_N; i++) if(this->coefficients[i] != 0) return false;
    return true;
}

void RqPolynomial::mod_q() const{
    for(int i = 0; i < NTRU_N; i++) this->coefficients[i] = modq(this->coefficients[i]);
}

void RqPolynomial::mods_q() const{
    for(int i = 0; i < NTRU_N; i++) this->coefficients[i] = modsq(this->coefficients[i]);
}

int RqPolynomial::lengthInBytes() const{
    return NTRU_N*log2q/8 + 1;
}

void RqPolynomial::toBytes(char dest[]) const{                                  // -Supposing dest is pointing to a suitable memory location
    const int buffBitsSize = 64;
    int i = 0, j = 0;                                                           // -log2q will hold the logarithm base two of q. Here we are assuming q < 2^32)
    int bitsAllocInBuff = 0;                                                    // -Amount of bits allocated (copied from coefficients array) in buffer
    int bytesAllocInDest = 0;
    int64_to_char buffer = {0};                                                 // -buffer will do the cast from int to char[]
    int64_t aux;
    for(bitsAllocInBuff = 0, aux = 0; i < NTRU_N;) {
        buffer.int64 >>= (bytesAllocInDest << 3);                               // -l*8; Ruling out the bits allocated in the last cycle
        while(bitsAllocInBuff < buffBitsSize - log2q) {                         // -Allocating bits from coefficients to buffer
            aux = this->coefficients[i++];
            if(aux < 0) aux += NTRU_Q;
            buffer.int64 |= aux << bitsAllocInBuff;                             // -Allocating log2q bits in buffer._int_;
            bitsAllocInBuff += log2q;                                           // -increasing amount of bits allocated in buffer
            if(i >= NTRU_N) break;
        }
        for(bytesAllocInDest = 0; bitsAllocInBuff >= 8; bytesAllocInDest++, bitsAllocInBuff -= 8)
            dest[j++] = buffer.chars[bytesAllocInDest];                         // -Writing buffer in destination (as long there are at least 8 bits in buffer)
    }
    if(bitsAllocInBuff > 0) dest[j++] = buffer.chars[bytesAllocInDest];         // -The bits that remain in buffer, we allocate them in the last byte
}

void RqPolynomial::print(const char* name,const char* tail) const{
    unsigned len_q = lengthHexadecimalInt(NTRU_Q);
    int coeffAmount = this->degree() + 1;                                       // -This three lines is a "casting" from int64_t array to int array
    int* array = new int[coeffAmount], i;                                       // ...
    for(i = 0; i < coeffAmount; i++) array[i] = (int)this->coefficients[i];     // ...
    printIntArray(array, (unsigned)coeffAmount, len_q+1, name, tail);
    delete[] array;
}

void RqPolynomial::println(const char* name) const{
    this->print(name, "\n");
}

static int64_t multiplyBy_3(int64_t t) {
    return (t << 1) + t;                                                        // -This expression is equivalent to t*2 + t
}

RqPolynomial& RqPolynomial::timesThree(){
    for(int i = 0; i < NTRU_N; i++){
        this->coefficients[i] = multiplyBy_3(this->coefficients[i]);            // -Getting 3p
    }
    return *this;
}
