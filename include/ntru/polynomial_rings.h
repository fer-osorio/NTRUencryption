#ifndef POLYNOMIAL_RINGS_H
#define POLYNOMIAL_RINGS_H

#include "parameters_constants.hpp"
#include <cstdint>
#ifndef PARAMETERS_H
    #define PARAMETERS_H
    #ifndef NTRU_N
        #define NTRU_N 701
    #endif
    #ifndef NTRU_Q
        #define NTRU_Q 8192
    #endif
#endif

#include <stdint.h>

enum Zp {zero = 0, one = 1, two = 2};

struct RpPolynomial_t{
    enum Zp coeffs[NTRU_N];
};

struct R0Polynomail_t{
    uint16_t coeffs[NTRU_N];
};

#endif
