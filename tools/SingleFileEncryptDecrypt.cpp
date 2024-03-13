#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstring> // For memcpy
#include <string>
#include <algorithm> // For std::min

#include "../include/aes/aes.hpp"

bool EncryptFile(const std::string& inputFile, const std::string& outputFile, const uint8_t* key, const uint8_t* iv) {
    std::ifstream file(inputFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Could not open input file for reading.\n";
        return false;
    }

    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    file.close();

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Could not open output file for writing.\n";
        return false;
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv); // Initialize the AES context with key and IV for CBC mode
    
    // Ensure the buffer size is a multiple of AES_BLOCKLEN for CBC mode
    size_t paddingLength = AES_BLOCKLEN - (buffer.size() % AES_BLOCKLEN);
    buffer.insert(buffer.end(), paddingLength, static_cast<uint8_t>(paddingLength)); // PKCS#7 padding

    AES_CBC_encrypt_buffer(&ctx, buffer.data(), buffer.size());

    outFile.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
    outFile.close();

    return true;
}

bool DecryptFile(const std::string& inputFile, const std::string& outputFile, const uint8_t* key, const uint8_t* iv) {
    std::ifstream file(inputFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Could not open input file for reading.\n";
        return false;
    }

    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    file.close();

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv); // Initialize the AES context with key and IV for CBC mode

    AES_CBC_decrypt_buffer(&ctx, buffer.data(), buffer.size());

    // Remove PKCS#7 padding
    uint8_t paddingLength = buffer.back();
    if(paddingLength > 0 && paddingLength <= AES_BLOCKLEN) {
        buffer.erase(buffer.end() - paddingLength, buffer.end());
    }

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Could not open output file for writing.\n";
        return false;
    }

    outFile.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
    outFile.close();

    return true;
}

int main(int argc, char* argv[]) {
    std::map<std::string, std::string> args;

    // Parsing command-line arguments
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 < argc) {
            args[std::string(argv[i])] = std::string(argv[i + 1]);
        } else {
            std::cerr << "Missing value for argument: " << argv[i] << std::endl;
            return 1;
        }
    }

    std::string keyString = args.find("-key") != args.end() ? args["-key"] : "";
    std::string ivString = args.find("-iv") != args.end() ? args["-iv"] : "";
    std::string runMode = args.find("-m") != args.end() ? args["-m"] : "encrypt";
    std::string input = args.find("-i") != args.end() ? args["-i"] : "";
    std::string output = args.find("-o") != args.end() ? args["-o"] : "";

    // Validate arguments
    if (keyString.empty() || ivString.empty() || input.empty() || output.empty()) {
        std::cerr << "Missing required arguments." << std::endl;
        return 1;
    }

    uint8_t key[AES_KEYLEN] = {0}; // Initialize key array with zeros
    uint8_t iv[AES_BLOCKLEN] = {0}; // Initialize iv array with zeros

    // Copy the key and IV from strings, respecting the library's size constraints
    memcpy(key, keyString.c_str(), std::min(keyString.size(), static_cast<size_t>(AES_KEYLEN)));
    memcpy(iv, ivString.c_str(), std::min(ivString.size(), static_cast<size_t>(AES_BLOCKLEN)));

    // Perform encryption or decryption based on runMode
    if(runMode == "encrypt") {
        if (!EncryptFile(input, output, key, iv)) {
            std::cerr << "Encryption failed." << std::endl;
            return 1;
        }
    } else if (runMode == "decrypt") {
        if (!DecryptFile(input, output, key, iv)) {
            std::cerr << "Decryption failed." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Invalid mode specified. Use -m encrypt or -m decrypt." << std::endl;
        return 1;
    }

    std::cout << "Operation completed successfully." << std::endl;
    return 0;
}
