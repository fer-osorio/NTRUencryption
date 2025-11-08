#include "../include/RpPolynomial.h"
#include "../include/parameters.h"
#include <stdlib.h>
#include <stdbool.h>

struct RpPolynomial_t{
  uint8_t coeffs[NTRU_N];
};

enum ExceptionCode RpPolynomial_t_getCopyFromArray(const uint8_t* source, RpPolynomial_t* dest){
  if(source == NULL) return NullSource;
  if(dest   == NULL) return NullDestination;
  for(size_t i = 0; i < NTRU_N; i++) dest->coeffs[i] = source[i];
  return NoException;
}

enum ExceptionCode RpPolynomial_t_writeOnArrayPointer(ptrRpPolynomial_t source, uint8_t* dest){
  if(source == NULL) return NullSource;
  if(dest   == NULL) return NullDestination;
  for(size_t i = 0; i < NTRU_N; i++) dest[i] = source->coeffs[i];
  return NoException;
}
