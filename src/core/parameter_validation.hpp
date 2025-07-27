#include<cstdint>

#ifndef PARAMETER_VALIDATION_HPP
#define PARAMETER_VALIDATION_HPP

enum ntru_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821, _1087_ = 1087, _1171_ = 1171, _1499_ = 1499 };	// All the possible values for the N
enum ntru_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };			// All the possible values for the q
//enum ntru_p {_3_= 3 };

#ifndef NTRU_N
    #define NTRU_N 701
#endif
#ifndef NTRU_Q
    #define NTRU_Q 8192
#endif

static int log2_q(ntru_q q) {                                              // -Returns logarithm base 2 of a ntru_q value
    int log2q = 0, qq = q;
    for(; qq > 1; qq >>= 1, log2q++) {}
    return log2q;
}

int64_t q_1     = (int64_t)NTRU_Q-1;										// Will hold q-1, this will help with mod q operation
int64_t negq_1  = ~q_1;													// This will help in the centering process. Set as ~q_1
int64_t q_div_2 = NTRU_Q>>1;
int     log2q   = log2_q((ntru_q)NTRU_Q);

int64_t modq(int64_t t) {											// operation t % q
    if(t >= 0)	return t & q_1;									// Equivalent to t % q since q is a power of 2
    else 		return (t | negq_1);								// Computing q - (-t%q) because -t = -(Q-1)q + (q-r) 0 <= r < q
}

int64_t modsq(int64_t a) {
    int64_t r;
    if(a >= 0) r = a & q_1;										// Equivalent to a % q since q is a power of 2
        else r = (a | negq_1) & q_1;							// Computing q - (-a%q) because -t = -(Q-1)q + (q-r) 0 <= r < q
    if(r < q_div_2) return r;										// At this point we know 0 <= r < q
    else return r | negq_1;										// This is equivalent to r - this->q when r < q
}

#endif