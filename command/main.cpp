#include <iostream>
#include <filesystem>
#include "../shared/log.hpp"

class DiskAnalyzer
{
  public:
    DiskAnalyzer(const std::filesystem::path& path) : m_rootPath(path) { }

    void analyze()
    {
        std::uintmax_t totalBlocks = 0;
        if (std::filesystem::is_directory(m_rootPath)) {
            processDirectory(m_rootPath, totalBlocks);
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

std::string buildCommand(int argc, char** argv, bool& size, bool& showData, bool& bytes)
{
    if (argc == 2) {
        return std::string(argv[1]);
    } else if (argc == 3) {
        std::string option(argv[1]);
        if (option == "-b") {
            bytes = true;
            return std::string(argv[2]);

        } else if (option == "-s") {
            showData = false;
            size     = true;
            return std::string(argv[2]);
        } else if (option == "-c") {
            size = true;
            return std::string(argv[2]);
        }
    }
    return "";
}

int main(int argc, char** argv)
{
    bool size     = false;
    bool showData = true;
    bool bytes    = false;

    std::string command = buildCommand(argc, argv, size, showData, bytes);
    if (command.empty()) {
        LOG_CRITICAL("Usage: du [-m|-g|-s|-b] <path>");
        return 1;
    }

    DiskAnalyzer analyzer(command);
    analyzer.setFlags(size, showData, bytes);
    analyzer.analyze();

    return 0;
}