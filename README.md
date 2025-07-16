# NTRU Encryption Library

A C++ implementation of the NTRU lattice-based cryptographic algorithm, providing quantum-resistant encryption capabilities.
Mail me at aosorios1502@alumno.ipn.mx and alexis.fernando.osorio.sarabio@gmail.com for questions and comments.

## Overview

NTRU (Number Theory Research Unit) is a lattice-based public-key cryptographic system resistant to attacks using Shor's alrithm. This library provides a complete implementation with support for multiple parameter sets and comprenhensive polynomial arithmetic operations.

## Features

- **Multiple Parameter Sets**: At the moment (1, july of 2025) this library supports for N values (509, 677, 701, 821, 1087, 1171, 1499) and q values (2048, 4096, 8192)
- **Polynomial Arithmetic**: Operations in Zp, Zq, and Z2 polynomial rings
- **Key Generation**: Automated generation of public/private key pairs with proper error handling
- **Encryption/Decryption**: Full encrypt/decrypt functionality
- **Performance Analysis**: Built-in timing and statistical analysis tools
- **File I/O**: Save and load keys and data in binary or text format
- **Memory Management**: Proper RAII with automatic cleanup

## Quick Start

### Basic Usage

```cpp
#include "NTRUencryption.hpp"

// Create an encryption instance (generates keys automatically)
NTRU::Encryption ntru;

// Encrypt a message
const char* message = "Hello world!";
NTRU::ZqPolynomial ciphertext = ntru.encrypt(message, strlen(message));

// Decrypt the message
NTRU::ZpPolynomial decrypted = ntru.decrypt(ciphertext);

// Convert back to bytes for your application
char result[1024];
decrypted.toBytes(result, true);
```

### Working with Keys

```cpp
// Save keys to files
ntru.saveKeys("public.key", "private.key");

// Load from existing key file
NTRU::Encryption ntru_from_file("private.key");

// Check if private key is available
if (ntru.validPrivateKeyAvailable()) {
    // Can encrypt and decrypt
} else {
    // Can only encrypt
}
```

- Use ``Encryption(const char* NTRUkeyFile)``; this will retrieve the keys from a binary file.
   1. If ``NTRUkeyFile`` represents a public key, the object will be created just with encryption capabilities.
   2. If ``NTRUkeyFile`` represents a private key, the object will have encryption and decryption capabilities.
   3. If the retrieved parameters [**N**, q] are different from the ones set by the program, an exception will be thrown.

### File Operations

```cpp
// Encrypt a file directly
NTRU::ZqPolynomial encrypted_file = ntru.encryptFile("document.txt");

// Work with plain text from files
NTRU::ZpPolynomial plaintext = NTRU::Encryption::ZpPolynomialPlainTextFromFile("message.txt");
NTRU::ZqPolynomial ciphertext = ntru.encrypt(plaintext);

// Save decrypted content as file (automatically handles end-of-content marker)
NTRU::ZpPolynomial decrypted = ntru.decrypt(ciphertext);
NTRU::Encryption::ZpPolynomialWriteFile(decrypted, "output.txt", false);
```

## Dependencies

