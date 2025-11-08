#ifdef __cplusplus
extern "C" {
#endif

#include "exceptions.h"
#include <stdint.h>

typedef struct RpPolynomial_t RpPolynomial_t;
typedef const struct RpPolynomial_t* ptrRpPolynomial_t;

// Get and write.
enum ExceptionCode RpPolynomial_t_getCopyFromArray(const uint8_t* source, RpPolynomial_t* dest);
enum ExceptionCode RpPolynomial_t_writeCopyOnArray(ptrRpPolynomial_t source, uint8_t* dest);

//enum ExceptionCode RpPolynomial_t_multiply(ptrRpPolynomial_t input1, ptrRpPolynomial_t input2, RpPolynomial_t* output);
#ifdef __cplusplus
}
#endif
