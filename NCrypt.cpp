#include <iostream>
#include <string>
#include <cstdlib>
#include <random>
#include <cstdio>

void generateRandomBytes(std::string& output, int length = 32) {
    const std::string chars =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    output.clear();
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> charDistribution(0, static_cast<int>(chars.size()) - 1);
    for (int i = 0; i < length; ++i) {
        output += chars[charDistribution(generator)];
    }
}

bool isExecutablePath(const std::string& command) {
    if (command.length() >= 4 && command.substr(command.length() - 4) == ".exe") {
        return true;
    }
    return false;
}

bool executeCommand(const std::string& command) {
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Command failed: " << command << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) { 
        std::cerr << "Usage: " << argv[0] << " -i <input>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[2];

    std::string outputFile;
    generateRandomBytes(outputFile, 64); 

    std::string key;
    generateRandomBytes(key, 32); 

    std::string iv;
    generateRandomBytes(iv, 32); 

    std::cout << "Encrypting File..." << std::endl;
    std::string sfedCmd = "tools\\SFED.exe -key \"" + key + "\" -iv \"" + iv + "\" -i \"" + inputFile + "\" -o \"" + outputFile + "\" -m encrypt";
    executeCommand(sfedCmd);

    std::cout << "Generating Binary Data Header File..." << std::endl;
    std::string genHeaderCmd = "tools\\GenerateHeader.exe -key \"" + key + "\" -iv \"" + iv + "\" -f \"" + outputFile + "\"";
    if(executeCommand(genHeaderCmd) != 0){

        printf("Created %o\n", outputFile);

        std::cout << "Compiling Final Program..." << std::endl;
        std::string compileCmd = "tcc\\tcc.exe Stub.c include\\aes\\aes.c -o out\\ncrypt-" + outputFile + ".exe";
        if(executeCommand(compileCmd) != 0){
            
            printf("Final PE created: out/ncrypt-%s.exe\n", outputFile.c_str());

            if (remove("tmp/Stub.h") != 0) {
                printf("Could not delete temporary binary data header file: tmp/Stub.h\n");
            } else {
                printf("Deleted tmp/Stub.h\n");
            }

            if(remove(outputFile.c_str()) != 0) {
                printf("Could not delete encrypted tmp file: %s\n", outputFile);
            } else {
                printf("Deleted: %s\n", outputFile.c_str());
            }

            // // test the crypt
            // std::string testCmd = "out\\ncrypt-" + outputFile + ".exe";
            // if(executeCommand(testCmd) != 0){
            
            //     printf("Final PE test & ran successfully...");

            // } else {
            //     printf("ERROR: Final PE test run failed.");
            // }

        } else {
            printf("Could not create final file... out/ncrypt-%s.exe\n", outputFile.c_str());
        }
    } else {
        printf("Could not create header file...");
    }
    
    return 0;
}