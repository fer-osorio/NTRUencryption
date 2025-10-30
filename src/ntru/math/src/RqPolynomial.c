#include "../include/RqPolynomial.h"
#include <stdlib.h>
#include <stdbool.h>

struct RqPolynomial_t{
  uint16_t coeffs[NTRU_N];
};

enum ExceptionCode RqPolynomial_t_getFromArrayPointer(const uint16_t* source, RqPolynomial_t* dest){
  if(source == NULL) return NullSource;
  if(dest   == NULL) return NullDestination;
  for(size_t i = 0; i < NTRU_N; i++) dest->coeffs[i] = source[i];
  return NoException;
}

enum ExceptionCode RqPolynomial_t_writeOnArrayPointer(ptrRqPolynomial_t source, uint16_t* dest){
  if(source == NULL) return NullSource;
  if(dest   == NULL) return NullDestination;
  for(size_t i = 0; i < NTRU_N; i++) dest[i] = source->coeffs[i];
  return NoException;
}
