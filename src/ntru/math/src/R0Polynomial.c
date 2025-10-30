#include "../include/RqPolynomial.h"
#include "../include/ZqInteger.h"
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

static void RqPolynomial_t_modq(ptrRqPolynomial_t input, RqPolynomial_t* output){
  for(size_t i = 0; i < NTRU_N; i++) output->coeffs[i] = MODQ(input->coeffs[i]);
}

enum ExceptionCode RqPolynomial_t_multiply(ptrRqPolynomial_t input1, ptrRqPolynomial_t input2, RqPolynomial_t* output){
  int i, j, k;
  for(i = 0; i < NTRU_N; i++) {
    k = NTRU_N - i;
    for(j = 0; j < k; j++)                                                      // Ensuring we do not get out of the polynomial
      output->coeffs[i+j] += input1->coeffs[i] * input2->coeffs[j];
    for(k = 0; k < i; j++, k++)                                                 // Using the definition of convolution polynomial ring
      output->coeffs[k] += input1->coeffs[i] * input2->coeffs[j];
  }
  RqPolynomial_t_modq(output,output);                                           // Applying mod q
}
