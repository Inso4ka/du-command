#include <iostream>
#include <filesystem>
#include "../shared/log.hpp"

class DiskAnalyzer {
public:
    DiskAnalyzer(const std::filesystem::path& path) : rootPath(path) {}

    void analyze() {
        if (std::filesystem::is_directory(rootPath)) {
            processDirectory(rootPath);
        } else {
            processFile(rootPath);
        }
    }

private:
    void processFile(const std::filesystem::path& filePath) {
        try {
            LOG_INFO(getFileSizeInfo(filePath));
        } catch (const std::filesystem::filesystem_error& e) {
            LOG_ERROR("Error: ", e.what());
        }
    }

    void processDirectory(const std::filesystem::path& directoryPath) {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                if (entry.is_directory()) {
                    processDirectory(entry.path());
                } else {
                    processFile(entry.path());
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            LOG_ERROR("Error: ", e.what());
        }
    }

    std::string getFileSizeInfo(const std::filesystem::path& filePath) {
        std::uintmax_t blockSize = 512; // Size of each block in bytes
        std::uintmax_t fileSize = std::filesystem::file_size(filePath);
        std::uintmax_t fileBlocks = (fileSize + blockSize - 1) / blockSize; // Calculating the number of blocks for the file
        return std::to_string(fileBlocks) + "\t" + filePath.string();
    }

private:
    std::filesystem::path rootPath;
};

std::string buildCommand(int argc, char** argv) {
    if (argc == 2) {
        return std::string(argv[1]);
    } else if (argc == 3) {
        std::string option(argv[1]);
        if (option == "-m") {
            return "du -m " + std::string(argv[2]);
        } else if (option == "-g") {
            return "du -g " + std::string(argv[2]);
        } else {
            LOG_CRITICAL("Invalid option");
            exit(0);
        }
    } else {
        LOG_CRITICAL("No arguments provided. Please provide a directory path.");
        exit(0);
    }
}

int main(int argc, char** argv) {
    LOG_INIT();

    std::filesystem::path filePath = buildCommand(argc, argv);
    DiskAnalyzer analyzer(filePath);
    analyzer.analyze();

    return 0;
}
