#include <iostream>
#include <filesystem>
#include "../shared/log.hpp"
#include <fstream>

class DiskAnalyzer
{
  public:
    DiskAnalyzer(const std::filesystem::path& path) : m_rootPath(path) { }

    void analyze()
    {
        std::uintmax_t totalBlocks = 0;
        if (std::filesystem::is_directory(m_rootPath)) {
            if (std::filesystem::is_empty(m_rootPath)) {
                totalBlocks += 1;
                LOG_INFO(std::to_string(totalBlocks) + "\t" + m_rootPath.string());
            } else {
                processDirectory(m_rootPath, totalBlocks);
            }
        } else {
            processFile(m_rootPath, totalBlocks);
        }
        if (flag.m_size) {
            LOG_INFO(std::to_string(totalBlocks) + "\t" + m_rootPath.string());
        }
    }

    void setFlags(bool size, bool showData, bool bytes)
    {
        flag.m_size     = size;
        flag.m_showData = showData;
        flag.m_bytes    = bytes;
    }

  private:
    void processFile(const std::filesystem::path& filePath, std::uintmax_t& totalBlocks)
    {
        try {
            std::uintmax_t fileSize = std::filesystem::file_size(filePath);
            if (flag.m_bytes) {
                LOG_INFO(getFileSizeInfo(filePath, fileSize));
            } else {
                std::uintmax_t blocks = (fileSize + 511) / 512;
                totalBlocks += blocks;
                if (flag.m_showData) {
                    LOG_INFO(getFileSizeInfo(filePath, blocks));
                }
            }

        } catch (const std::filesystem::filesystem_error& e) {
            LOG_ERROR("Error 1 ", e.what());
        }
    }

    void processDirectory(const std::filesystem::path& directoryPath, std::uintmax_t& totalBlocks)
    {
        try {
            if (std::filesystem::is_empty(directoryPath)) {
                totalBlocks += 1;
            } else {
                for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                    if (entry.is_directory()) {
                        processDirectory(entry.path(), totalBlocks);
                    } else {
                        processFile(entry.path(), totalBlocks);
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            LOG_ERROR("Error 2 ", e.what());
        }
    }

    std::string getFileSizeInfo(const std::filesystem::path& filePath, const std::uintmax_t size) const
    {
        std::uintmax_t fileSize = std::filesystem::file_size(filePath);
        return std::to_string(size) + "\t" + filePath.string();
    }

  private:
    std::filesystem::path m_rootPath;
    struct flags
    {
        bool m_size;
        bool m_showData;
        bool m_bytes;
    };
    flags flag;
};

std::vector<std::filesystem::path> readPathsFromFile(const std::string& filePath) {
    std::vector<std::filesystem::path> paths;
    std::ifstream file(filePath);

    if (!file) {
        LOG_CRITICAL("File error");
    } else {
        std::string line;
        while (std::getline(file, line)) {
            paths.emplace_back(line);
        }

        file.close();
    }

    return paths;
}

std::string buildCommand(int argc, char** argv, bool& size, bool& showData, bool& bytes, bool& list) {
    if (argc == 2) {
        return argv[1];
    } else if (argc == 3) {
        std::string option(argv[1]);
        if (option == "-b") {
            bytes = true;
            return argv[2];
        } else if (option == "--files") {
            list = true;
            return argv[2];
        } else if (option == "-s") {
            showData = false;
            size = true;
            return argv[2];
        } else if (option == "-c") {
            size = true;
            return argv[2];
        }
    }
    return "";
}

int main(int argc, char** argv) {
    bool size = false;
    bool showData = true;
    bool bytes = false;
    bool list = false;

    std::string command = buildCommand(argc, argv, size, showData, bytes, list);
    if (command.empty()) {
        LOG_CRITICAL("Usage: du [-m|-g|-s|-b] <path>");
        return 1;
    }

    if (list) {
        std::vector<std::filesystem::path> paths = readPathsFromFile(command);
        for (const auto& path : paths) {
            DiskAnalyzer analyzer(path);
            analyzer.setFlags(size, showData, bytes);
            analyzer.analyze();
        }
    } else {
        DiskAnalyzer analyzer(command);
        analyzer.setFlags(size, showData, bytes);
        analyzer.analyze();
    }

    return 0;
}

