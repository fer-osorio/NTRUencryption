#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../include/ntru/parameters_constants.hpp"
#include "exceptions.h"
#include <stdint.h>

typedef struct RqPolynomial_t RqPolynomial_t;
typedef const struct RqPolynomial_t* ptrRqPolynomial_t;

// Get and write.
enum ExceptionCode RqPolynomial_t_getFromArrayPointer(const uint16_t* source, RqPolynomial_t* dest);
enum ExceptionCode RqPolynomial_t_writeOnArrayPointer(ptrRqPolynomial_t source, uint16_t* dest);

enum ExceptionCode RqPolynomial_t_multiply(ptrRqPolynomial_t input1, ptrRqPolynomial_t input2, RqPolynomial_t* output);

#ifdef __cplusplus
}
#endif
