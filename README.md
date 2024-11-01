# NTRUencryption

NTRU (Nth-degree Truncated Polynomial Ring Unit) implementation in C++, applied to the encryption of text files and
binary files.

Mail me at aosorios1502@alumno.ipn.mx and alexis.fernando.osorio.sarabio@gmail.com for questions and comments.

# Overview

Implementation of the NTRU, a public-key lattice-based cryptosystem resistant to attacks using Shor's alrithm. Due to
this and other reasons, NTRU became a finalist in the third round of [NIST's Post-Quantum Cryptography Standardization
](https://csrc.nist.gov/projects/post-quantum-cryptography/post-quantum-cryptography-standardization)
project.

Its first appereance can be found in the article [NTRU: A Ring Based Public Key Cryptosystem
](https://web.archive.org/web/20071021011338/http://www.ntru.com/cryptolab/pdf/ANTS97.pdf), but better resources can
be found in [ntru.org](https://ntru.org/). For a depper understanding of this cryptosystem, I strongly recomend the to
look at the books listed below:

- [An Introduction to Mathematical Cryptography](https://link.springer.com/book/10.1007/978-1-4939-1711-2)
- [Cryptography: Theory and Practice
  ](https://www.google.com.mx/books/edition/Cryptography/cJuDDwAAQBAJ?hl=en&gbpv=1&printsec=frontcover)
- [Criptografía. Temas selectos](https://www.alfaomegaeditor.com.mx/default/criptografia-temas-selectos-10825.html)

At the moment (31, october of 2024), the parameter accepted by this library are

- For **N**: <pre>    509;  677;  701;  821;  1087;  1171;  1499.  </pre>
- For **q**: <pre>    2048,  4096,  8192.  </pre>
- For **p**: <pre>    3.  </pre>

## Important notes

At this moment (31, october of 2024):

- No attempt is made to generate secure cryptographic keys.
- Paddgin process consist just in fillig with zeros.
- If the input data exceeds the maximum size for plaint text, then the intput is truncated prior to processing

# Usage

## Encryption object
To create one instance of NTRU::Encryption, two options are available:

1. Use ``Encryption(NTRU_N, NTRU_q)``; this will set **N** and **q** and will create valid private and public keys.
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

### In a nutshell, to create a Encryption object

1. First option: Passing parameters you desire to implement to the constructor ``Encryption(NTRU_N, NTRU_q)``.
2. Second option: (Having a NTRUkeyFile) passing the name of the file to ``Encryption(const char* NTRUkeyFile)``.

**Important note:** The default constructor for Encryption class, ``Encryption()``, sets each coefficient of the keys
as zero, so is mandatory to not use this constructor for nothing more than type declaration.

## Encryption and decryption

Once we have a Encryption object, for encryption of an array named ``data`` with ``size`` bytes it is only necessary to
invoke the member function 

```
ZqPolynomial encrypt(const char bytes[] ,int size, bool showEncryptionTime = false) const;
```

Encryption and decryption process will succeed if and only if the Encryption objects used in each end have the
identical public and private keys.