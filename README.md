# NTRU Cryptosystem Implementation

A C++ implementation of the NTRU (Nth Degree Truncated Polynomial Ring) public key cryptosystem with comprehensive performance analysis and debugging tools.

*Mail me at aosorios1502@alumno.ipn.mx and alexis.fernando.osorio.sarabio@gmail.com for questions and comments.*

## Overview

NTRU is a lattice-based public key cryptosystem that offers post-quantum security. This implementation provides:
- Complete NTRU encryption and decryption functionality
- Performance benchmarking and statistical analysis tools
- Comprehensive debugging and visualization utilities
- Configurable parameter sets for different security levels

## Features
### Core Cryptographic Functions
- **Key Generation**: Automatic generation of NTRU public/private key pairs
- **Encryption**: Encrypt binary data and polynomial messages
- **Decryption**: Decrypt ciphertext back to original plaintext
- **File Operations**: Direct encryption/decryption of files
- **Key Persistence**: Save and load keys from files

### Performance Analysis
- **Timing Measurements**: Benchmark key generation, encryption, and decryption operations
- **Statistical Analysis**: Calculate variance, average absolute deviation, and other dispersion metrics
- **Randomness Testing**: Evaluate entropy, chi-square, and correlation of encrypted data

### Debugging and Utilities
- **Polynomial Visualization**: Print polynomials in various formats
- **Byte Array Display**: View data in binary, hexadecimal, or character format
- **Difference Analysis**: Compare vectors and highlight differences

## Directory Structure

```
include/
├── ntru/
│   ├── encryption.hpp           # Main encryption/decryption interface
│   ├── polynomials.hpp         # Polynomial arithmetic (Rp, Rq, R2)
│   ├── parameters_constants.hpp # NTRU parameter definitions
│   └── exceptions.hpp          # Custom exception classes
├── metrics-analysis/
│   ├── statistical_measures.hpp # Statistical analysis tools
│   ├── timing.hpp              # Performance timing utilities
│   └── data_generation.hpp     # Test data generation
└── print_debug/
    ├── print_array_table.hpp   # Array printing utilities
    └── debug_helpers.hpp       # Debug visualization tools
```

## Mathematical Background

### Polynomial Rings
The implementation works with three polynomial rings:
- **Rp = Z₃[x]/(x^N-1)**: Polynomials with coefficients in {-1, 0, 1}
- **Rq = Zq[x]/(x^N-1)**: Polynomials with coefficients modulo q
- **R2 = Z₂[x]/(x^N-1)**: Binary polynomials with coefficients in {0, 1}

### Key Generation
1. Generate random ternary polynomials f and g
2. Compute F = 1 + p·f then F⁻¹ in Rq (private key inverse)
3. Public key h = g × F⁻¹ (mod q)

### Encryption/Decryption
- **Encryption**: c = h × r + m (mod q), where r is random and m is the message
- **Decryption**: a = F × c (mod q), then recover m from a

## Configuration

### Compile-Time Parameters
Parameters are configured at compile time through preprocessor definitions:

```cpp
// Available parameter sets
enum ntru_N { _509_, _677_, _701_, _821_, _1087_, _1171_, _1499_ };
enum ntru_q { _2048_, _4096_, _8192_ };
```

Default configuration:
- **N = 701**: Polynomial degree
- **Q = 8192**: Coefficient modulus
- **p = 3**: Small modulus (fixed)

### Customization
Pass parameters to the compiler during build:
```bash
g++ -DNTRU_N=509 -DNTRU_Q=2048 your_files.cpp
```

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

### Performance Analysis
```cpp
// Measure key generation time
auto key_gen_stats = NTRU::Encryption::keyGenerationTime();
std::cout << "Average key generation: " << *key_gen_stats.getAverage() << " ms\n";

// Measure encryption time
NTRU::RpPolynomial message = NTRU::Encryption::randomTernary();
auto encrypt_stats = NTRU::Encryption::cipheringTime(ntru, message);

// Analyze data randomness
std::vector<std::byte> test_data = /* your data */;
StatisticalMeasures::DataRandomness randomness(test_data);
std::cout << "Entropy: " << *randomness.getEntropy() << "\n";
```

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

- **C++17 or later**: Required for modern C++ features
- **GMP/GMPXX**: GNU Multiple Precision Arithmetic Library (experimental features only)
- **Standard Library**: Uses `<vector>`, `<optional>`, `<functional>`, etc.

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

## Exception Handling

The library provides structured exception handling:

```cpp
try {
    NTRU::Encryption ntru;
    // ... operations
} catch (const NTRU::MathException& e) {
    // Mathematical operation failed (e.g., no inverse exists)
    std::cerr << e.what() << std::endl;
} catch (const NTRU::FileIOException& e) {
    // File operation failed
    std::cerr << e.what() << std::endl;
} catch (const NTRU::ParameterMismatchException& e) {
    // Parameter mismatch in loaded keys/data
    std::cerr << e.what() << std::endl;
} catch (const NTRU::InvalidPrivateKey& e) {
    // Private key validation failed
    std::cerr << e.what() << std::endl;
}
```

