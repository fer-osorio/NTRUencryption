# Passing arguments to the executables.
**Important**: Right now I am assuming you have a pair of public and private key available; if you do not, please run the
encryption executable with no arguments:
```
./NTRUencryption.exe
```
Then, the program will give you instructions for the creation of public-private key pairs (and other encryption options, if
desired). The following image shows the process of public-private key creation:

![Key Creation](../../Pictures/KeyCreation.gif)

We can pass arguments to the executables to encrypt/decrypt a single file. Set as first argument the name/path of the encryption
key we want to use followed by the files that are meant to be encrypted as the following arguments, this will encrypt/decrypt
the files using the key referenced in the first argument.

**Examples:**

Note: Videos were edited to decrease the size of these gifts.

1. Encrypting two ``.txt`` files by passing the relative paths of these files to the executable. In concrete, executing:
``./NTRUencryption.exe key.ntrupub FilesForTesting/Text00.txt FilesForTesting/Text01.txt``.

![Encryption](../../Pictures/NTRUencryption.gif)

2. Decrypting two text files by passing the relative paths of these files to the executable. In concrete, executing:
``./NTRUdecryption.exe key.ntruprv FilesForTesting/Text00.txt_encrypted.ntruq FilesForTesting/Text01.txt_encrypted.ntruq``.

![Decryption](../../Pictures/NTRUdecryption.gif)
