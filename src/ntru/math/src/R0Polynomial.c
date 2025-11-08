#include "../include/R0Polynomial.h"
#include "../include/ZqInteger.h"
// #include "../include/parameters.h" // Already included in ZqInteger.h
#include <stdlib.h>
#include <stdbool.h>

struct R0Polynomial_t{
  uint16_t coeffs[NTRU_N];
};

enum ExceptionCode R0Polynomial_t_getFromArrayPointer(const uint16_t* source, R0Polynomial_t* dest){
  if(source == NULL) return NullSource;
  if(dest   == NULL) return NullDestination;
  for(size_t i = 0; i < NTRU_N; i++) dest->coeffs[i] = source[i];
  return NoException;
}

enum ExceptionCode R0Polynomial_t_writeOnArrayPointer(ptrR0Polynomial_t source, uint16_t* dest){
  if(source == NULL) return NullSource;
  if(dest   == NULL) return NullDestination;
  for(size_t i = 0; i < NTRU_N; i++) dest[i] = source->coeffs[i];
  return NoException;
}

static void R0Polynomial_t_modq(ptrR0Polynomial_t input, R0Polynomial_t* output){
  for(size_t i = 0; i < NTRU_N; i++) output->coeffs[i] = MODQ(input->coeffs[i]);
}

enum ExceptionCode R0Polynomial_t_multiply(ptrR0Polynomial_t input1, ptrR0Polynomial_t input2, R0Polynomial_t* output){
  int i, j, k;
  if(input1 == NULL || input2 == NULL) return NullInput;
  if(output == NULL) return NullOutput;
  for(i = 0; i < NTRU_N; i++) {
    k = NTRU_N - i;
    for(j = 0; j < k; j++)                                                      // Ensuring we do not get out of the polynomial
      output->coeffs[i+j] += input1->coeffs[i] * input2->coeffs[j];
    for(k = 0; k < i; j++, k++)                                                 // Using the definition of convolution polynomial ring
      output->coeffs[k] += input1->coeffs[i] * input2->coeffs[j];
  }
  R0Polynomial_t_modq(output,output);                                           // Applying mod q
  return NoException;
}
