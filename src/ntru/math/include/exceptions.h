#ifndef EXCEPTION
#define EXCEPTION

#ifdef __cplusplus
extern "C" {
#endif

enum ExceptionCode {
  NoException,
  NullSource, NullDestination, NullInput, NullOutput,
  ZeroLength, InvalidKeyLength, InvalidInputSize,
  UnknownOperation
};

#ifdef __cplusplus
}
#endif

#endif
