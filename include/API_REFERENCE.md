# NTRU API Reference

This document provides detailed API documentation for the NTRU cryptosystem implementation. For general project information and building instructions, see the main [README.md](../README.md).

## Table of Contents

- [Core Cryptographic API](#core-cryptographic-api)
  - [Encryption Class](#encryption-class)
  - [Polynomial Classes](#polynomial-classes)
- [Performance Analysis API](#performance-analysis-api)
- [Debug and Utility API](#debug-and-utility-api)
- [Exception Handling](#exception-handling)
- [Constants and Parameters](#constants-and-parameters)

## Core Cryptographic API

### Encryption Class

`NTRU::Encryption` - Main interface for NTRU cryptographic operations.

#### Constructors

```cpp
// Generate new key pair automatically
NTRU::Encryption();

// Load from existing private key file
NTRU::Encryption(const char* NTRUkeyFile);
```

**Exceptions**: `std::runtime_error`, `NTRU::MathException`, `NTRU::FileIOException`, `NTRU::ParameterMismatchException`

#### Core Operations

```cpp
// Encrypt binary data
RqPolynomial encrypt(const char bytes[], size_t size) const;

// Encrypt polynomial message
RqPolynomial encrypt(const RpPolynomial& message) const;

// Encrypt file contents
RqPolynomial encryptFile(const char fileName[]) const;

// Decrypt to polynomial
RpPolynomial decrypt(const RqPolynomial& ciphertext) const;

// Decrypt binary data
RpPolynomial decrypt(const char bytes[], size_t size) const;
```

**Exceptions**: `NTRU::FileIOException` (file operations)

#### Key Management

```cpp
// Check if private key is available for decryption
bool validPrivateKeyAvailable() const;

// Save keys to files
void saveKeys(const char publicKeyName[] = NULL, 
              const char privateKeyName[] = NULL) const;

// Print keys to console
void printKeys(const char publicKeyName[] = NULL, 
               const char privateKeyName[] = NULL) const;
```

**Exceptions**: `NTRU::FileIOException` (save operations)

#### Static Utility Methods

```cpp
// Size information
static size_t inputPlainTextMaxSizeInBytes();
static size_t outputPlainTextMaxSizeInBytes();
static size_t cipherTextSizeInBytes();
static size_t privateKeySizeInBytes();
static size_t publicKeySizeInBytes();

// Polynomial conversion utilities
static RpPolynomial RpPolynomialFromBytes(const char bytes[], size_t size, bool isPrivateKey);
static RpPolynomial RpPolynomialPlainTextFromFile(const char* fileName);
static void RpPolynomialtoBytes(const RpPolynomial& org, char dest[], bool isPrivateKey);
static void RpPolynomialPlainTextSave(const RpPolynomial& org, const char* name, bool saveAsText = false);
static void RpPolynomialWriteFile(const RpPolynomial& org, const char* fileName, bool writeAsText);

// File I/O for encrypted data
static void RqPolynomialSave(const RqPolynomial& pl, const char* name, bool saveAsText = false);
static RqPolynomial RqPolynomialFromFile(const char* fileName);

// Random polynomial generation
static RpPolynomial randomTernary();
```

**Exceptions**: `std::runtime_error`, `NTRU::FileIOException`, `NTRU::ParameterMismatchException`

#### Performance Measurement

```cpp
// Timing analysis
static StatisticalMeasures::Dispersion<uint32_t> keyGenerationTime();
static StatisticalMeasures::Dispersion<uint32_t> cipheringTime(const Encryption& e, const RpPolynomial& msg);
static StatisticalMeasures::Dispersion<uint32_t> decipheringTime(const Encryption& e, const RqPolynomial& emsg);

// Randomness analysis
static StatisticalMeasures::DataRandomness encryptedDataRandomness(const Encryption& e, const std::vector<std::byte>& plain_data);
```

### Polynomial Classes

#### RpPolynomial - Ternary Polynomials (Z₃[x]/(x^N-1))

```cpp
class RpPolynomial {
public:
    enum Z3 { _0_ = 0, _1_ = 1, _2_ = 2 };
    
    // Constructors
    RpPolynomial();                              // Zero polynomial
    RpPolynomial(const RpPolynomial& P);         // Copy constructor
    
    // Operators
    RpPolynomial& operator=(const RpPolynomial& P);
    int operator[](int i) const;                 // Access coefficient
    
    // Utilities
    int degree() const;                          // Polynomial degree
    mpz_class toNumber() const;                  // Base-3 interpretation (experimental)
    void print(const char* name = "", bool centered = true, const char* tail = "") const;
    void println(const char* name = "", bool centered = true) const;
};
```

#### RqPolynomial - Large Modulus Polynomials (Zq[x]/(x^N-1))

```cpp
class RqPolynomial {
public:
    // Constructors
    RqPolynomial();
    RqPolynomial(const RqPolynomial& P);
    RqPolynomial(const char data[], int dataLength);  // From byte array
    
    // Operators
    RqPolynomial& operator=(const RqPolynomial& P);
    int64_t operator[](int i) const;
    RqPolynomial operator+(const RqPolynomial& P) const;
    RqPolynomial operator*(const RqPolynomial& P) const;
    
    // Utilities
    int degree() const;
    bool equalsOne() const;
    void mod_q() const;                          // Reduce coefficients mod q
    void mods_q() const;                         // Symmetric reduction mod q
    int lengthInBytes() const;
    void toBytes(char dest[]) const;
    void print(const char* name = "", const char* tail = "") const;
    void println(const char* name = "") const;
};
```

#### R2Polynomial - Binary Polynomials (Z₂[x]/(x^N-1))

```cpp
class R2Polynomial {
public:
    enum Z2 { _0_ = 0, _1_ = 1 };
    
    // Constructors
    R2Polynomial();
    R2Polynomial(const R2Polynomial& P);
    R2Polynomial(const RpPolynomial& P);         // Convert from ternary
    
    // Operators
    R2Polynomial& operator=(const R2Polynomial& P);
    R2Polynomial& operator=(const RpPolynomial& P);
    R2Polynomial& operator=(Z2 t);
    R2Polynomial operator+(const R2Polynomial&) const;  // XOR operation
    R2Polynomial operator-(const R2Polynomial&) const;  // Same as +
    R2Polynomial operator*(const R2Polynomial&) const;
    bool operator==(int t) const;
    bool operator==(const R2Polynomial& P) const;
    bool operator!=(int t) const;
    Z2 operator[](int i) const;
    
    // Mathematical operations
    void division(const R2Polynomial& P, R2Polynomial res[2]) const;
    R2Polynomial gcdXNmns1(R2Polynomial& thisBezout) const;
    
    // Utilities
    int degree() const;
    void print(const char* name = "", const char* tail = "") const;
    void println(const char* name = "") const;
};
```

#### Global Polynomial Operations

```cpp
// Convolution operations between different polynomial types
RqPolynomial convolutionRq(const R2Polynomial&, const RpPolynomial&);
RqPolynomial convolutionRq(const R2Polynomial&, const RqPolynomial&);
RqPolynomial convolutionRq(const RpPolynomial&, const RqPolynomial&);

// Modular reduction
RpPolynomial mods_p(const RqPolynomial&);

// Subtraction with scalar
RqPolynomial operator-(int64_t, const RqPolynomial&);
```

## Performance Analysis API

### Statistical Measures

#### Dispersion Analysis

```cpp
template <typename T>
class StatisticalMeasures::Dispersion {
public:
    explicit Dispersion(const std::vector<T>& data);
    
    // Getters (all return std::optional<double>)
    std::optional<double> getMaximum() const noexcept;
    std::optional<double> getMinimum() const noexcept;
    std::optional<double> getAverage() const noexcept;
    std::optional<double> getVariance() const noexcept;
    std::optional<double> getAAD() const noexcept;  // Average Absolute Deviation
};
```

#### Data Randomness Analysis

```cpp
class StatisticalMeasures::DataRandomness {
public:
    explicit DataRandomness(const std::vector<std::byte>& data);
    
    // Statistical measures (all return std::optional<double>)
    std::optional<double> getEntropy() const noexcept;           // Shannon entropy
    std::optional<double> getChiSquare() const noexcept;        // Chi-square test
    std::optional<double> getCorrelationAdjacentByte() const noexcept;
    
    // Custom correlation analysis
    std::optional<double> calculateCorrelation(const std::vector<std::byte>& data, size_t offset) const;
};
```

### Timing Utilities

```cpp
namespace Timing {
    enum struct TestIDnum { KEY_GENERATION, CIPHERING, DECIPHERING };
    
    struct TestID {
        explicit TestID(TestIDnum ID_num);
        TestIDnum get_ID_num() const;
        std::string get_label() const;
    };
    
    // Execute function multiple times and measure timing
    template<typename T> 
    std::vector<uint32_t> functionExecutionTiming(std::function<T()> func, TestID TID);
}
```

### Data Generation

```cpp
namespace DataGeneration {
    // Generate test data using custom generator function
    std::vector<std::byte> dataSource(std::function<std::byte(size_t)> generator, size_t size);
    
    // Generate simple test data
    std::vector<std::byte> simpleDataSource(size_t);
}
```

## Debug and Utility API

### Array Display Functions

```cpp
// Calculate hex representation length
size_t lenHexRep(int a);

// Print data in table format
template <typename T> 
void print_table(const T* data, size_t data_size, 
                 std::string_view title = "", std::string_view tail = "",
                 const unsigned int column_width = 5, const unsigned int columns = 16);
```

### Debug Helpers

#### Number Base Utilities

```cpp
struct NumberBaseTwosPower {
    enum struct Base { BINARY = 1, QUATERNARY = 3, OCTAL = 7, HEXADECIMAL = 15 };
    
    static size_t max_bit_amount(Base b);
    static unsigned char to_upper_bits(Base b);
};
```

#### Object and Array Visualization

```cpp
// Print object as byte array in specified base
template <typename T>
size_t print_obj_byte_view(const T& obj, NumberBaseTwosPower::Base b);

// Print slice of vector centered around index
template <typename T>
size_t print_slice_centered(const std::vector<T>& v, size_t at_index, 
                           size_t slice_width, NumberBaseTwosPower::Base b);

// Compare vectors and show first difference
int print_first_difference(std::vector<char> v1, std::vector<char> v2, 
                          std::string diff_msg, std::string v1_slice_front, 
                          std::string v2_slice_front, size_t width);
```

#### Byte Array Display

```cpp
// Display byte arrays with formatting
void displayByteArray(const char byteArray[], size_t size, 
                     size_t columnSize, const char format[]);
void displayByteArrayBin(const char byteArray[], size_t size);    // Binary format
void displayByteArrayChar(const char byteArray[], size_t size);   // Character format
```

## Exception Handling

### Base Exception Class

```cpp
class NTRU::NTRUexception : public std::runtime_error {
public:
    explicit NTRUexception(const std::string& what_arg);
    void add_trace_point(const std::string& trace_info);
    const char* what() const noexcept override;  // Returns full trace
};
```

### Specialized Exception Classes

```cpp
// Mathematical operation failures
class NTRU::MathException : public NTRUexception {
public:
    explicit MathException(const std::string& what_arg);
};

// File I/O errors
class NTRU::FileIOException : public NTRUexception {
public:
    explicit FileIOException(const std::string& what_arg);
};

// Parameter compatibility issues
class NTRU::ParameterMismatchException : public NTRUexception {
public:
    explicit ParameterMismatchException(const std::string& what_arg);
};

// Private key validation errors
class NTRU::InvalidPrivateKey : public NTRUexception {
public:
    explicit InvalidPrivateKey(const std::string& what_arg);
};
```

### Exception Usage Pattern

```cpp
try {
    // NTRU operations
} catch (const NTRU::MathException& e) {
    e.add_trace_point("Function: myFunction()");
    throw;  // Re-throw with added context
} catch (const NTRU::NTRUexception& e) {
    std::cerr << e.what() << std::endl;  // Prints full trace
}
```

## Constants and Parameters

### Parameter Enumerations

```cpp
namespace NTRU {
    // Available polynomial degrees
    enum ntru_N { _509_ = 509, _677_ = 677, _701_ = 701, _821_ = 821, 
                  _1087_ = 1087, _1171_ = 1171, _1499_ = 1499 };
    
    // Available modulus values
    enum ntru_q { _2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };
    
    // Utility function
    constexpr static int log2_q(ntru_q q);    // Returns log₂(q)
    constexpr int log2q;                      // Precomputed log₂(NTRU_Q)
}
```

### Compile-Time Configuration

These macros control the NTRU parameters:

```cpp
#ifndef NTRU_N
    #define NTRU_N 701        // Default polynomial degree
#endif

#ifndef NTRU_Q  
    #define NTRU_Q 8192       // Default modulus
#endif
```

## Usage Examples

### Basic Encryption/Decryption

```cpp
#include "ntru/encryption.hpp"

// Generate keys and encrypt
NTRU::Encryption ntru;
std::string message = "Secret message";
auto ciphertext = ntru.encrypt(message.c_str(), message.size());
auto plaintext = ntru.decrypt(ciphertext);
```

### Performance Analysis

```cpp
#include "metrics-analysis/statistical_measures.hpp"

// Analyze timing
auto timing_stats = NTRU::Encryption::keyGenerationTime();
if (timing_stats.getAverage().has_value()) {
    std::cout << "Average: " << *timing_stats.getAverage() << " ms\n";
}

// Analyze randomness
std::vector<std::byte> data = /* encrypted data */;
StatisticalMeasures::DataRandomness randomness(data);
if (randomness.getEntropy().has_value()) {
    std::cout << "Entropy: " << *randomness.getEntropy() << " bits\n";
}
```

### Debugging

```cpp
#include "print_debug/debug_helpers.hpp"

// Print polynomial coefficients
RpPolynomial poly = NTRU::Encryption::randomTernary();
poly.print("Random polynomial");

// Display byte array in hex
char data[] = {0x12, 0x34, 0xAB, 0xCD};
displayByteArray(data, sizeof(data), 8, "hex");
```

## Thread Safety

**Important**: This library is **not thread-safe**. If using in multithreaded applications:

- Use separate `NTRU::Encryption` instances per thread, or
- Implement external synchronization (mutexes, etc.)
- Static methods are not thread-safe either

## Memory Management

- All classes follow RAII principles
- Dynamic memory is automatically managed
- No manual cleanup required for normal usage
- Exception safety: operations are either successful or leave objects in valid state

## Performance Notes

- Key generation is the most expensive operation (~5-10ms with N = 701, q = 8192)
- Encryption/decryption are typically fast (~1ms with N = 701, q = 8192)
- Performance varies significantly with parameter choices
- Consider caching `Encryption` objects rather than recreating them
- File operations add I/O overhead to cryptographic operations
