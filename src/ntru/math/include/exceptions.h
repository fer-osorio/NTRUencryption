#ifndef EXCEPTION
#define EXCEPTION

#ifdef __cplusplus
extern "C" {
#endif

enum ExceptionCode {
  NoException,
  DivisionByZero,                                                               // Math exception
  NullSource, NullDestination, NullInput, NullOutput,                           // Null pointer exception
  ZeroLength, InvalidKeyLength, InvalidInputSize,                               // Invalid size
  UnknownOperation                                                              // Unknown Operation/Method
};

#ifdef __cplusplus
}
#endif

#endif
