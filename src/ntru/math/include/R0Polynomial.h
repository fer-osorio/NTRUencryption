#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../include/ntru/parameters_constants.hpp"
#include "exceptions.h"
#include "../include/R2Polynomial.h"
#include "../include/RpPolynomial.h"
#include <stdint.h>

typedef struct R0Polynomial_t R0Polynomial_t;
typedef const struct R0Polynomial_t* ptrR0Polynomial_t;

// Get and write.
enum ExceptionCode R0Polynomial_t_getFromArrayPointer(const uint16_t* source, R0Polynomial_t* dest);
enum ExceptionCode R0Polynomial_t_writeOnArrayPointer(ptrR0Polynomial_t source, uint16_t* dest);

enum ExceptionCode R0Polynomial_t_multiply(ptrR0Polynomial_t input1, ptrR0Polynomial_t input2, R0Polynomial_t* output);
enum ExceptionCode R0Polynomial_t_convolutionR2Rp(ptrR2Polynomial_t input1, ptrRpPolynomial_t input2, R0Polynomial_t* output);
enum ExceptionCode R0Polynomial_t_convolutionRpR0(ptrRpPolynomial_t input1, ptrR0Polynomial_t input2, R0Polynomial_t* output);

#ifdef __cplusplus
}
#endif