## Building

The project uses a comprehensive Makefile system for building the library, examples, and tests.

### Project Structure
```
project_root/
├── Makefile              # Root makefile (orchestrates entire build)
├── config.mk            # Configuration settings
├── mk/
│   └── common.mk        # Common build functions
├── src/
│   ├── Makefile         # Builds the main NTRU library
│   ├── ntru/            # Core NTRU implementation
│   ├── metrics-analysis/ # Performance analysis code
│   └── print_debug/     # Debug utilities
├── examples/
│   ├── Makefile         # Builds example programs
│   ├── basic-usage/     # Basic usage examples
│   └── file-encryption/ # File encryption examples
└── tests/
    ├── Makefile         # Builds and runs tests
    └── performance/     # Performance test suite
```

### Quick Start
```bash
# Build everything (library + examples + tests)
make all

# Build only the library
make lib

# Build examples (requires library)
make examples

# Build tests (requires library)  
make test

# Clean all build artifacts
make clean

# Show help
make help
```

### Advanced Building

#### Custom NTRU Parameters
Configure parameters in `config.mk` or pass them directly:
```bash
# Build with N=509, Q=2048
make NTRU_N=509 NTRU_Q=2048

# Build optimized version
make CXXFLAGS="-O3 -DNDEBUG"
```

#### Building Specific Components
```bash
# Build only basic usage examples
make -C examples basic

# Build only file encryption examples  
make -C examples file

# Build only performance tests
make -C tests performance
```

#### Library Output
The build system generates:
- **Static Library**: `build/lib/libntru.a`
- **Example Executables**: `build/bin/basic-usage/`, `build/bin/file-encryption/`
- **Test Executables**: `build/bin/performance/`

### Manual Building (Without Makefiles)

If you prefer to build manually or integrate into your own build system:

#### Core Library Compilation
```bash
# Create build directories
mkdir -p build/lib build/obj

# Compile NTRU core files (with parameters)
g++ -std=c++17 -fPIC -DNTRU_N=701 -DNTRU_Q=8192 -I./include \
    -c src/ntru/encryption.cpp -o build/obj/encryption.o
g++ -std=c++17 -fPIC -DNTRU_N=701 -DNTRU_Q=8192 -I./include \
    -c src/ntru/polynomials.cpp -o build/obj/polynomials.o
g++ -std=c++17 -fPIC -DNTRU_N=701 -DNTRU_Q=8192 -I./include \
    -c src/ntru/exceptions.cpp -o build/obj/exceptions.o

# Compile analysis and debug utilities (no parameters needed)
g++ -std=c++17 -fPIC -I./include \
    -c src/metrics-analysis/statistical_measures.cpp -o build/obj/statistical_measures.o
g++ -std=c++17 -fPIC -I./include \
    -c src/metrics-analysis/timing.cpp -o build/obj/timing.o
g++ -std=c++17 -fPIC -I./include \
    -c src/print_debug/debug_helpers.cpp -o build/obj/debug_helpers.o

# Create static library
ar rcs build/lib/libntru.a build/obj/*.o
```

#### Example Application
```bash
# Compile your application
g++ -std=c++17 -I./include your_app.cpp build/lib/libntru.a -lgmp -lgmpxx -o your_app

# With custom parameters
g++ -std=c++17 -DNTRU_N=509 -DNTRU_Q=2048 -I./include \
    your_app.cpp build/lib/libntru.a -lgmp -lgmpxx -o your_app
```

#### Optimization Flags
```bash
# Release build with optimizations
g++ -std=c++17 -O3 -DNDEBUG -march=native -I./include \
    your_app.cpp build/lib/libntru.a -lgmp -lgmpxx -o your_app

# Debug build with symbols
g++ -std=c++17 -g -O0 -DDEBUG -I./include \
    your_app.cpp build/lib/libntru.a -lgmp -lgmpxx -o your_app
```

### Build Configuration

The build system supports several configuration options that can be set in `config.mk`:

- **NTRU_N**: Polynomial degree (default: 701)
- **NTRU_Q**: Coefficient modulus (default: 8192)  
- **BUILD_DIR**: Build output directory (default: build)
- **CXX**: C++ compiler (default: g++)
- **CXXFLAGS**: Compiler flags
- **LDFLAGS**: Linker flags

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

## Limitations

- **Parameter Validation**: No compile-time parameter validation nor runtime check for parameter compatibility
- **GMP Integration**: GMP-dependent features are experimental
- **Thread Safety**: Not thread-safe without external synchronization
- **Constant Time**: Operations may not be constant-time (timing attack vulnerable)

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
