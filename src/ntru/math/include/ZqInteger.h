#include "parameters.h"
#include<stdint.h>

const uint16_t q_1     = (uint16_t)NTRU_Q-1;					// Will hold q-1, this will help with mod q operation
const uint16_t q_div_2 = (uint16_t)NTRU_Q >> 1;
const int16_t  negq_1  = (int16_t)~q_1;                                         // This will help in the centering process. Set as ~q_1

// Defininf macros for performance

#define MODQ(t) (t) & q_1

// When t >= 0, t & q_1 is equivalent to t % q since q is a power of 2
// When t <0, ((t) | negq_1) & q_1 computes q - (-t % q) because -t = -(Q-1)q + (q-r) for some 0 <= r < q (Euclid's division lemma)
// When 0 <= t < q, t | negq_1 is equivalent to t - q
#define MODSQ(t) \
  ((t) = (t) >= 0 ? (t) & q_1 : ((t) | negq_1) & q_1) < q_div_2 ? t : t | negq_1;

uint16_t modq(uint16_t t);
uint16_t modsq(uint16_t a);