- **GMP (GNU Multiple Precision Arithmetic Library)**: Required for arbitrary precision arithmetic
- **C++11 or later**: Uses modern C++ features

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install libgmp-dev libgmpxx4ldbl
```

**Fedora**
```bash
sudo dnf install gmp gmp-devel
```

**macOS (Homebrew):**
```bash
brew install gmp
```

**Windows (vcpkg):**
```bash
vcpkg install gmp:x64-windows
```

##  Building

### Before doing anything, I am assuming:

1. You have installed GNU ``g++`` compiler.
2. The command-line interface software GNU ``Make`` is installed in your computer.

### Use make commands

```bash
mkdir build && cd build
make
```

### Manual Compilation

```bash
g++ -std=c++11 -O3 -lgmp -lgmpxx your_program.cpp -o your_program
```

### Build Apps

Run ``make help`` for complete reference.

1. ``make basic_example`` to build basic example executable.
2. ``make NTRUencryption [arg1] [arg2]`` to build encryption executable with N=arg1 and q=arg2. The arguments are optional.
3. ``make performance_test [arg1] [arg2]`` to build performance test executable with N=arg1 and q=arg2. The arguments are optional.

### Executable Files
The executables are located in [Executables](Apps/Executables) directory.

## API Reference

### Core Classes

#### `NTRU::Encryption`
The main encryption class that handles key generation, encryption, and decryption.

**Constructors:**
- `Encryption()` - Creates new instance with auto-generated keys (throws `std::runtime_error`)
- `Encryption(const char* keyFile)` - Loads from existing key file (throws `std::runtime_error`)

**Core Methods:**
- `ZqPolynomial encrypt(const char bytes[], size_t size)` - Encrypt byte array
- `ZqPolynomial encrypt(const ZpPolynomial& poly)` - Encrypt polynomial directly
- `ZqPolynomial encryptFile(const char fileName[])` - Encrypt file contents
- `ZpPolynomial decrypt(const ZqPolynomial& cipher)` - Decrypt ciphertext
- `ZpPolynomial decrypt(const char bytes[], size_t size)` - Decrypt byte array
- `void saveKeys(const char* pubKey, const char* privKey)` - Save keys to files
- `bool validPrivateKeyAvailable()` - Check if private key is available

**Size Information:**
- `static size_t inputPlainTextMaxSizeInBytes()` - Maximum input plaintext size
- `static size_t outputPlainTextMaxSizeInBytes()` - Maximum output plaintext size
- `static size_t cipherTextSizeInBytes()` - Ciphertext size
- `static size_t privateKeySizeInBytes()` - Private key size
- `static size_t publicKeySizeInBytes()` - Public key size

**Static Utilities:**
- `static ZpPolynomial ZpPolynomialFromBytes(const char[], size_t, bool isPrivateKey)` - Create polynomial from bytes
- `static ZpPolynomial ZpPolynomialPlainTextFromFile(const char* fileName)` - Load plaintext from file
- `static void ZpPolynomialtoBytes(const ZpPolynomial&, char[], bool isPrivateKey)` - Convert polynomial to bytes
- `static void ZpPolynomialPlainTextSave(const ZpPolynomial&, const char*, bool saveAsText)` - Save plaintext polynomial
- `static void ZpPolynomialWriteFile(const ZpPolynomial&, const char*, bool writeAsText)` - Write polynomial content to file

#### `NTRU::ZpPolynomial`
Represents polynomials in Zp[x]/(x^N-1) with coefficients in {-1, 0, 1}.

**Key Methods:**
- `static ZpPolynomial randomTernary()` - Generate random ternary polynomial
- `static ZpPolynomial getPosiblePrivateKey()` - Generate potential private key
- `mpz_class toNumber()` - Convert to base-3 number representation
- `void print(const char* name, bool centered, const char* tail)` - Print polynomial

#### `NTRU::ZqPolynomial`
Represents polynomials in Zq[x]/(x^N-1) for ciphertext operations.

**Key Methods:**
- `ZqPolynomial(const char data[], int dataLength)` - Construct from byte array
- `static ZqPolynomial timesThree(const ZpPolynomial& p)` - Multiply ZpPolynomial by 3
- `void toBytes(char dest[])` - Convert to byte array
- `void save(const char* name, bool saveAsText)` - Save to file
- `static ZqPolynomial fromFile(const char* fileName)` - Load from file
- `static int log2(NTRU_q q)` - Calculate log base 2 of q parameter

#### `NTRU::Z2Polynomial`
Represents polynomials in Z2[x]/(x^N-1) for internal computations.

**Key Methods:**
- `Z2Polynomial(const ZpPolynomial& P)` - Construct from ZpPolynomial
- `void division(const Z2Polynomial& P, Z2Polynomial res[2])` - Polynomial division
- `Z2Polynomial gcdXNmns1(Z2Polynomial& thisBezout)` - GCD with x^N-1

### Global Utilities

**Convolution Operations:**
- `ZqPolynomial convolutionZq(const Z2Polynomial&, const ZpPolynomial&)`
- `ZqPolynomial convolutionZq(const Z2Polynomial&, const ZqPolynomial&)`  
- `ZqPolynomial convolutionZq(const ZpPolynomial&, const ZqPolynomial&)`

**Helper Functions:**
- `ZpPolynomial mods_p(ZqPolynomial)` - Modular reduction to Zp
- `int get_N()` - Get current N parameter
- `int get_q()` - Get current q parameter
- `void displayByteArrayBin(const char[], size_t)` - Debug: display bytes in binary
- `void displayByteArrayChar(const char[], size_t)` - Debug: display bytes as characters

### Parameter Configuration

The library supports the following parameters:

```
N = 509, 677, 701, 821, 1087, 1171, 1499
q = 2048, 4096, 8192
```

Expected security for some parameter combinations:

| N    | q    | Security Level |
|------|------|----------------|
| 509  | 2048 | ~112 bits      |
| 677  | 2048 | ~128 bits      |
| 701  | 8192 | ~192 bits      |
| 821  | 4096 | ~256 bits      |

## Performance Analysis

The library includes built-in performance measurement tools:

```cpp
// Measure key generation time
auto keyGenStats = NTRU::Encryption::Statistics::Time::keyGeneration();
std::cout << "Key generation average: " << keyGenStats.getAverage() << " ms" << std::endl;
std::cout << "Key generation variance: " << keyGenStats.getVariance() << " ms" << std::endl;
std::cout << "Average absolute deviation: " << keyGenStats.getAAD() << " ms" << std::endl;

