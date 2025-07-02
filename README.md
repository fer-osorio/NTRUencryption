# NTRU Encryption Library

A C++ implementation of the NTRU lattice-based cryptographic algorithm, providing quantum-resistant encryption capabilities.
Mail me at aosorios1502@alumno.ipn.mx and alexis.fernando.osorio.sarabio@gmail.com for questions and comments.

# Overview

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

# Usage

## Encryption object
To create one instance of NTRU::Encryption, two options are available:

1. Use ``Encryption()``; this will set **N** and **q** and will create valid private and public keys.
2. Use ``Encryption(const char* NTRUkeyFile)``; this will retrieve **N**, **q** and keys from a binary file.
   1. If ``NTRUkeyFile`` represents a public key, the object will be created just with encryption capabilities.
   2. If ``NTRUkeyFile`` represents a private key, the object will have encryption and decryption capabilities.

### Binary files for ntru keys

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

#  Building

## Before doing anything, I am assuming:

1. You have installed GNU ``g++`` compiler.
2. The command-line interface software GNU ``Make`` is installed in your computer.

## Use make commands

Use ``make`` commands to build the executables.

1. ``make NTRUencryption.exe`` to build executable for encryption.
2. ``make NTRUdecryption.exe`` to build executable for decryption.
3. ``make Statistics.exe`` to build executable for statistics.
4. ``make`` to build both.

Optionally, you can run the following commands on your terminal (command prompt on Windows)

For NTRUencryption.exe:
```
# This is, literally, the command that "make NTRUencryption.exe" calls.
g++ -o Apps/Executables/NTRUencryption.exe -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors -ggdb -fno-omit-frame-pointer -O2 -std=c++2a
Apps/encryption.cpp Apps/Settings.cpp Source/*.cpp
```

For NTRUdecryption.exe:
```
# This is, literally, the command that "make NTRUdecryption.exe" calls.
g++ -o Apps/Executables/NTRUdecryption.exe -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors -ggdb -fno-omit-frame-pointer -O2 -std=c++2a
Apps/decryption.cpp Apps/Settings.cpp Source/*.cpp
```

For Statistics.exe:
```
# This is, literally, the command that "make Statistics.exe" calls.
g++ -o Apps/Executables/Statistics.exe -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors -ggdb -fno-omit-frame-pointer -O2 -std=c++2a
Apps/Statistics.cpp Source/*.cpp
```

These last two commands are convenient if you do not have ``make`` installed.

# Executable Files

The executables are located in [Executables](Apps/Executables) directory. Instructions for passing arguments to them can be found
[here](Apps/Executables/README.md)

## API Reference

### Core Classes

#### `NTRU::Encryption`
The main encryption class that handles key generation, encryption, and decryption.

**Constructors:**
- `Encryption()` - Creates new instance with auto-generated keys
- `Encryption(const char* keyFile)` - Loads from existing key file

**Methods:**
- `ZqPolynomial encrypt(const char bytes[], int size)` - Encrypt byte array
- `ZpPolynomial decrypt(const ZqPolynomial& cipher)` - Decrypt ciphertext
- `void saveKeys(const char* pubKey, const char* privKey)` - Save keys to files
- `size_t plainTextMaxSizeInBytes()` - Maximum plaintext size
- `size_t cipherTextSizeInBytes()` - Ciphertext size

#### `NTRU::ZpPolynomial`
Represents polynomials in Zp[x]/(x^N-1) with coefficients in {-1, 0, 1}.

#### `NTRU::ZqPolynomial`
Represents polynomials in Zq[x]/(x^N-1) for ciphertext operations.

#### `NTRU::Z2Polynomial`
Represents polynomials in Z2[x]/(x^N-1) for internal computations.

### Parameter Configuration

The library supports these parameter combinations:

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

// Analyze ciphertext entropy
auto dataStats = NTRU::Encryption::Statistics::Data::encryption(&ntru);
std::cout << "Ciphertext entropy: " << dataStats.getEntropy() << std::endl;
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Security Considerations

At this moment (1, july of 2025):

- **Parameter Selection**: Use recommended parameter sets for your security requirements
- **Key Storage**: Store private keys securely and never transmit them
- **Random Number Generation**: This implementation uses `C++` standard library for random number generation
- **Side-Channel Attacks**: This implementation may be vulnerable to timing attacks in hostile environments
- **Padding process**: Paddgin process consist just in fillig with zeros.
- **Truncated data**: If the input data exceeds the maximum size for plaint text, then the intput is truncated prior to processing

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
