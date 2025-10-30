#include "../include/R2Polynomial.h"
#include <stdlib.h>
#include <stdbool.h>

union R2Polynomial_t{
  uint8_t coeffs[NTRU_N];
  uint64_t coeffs_64[NTRU_N/64 + 1];
};

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

/*
 * Gets degree of polynomial
 * If polynomial is zero, it signals it by returning -1
 * */
static int R2Polynomial_t_getDegree(ptrR2Polynomial_t input){
  for(int i = NTRU_N-1; i >= 0; i--) if(input->coeffs[i] != 0) return i;
  return -1;    // Polynomial is zero, returning -1
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
  int dividendDegree = R2Polynomial_t_getDegree(dividend);
  int divisorDegree  = R2Polynomial_t_getDegree(divisor);

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

enum ExceptionCode R2Polynomial_t_computePseudoInverse(ptrR2Polynomial_t input, R2Polynomial_t* destGCD, R2Polynomial_t* destPseudoInverse){
  // Input validation
  if(input == NULL) return NullSource;
  if(destGCD == NULL || destPseudoInverse == NULL) return NullDestination;

  // Working variables
  R2Polynomial_t quotient;
  R2Polynomial_t remainder;
  R2Polynomial_t pastRemainder;
  R2Polynomial_t tmp[2];
  R2Polynomial_t convResult;  // Temporary for multiplication

  // Get degree and leading coefficient of input polynomial
  int inputDeg = R2Polynomial_t_getDegree(input);
  if(inputDeg < 0) return DivisionByZero;         // Guarding against zero division

  // Initialize arrays to zero
  R2Polynomial_t_rewriteAsZero(&tmp[0]);
  R2Polynomial_t_rewriteAsZero(&tmp[1]);
  R2Polynomial_t_rewriteAsZero(&quotient);
  R2Polynomial_t_rewriteAsZero(&remainder);

  // ===== Phase 1: Division of x^N - 1 by input polynomial =====

  // First coefficient of quotient: x^(N-inputDeg) * leadCoeff
  // In Z/2Z, lead coefficient of any polynomial is always 1
  quotient.coeffs[NTRU_N - inputDeg] = 1;

  // Initialize remainder with (x^N-1) - input*x^(N-inputDeg)
  // In Z/2Z, negation is identity, so this is just copying coefficients
  for(size_t i = inputDeg - 1, j = NTRU_N - 1; i > 0; i--, j--) {
    remainder.coeffs[j] = input->coeffs[i];
  }
  remainder.coeffs[NTRU_N - inputDeg] = input->coeffs[0];                       // Special handling for i = 0
  remainder.coeffs[0] = 1;                                                      // Add the constant term from x^N - 1 (the "-1" part). In Z/2Z: -1 = 1

  // Continue division algorithm
  int degDiff, k, l;
  int remdeg = R2Polynomial_t_getDegree(&remainder);
  while(remdeg >= inputDeg) {
    degDiff = remdeg - inputDeg;                                                // Position for next quotient coefficient
    // New quotient coefficient
    quotient.coeffs[degDiff] = 1;                                               // In Z/2Z, inverse of lead coefficient times non-zero elements is always 1
    // Subtract (quotient.coeffs[degDiff] * input) from remainder
    // In Z/2Z: subtraction is the same as addition (both are XOR)
    for(k = inputDeg, l = remdeg; k >= 0; k--, l--) {
      //remainder.coeffs[l] ^= (quotient.coeffs[degDiff] & input->coeffs[k]);   // But quotient.coeffs[degDiff] is 1, defore we obtain the following line
      remainder.coeffs[l] ^= input->coeffs[k];
    }
    // Find the actual degree of current remainder
    remdeg = R2Polynomial_t_getDegree(&remainder);
  }

  // ===== Phase 2: Extended Euclidean Algorithm =====

  // Initialize Bézout coefficient tracking
  // destPseudoInverse = 1 (represents v[-1] in EEA)
  R2Polynomial_t_rewriteAsZero(destPseudoInverse);
  destPseudoInverse->coeffs[0] = 1;

  // tmp[1] = quotient (which is -q[1] in EEA, but in Z/2Z negation is identity)
  R2Polynomial_t_rewriteAs(&tmp[1], &quotient);
  R2Polynomial_t_rewriteAs(&tmp[0], &tmp[1]);

  // Initialize GCD and pastRemainder
  R2Polynomial_t_rewriteAs(destGCD, input);
  R2Polynomial_t_rewriteAs(&pastRemainder, &remainder);

  // Main EEA loop
  enum ExceptionCode divExcep;
  while(!R2Polynomial_t_isZero(&pastRemainder)) {
    divExcep = R2Polynomial_t_division(                                         // Perform division: destGCD / pastRemainder
      destGCD,
      &pastRemainder,
      &quotient,
      &remainder
    );

    if(divExcep != NoException) {
      return divExcep;                                                          // Propagate exception
    }

    // Update Bézout coefficient: tmp[1] = destPseudoInverse - quotient * tmp[0]
    // In Z/2Z: subtraction is identical to addition
    R2Polynomial_t_convolution(&quotient, &tmp[0], &convResult);
    R2Polynomial_t_addition(destPseudoInverse, &convResult, &tmp[1]);

    // Update values for next iteration
    R2Polynomial_t_rewriteAs(destPseudoInverse, &tmp[0]);
    R2Polynomial_t_rewriteAs(&tmp[0], &tmp[1]);
    R2Polynomial_t_rewriteAs(destGCD, &pastRemainder);
    R2Polynomial_t_rewriteAs(&pastRemainder, &remainder);
  }

  return NoException;
}