// Measure encryption/decryption performance
NTRU::ZpPolynomial message = NTRU::Encryption::ZpPolynomialFromBytes("test", 4, false);
auto encStats = NTRU::Encryption::Statistics::Time::ciphering(&ntru, &message);
auto decStats = NTRU::Encryption::Statistics::Time::deciphering(&ntru, &ciphertext);

// Analyze ciphertext entropy and randomness
auto dataStats = NTRU::Encryption::Statistics::Data::encryption(&ntru);
std::cout << "Ciphertext entropy: " << dataStats.getEntropy() << std::endl;
std::cout << "Chi-square test: " << dataStats.getXiSquare() << std::endl;
std::cout << "Correlation coefficient: " << dataStats.getCorrelation() << std::endl;
```

## Binary files for ntru keys

As you can notice, constructor ``Encryption(const char* NTRUkeyFile)`` accept a string representing the name of a file.
This file, when reeding in binary mode, must have a particular format in order to obtain the oportunity to succeed in
the creation of the ``Encryption`` object. This format is the same used in the ``Encryption`` member function
``void saveKeys(const char publicKeyName[] = NULL, const char privateKeyName[] = NULL) const;`` to save the
criptographic keys. Formats for the key files are:

- Public key:
  1. The first 13 bytes corresponde to the string (with no line ending) "NTRUpublicKey".
  2. Next two bytes (byte 14 to byte 15) represents the parameter N.
  3. Next two bytes (byte 16 to byte 17) represents the parameter q.
  4. Denoting the size in bytes of public key as publicKeySizeInBytes, then the following publicKeySizeInBytes bytes
     represent the coefficients of the polynomial in the ring Zq[x]/(x^N-1).

- Private key:
  1. The first 13 bytes corresponde to the string (with no line ending) "NTRUprivatKey".
  2. Next two bytes (byte 14 to byte 15) represents the parameter N.
  3. Next two bytes (byte 16 to byte 17) represents the parameter q.
  4. Denoting the size in bytes of private key as privateKeySizeInBytes, then the following privateKeySizeInBytes bytes
     represent the coefficients of the polynomial in its respective ring Zp[x]/(x^N-1).

The idea under this format and ``saveKeys`` function is to ensure the key reatrived from any of this files fulfils all
the necessary requirements for a NTRU key.

## Security Considerations

At this moment (15, july of 2025):

- **Parameter Selection**: Use recommended parameter sets for your security requirements
- **Key Storage**: Store private keys securely and never transmit them
- **Random Number Generation**: This implementation uses `C++` standard library for random number generation
- **Side-Channel Attacks**: This implementation may be vulnerable to timing attacks in hostile environments
- **Padding process**: Paddgin process consist just in fillig with zeros.
- **Truncated data**: If the input data exceeds the maximum size for plaint text, then the intput is truncated prior to processing
- **Error Handling**: All constructors and file operations can throw `std::runtime_error` - handle appropriately

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## References

The first appereance of this cryptographic system can be found in the article [NTRU: A Ring Based Public Key Cryptosystem
](https://web.archive.org/web/20071021011338/http://www.ntru.com/cryptolab/pdf/ANTS97.pdf), but better resources can
be found in [ntru.org](https://ntru.org/). It is worth mentioning that NTRU became a finalist in the third round of [NIST's Post-Quantum Cryptography Standardization
](https://csrc.nist.gov/projects/post-quantum-cryptography/post-quantum-cryptography-standardization)
project. For a depper understanding of this cryptosystem, I strongly recomend the to
look at the books listed below:

- [An Introduction to Mathematical Cryptography](https://link.springer.com/book/10.1007/978-1-4939-1711-2)
- [Cryptography: Theory and Practice
  ](https://www.google.com.mx/books/edition/Cryptography/cJuDDwAAQBAJ?hl=en&gbpv=1&printsec=frontcover)
- [Criptografía. Temas selectos](https://www.alfaomegaeditor.com.mx/default/criptografia-temas-selectos-10825.html)
