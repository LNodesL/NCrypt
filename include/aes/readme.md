# aes

This is a mini, portable aes package that lets us encrypt & decrypt without needing external dependencies. 

Simply enable 1 AES definition and the associated blocklen. You can change this, but be aware this program is only tested with AES 256:
```
// #define AES128 1
//#define AES192 1
#define AES256 1

// #define AES_BLOCKLEN 16 // Block length in bytes - 128 block only
// #define AES_BLOCKLEN 24 // Block length in bytes - 192 block only
#define AES_BLOCKLEN 32 // Block length in bytes - 256 block only
```

