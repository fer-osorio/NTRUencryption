#include "../headers/R2Polynomial.h"
#include <stdlib.h>

static const size_t NTRUN_div_64_1 = NTRU_N/64 + 1;

enum ExceptionCode R2Polynomial_t_getFromArrayPointer(const uint8_t* source, R2Polynomial_t* dest){
  if(source == NULL) return NullSource;
  if(dest   == NULL) return NullDestination;
  for(size_t i = 0; i < NTRU_N; i++) dest->coeffs[i] = source[i];
  return NoException;
}

static void R2Polynomial_t_addition(ptrR2Polynomial_t input1, ptrR2Polynomial_t input2, R2Polynomial_t* output){
  for(size_t i = 0; i < NTRUN_div_64_1; i++)
    output->coeffs_64[i] = input1->coeffs_64[i] ^ input2->coeffs_64[i];
}

static void R2Polynomial_t_rotateLeft(size_t rotAmount, ptrR2Polynomial_t input, R2Polynomial_t* output){
  if(rotAmount >= NTRU_N) rotAmount %= NTRU_N;
  size_t diff = NTRU_N - rotAmount, i, j;
  for(i = 0, j = diff; j < NTRU_N; i++, j++) output->coeffs[i] = input->coeffs[j];
  for(j = 0; i < NTRU_N; i++, j++) output->coeffs[i] = input->coeffs[j];
}

enum ExceptionCode R2Polynomial_t_convolution(ptrR2Polynomial_t input1, ptrR2Polynomial_t input2, R2Polynomial_t* output){
  if(input1 == NULL || input2 == NULL) return NullInput;
  if(output == NULL) return NullOutput;
  R2Polynomial_t buff;
  for(size_t i = 0; i < NTRU_N; i++){
    if(input2->coeffs[i] != 0){
      R2Polynomial_t_rotateLeft(i, input2, &buff);
      R2Polynomial_t_addition(&buff, output, output);
    }
  }
  return NoException;
}
