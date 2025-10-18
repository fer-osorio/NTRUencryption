#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../include/ntru/parameters_constants.hpp"
#include "exceptions.h"
#include <stdint.h>

typedef union R2Polynomial_t{
  uint8_t coeffs[NTRU_N];
  uint64_t coeffs_64[NTRU_N/64 + 1];
} R2Polynomial_t;
typedef const union R2Polynomial_t* ptrR2Polynomial_t;

enum ExceptionCode R2Polynomial_t_getFromArrayPointer(const uint8_t* source, R2Polynomial_t* dest);
enum ExceptionCode R2Polynomial_t_convolution(ptrR2Polynomial_t input1, ptrR2Polynomial_t input2, R2Polynomial_t* output);
enum ExceptionCode R2Polynomial_t_division(ptrR2Polynomial_t divident, ptrR2Polynomial_t divisor, R2Polynomial_t* quotient, R2Polynomial_t* remainder);

#ifdef __cplusplus
}
#endif
