#include<cstdint>
#include"../../include/ntru/parameters.hpp"

#ifndef PARAMETER_VALIDATION_HPP
#define PARAMETER_VALIDATION_HPP

#ifndef _N_
    #define _N_ 701
#endif
#ifndef _q_
    #define _q_ 8192
#endif

static int log2_q(NTRU_q q) {                                              // -Returns logarithm base 2 of a NTRU_q value
    int log2q = 0, qq = q;
    for(; qq > 1; qq >>= 1, log2q++) {}
    return log2q;
}

int64_t q_1     = (int64_t)_q_-1;										// Will hold q-1, this will help with mod q operation
int64_t negq_1  = ~q_1;													// This will help in the centering process. Set as ~q_1
int64_t q_div_2 = _q_>>1;
int     log2q   = log2_q((NTRU_q)_q_);

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