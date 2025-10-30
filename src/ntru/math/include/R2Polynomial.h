#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../include/ntru/parameters_constants.hpp"
#include "exceptions.h"
#include <stdint.h>

typedef union R2Polynomial_t R2Polynomial_t;
typedef const union R2Polynomial_t* ptrR2Polynomial_t;

// Get and write.
enum ExceptionCode R2Polynomial_t_getFromArrayPointer(const uint8_t* source, R2Polynomial_t* dest);
enum ExceptionCode R2Polynomial_t_writeOnArrayPointer(ptrR2Polynomial_t source, uint8_t* dest);

enum ExceptionCode R2Polynomial_t_multiply(ptrR2Polynomial_t input1, ptrR2Polynomial_t input2, R2Polynomial_t* output);
enum ExceptionCode R2Polynomial_t_division(ptrR2Polynomial_t divident, ptrR2Polynomial_t divisor, R2Polynomial_t* quotient, R2Polynomial_t* remainder);
enum ExceptionCode R2Polynomial_t_computePseudoInverse(ptrR2Polynomial_t input, R2Polynomial_t* destGCD, R2Polynomial_t* destPseudoInverse);

#ifdef __cplusplus
}
#endif
