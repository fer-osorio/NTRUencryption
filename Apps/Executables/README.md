#  Compilation

### Before doing anything, I am assuming:

1. You have installed GNU ``g++`` compiler.
2. The command-line interface software GNU ``Make`` is installed in your computer.

In order to check if you have ``g++`` available you can run:

- For Windows:
    - Open command prompt; one way to do this is searching *cmd* in Start menu.
    - Type ``g++ --version`` and press enter.

- For macOS and Linux:
    - Open terminal application.
    - Type ``g++ --version`` and press enter.

If you do not have this compiler installed, I strongly recommend you to install the GNU compiler collection (GCC). Installation
instructions can be found here:
[Linux](https://www.geeksforgeeks.org/how-to-install-gcc-compiler-on-linux/), 
[MacOS](https://cs.millersville.edu/~gzoppetti/InstallingGccMac.html),
[Windows](https://www.ibm.com/docs/en/devops-test-embedded/9.0.0?topic=overview-installing-recommended-gnu-compiler-windows).

To verify if you hame GNU ``make`` installed:

- For Linux and macOS:
    - Open terminal application.
    - Type ``make --version``

To install GNU ``make``, you can follow the instructions of the following links:
[Windows](https://stackoverflow.com/a/57042516), 
[MacOS](https://ipv6.rs/tutorial/macOS/GNU_Make/)

## Use make commands

Use ``make`` commands to build the executables.

1. ``make NTRUencryption.exe`` to build executable for encryption.
2. ``make NTRUdecryption.exe`` to build executable for decryption.
3. ``make`` to build both.

Optionally, you can run the following commands on your terminal (command prompt on Windows)

For NTRUencryption.exe:
```
# This is, literally, the command that "make NTRUencryption.exe" calls.
g++ -o NTRUencryption.exe -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors -ggdb -fno-omit-frame-pointer -O2 -std=c++2a
../encryption.cpp ../Settings.cpp ../../Source/*.cpp
```

For NTRUdecryption.exe:
```
# This is, literally, the command that "make NTRUdecryption.exe" calls.
g++ -o NTRUdecryption.exe -Wall -Weffc++ -Wextra -Wsign-conversion -pedantic-errors -ggdb -fno-omit-frame-pointer -O2 -std=c++2a
../decryption.cpp ../Settings.cpp ../../Source/*.cpp
```

These last two commands are convenient if you do not have ``make`` installed. 

## Passing arguments to the executables.
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
