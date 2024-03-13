# NCrypt

NCrypt is my first crypter project, ever. Since starting this project I have started & released side projects, worked on prototypes, and even have another brand new similar codebase in the works. However, NCrypt holds its own!

I am releasing NCrypt as a starter kit for anyone wanting to protect their own programs, intellectual property, and data. NCrypt can help you learn the building blocks of how a crypter works, while being ready to use as-is too!

NCrypt is ideal if you don't have a dev setup because it contains a tiny C compiler inside of it. This means you can modify the Stub.c file whenever you want and the next time you use NCrypt.exe, it will use the new code from there!

## How to Use

Using NCrypt is simple:

```
./NCrypt.exe -i path/to/original.exe
```

This will automatically encrypt original.exe (temporarily added to tmp/ folder, and then removed), add the encrypted data to a header file (include/Stub.h)

## Development

### Preparation of Source Code

Although this crypter is built with portability in mind, you will need a C++ compiler if you are wanting to change the core tools (SFED, GenerateHeader, NCrypt itself). The benefit of tiny C compiler (tcc) is that you do not need a C++ (or any other) compiler to re-compile the Stub.c file.

AES Resource File:

```
gcc -c include/aes/aes.c -o include/aes/aes.o
```

SingleFileEncryptDecrypt / SFED:

```
g++ tools/SingleFileEncryptDecrypt.cpp include/aes/aes.o -o tools/SFED  -static
```

GenerateHeader:

```
g++ tools/GenerateHeader.cpp include/aes/aes.c -static -o tools/GenerateHeader
```

NCrypt:

```
g++ NCrypt.cpp include/aes/aes.c -static -o NCrypt
```

### Manual Crypt (Without NCrypt.exe)

This is not needed unless you wish to do it manually. You can use the NCrypt program instead.

1. Encrypt the exe

```
./tools/SFED -m "encrypt" -i "path/to/original.exe" -o "tmp/encryptedFile" -key secureKeyHere -iv validIvHere
```

2. Load array of bytes from encrypted exe into header file

```
./tools/GenerateHeader -key secureKeyHere -iv validIvHere -f tmp/encryptedFile
```

3. Create crypted PE file

```
./tcc/tcc.exe Stub.c include/aes/aes.c -o out/ncrypt-final.exe
```
