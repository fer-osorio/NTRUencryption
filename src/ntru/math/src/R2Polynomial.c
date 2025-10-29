#include "../include/R2Polynomial.h"
#include <stdlib.h>
#include <stdbool.h>

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

static bool R2Polynomial_t_isZero(ptrR2Polynomial_t input){
  for(size_t i = 0; i < NTRU_N; i++) if(input->coeffs[i] != 0) return false;
  return true;
}

static size_t R2Polynomial_t_getDegree(ptrR2Polynomial_t input){
  for(size_t i = NTRU_N-1; i > 0; i--) if(input->coeffs[i] != 0) return i;
  return 0;
}

static void R2Polynomial_t_rewriteAsZero(R2Polynomial_t* output){
  for(size_t i = 0; i < NTRU_N; i++) output->coeffs[i] = 0;
}

static void R2Polynomial_t_rewriteAs(R2Polynomial_t* dest, ptrR2Polynomial_t source){
  for(size_t i = 0; i < NTRU_N; i++) dest->coeffs[i] = source->coeffs[i];
}

enum ExceptionCode R2Polynomial_t_division(ptrR2Polynomial_t dividend, ptrR2Polynomial_t divisor, R2Polynomial_t* quotient, R2Polynomial_t* remainder){
  if(dividend == NULL || divisor == NULL)   return NullSource;                  // Guarding against null sources
  if(quotient == NULL || remainder == NULL) return NullDestination;             // Guarding against null destinations
  if(R2Polynomial_t_isZero(divisor)) return DivisionByZero;                     // Guarding against division by zero

  // Handle zero dividend case
  if(R2Polynomial_t_isZero(dividend)) {
    // Zero out quotient and remainder
    R2Polynomial_t_rewriteAsZero(quotient);
    R2Polynomial_t_rewriteAsZero(remainder);
    return NoException;
  }

  // Get degrees
  int dividendDegree = (int)R2Polynomial_t_getDegree(dividend);
  int divisorDegree  = (int)R2Polynomial_t_getDegree(divisor);

  // Handle case where dividend degree < divisor degree
  if(dividendDegree < divisorDegree) {
    // quotient equals 0
    R2Polynomial_t_rewriteAsZero(quotient);
    // Remainder equals dividend
    R2Polynomial_t_rewriteAs(remainder, dividend);
    return NoException;
  }

  // Initialize quotient to zero
  R2Polynomial_t_rewriteAsZero(quotient);
  // Initialize remainder with dividend
  R2Polynomial_t_rewriteAs(remainder, dividend);

  int degreeDiff = dividendDegree - divisorDegree;
  int remDeg = dividendDegree;

  // Polynomial long division loop
  while(degreeDiff >= 0) {
    // Calculate new quotient coefficient
    // In Z/2Z (binary field), the leading coefficient is always 1 (non-zero) and 1 is its own multiplicative inverse, so leadCoeffOfDivisorInverse = 1 and
    // remainder->coeffs[remDeg] = 1, so leadCoeffOfDivisorInverse * remainder->coeffs[remDeg] simplifies to just 1
    quotient->coeffs[degreeDiff] = 1;

    // Update remainder by subtracting (quotient_coeff * divisor)
    // In Z/2Z, subtraction is the same as addition (both are XOR)
    for(int i = remDeg; i >= degreeDiff; i--) {
      // remainder->coeffs[i] -= quotient->coeffs[degreeDiff] * divisor->coeffs[i - degreeDiff];
      // In Z/2Z with uint8_t storage, this becomes:
      remainder->coeffs[i] ^= (quotient->coeffs[degreeDiff] & divisor->coeffs[i - degreeDiff]);
    }
    // Find new degree of remainder
    remDeg = R2Polynomial_t_getDegree(remainder);
    // Update degree difference for next iteration
    degreeDiff = remDeg - divisorDegree;
  }
  return NoException;
}
