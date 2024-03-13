#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

int main(int argc, char* argv[]) {
    std::map<std::string, std::string> args;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 < argc) { // Ensure value for flag exists
            args[argv[i]] = argv[i + 1];
        }
    }

    std::string keyString = args.find("-key") != args.end() ? args["-key"] : "nodes";
    std::string ivString = args.find("-iv") != args.end() ? args["-iv"] : "secure";
    std::string inputFile = args.find("-f") != args.end() ? args["-f"] : "secure";

    const char* outputHeaderFile = "tmp/Stub.h";

    std::ifstream file(inputFile, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << inputFile << std::endl;
        return 1;
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});

    std::ofstream headerFile(outputHeaderFile);
    if (!headerFile) {
        std::cerr << "Cannot open file: " << outputHeaderFile << std::endl;
        return 1;
    }

    headerFile << "#ifndef STUB_H\n#define STUB_H\n\n";
    headerFile << "#include <stddef.h>\n\n";
    headerFile << "char keyString[] = \"" << keyString << "\";\n";
    headerFile << "char ivString[] = \"" << ivString << "\";\n\n";
    
    headerFile << "unsigned char encryptedData[] = {\n    ";

    for (size_t i = 0; i < buffer.size(); ++i) {
        headerFile << "0x" << std::hex << (0xFF & static_cast<int>(buffer[i]));
        if (i != buffer.size() - 1) {
            headerFile << ", ";
        }
        if ((i + 1) % 12 == 0) { // Adjust for line length
            headerFile << "\n    ";
        }
    }

    headerFile << "\n};\n";
    headerFile << "size_t encryptedDataSize = sizeof(encryptedData);\n\n";
    headerFile << "#endif // STUB_H\n";

    return 0;
}
