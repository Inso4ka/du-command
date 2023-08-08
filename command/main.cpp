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
        if (m_size) {
            std::cout << "Total size: " << totalBlocks << " blocks" << std::endl;
        }
    }

    void setFlags(bool size, bool showData)
    {
        m_size     = size;
        m_showData = showData;
    }

  private:
    void processFile(const std::filesystem::path& filePath, std::uintmax_t& totalBlocks)
    {
        try {
            std::uintmax_t fileSize = std::filesystem::file_size(filePath);
            std::uintmax_t blocks   = (fileSize + 511) / 512; // Размер файла в блоках
            totalBlocks += blocks;
            if (m_showData) {
                LOG_INFO(getFileSizeInfo(filePath, blocks));
            }

        } catch (const std::filesystem::filesystem_error& e) {
            LOG_ERROR("Error 1 ", e.what());
        }
    }

    void processDirectory(const std::filesystem::path& directoryPath, std::uintmax_t& totalBlocks)
    {
        try {
            if (std::filesystem::is_empty(directoryPath)) {
                totalBlocks += 1; // Учитываем пустую папку размером в 1 блок
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

    std::string getFileSizeInfo(const std::filesystem::path& filePath, const std::uintmax_t blocks) const
    {
        std::uintmax_t fileSize = std::filesystem::file_size(filePath);
        return std::to_string(blocks) + "\t" + filePath.string();
    }

  private:
    std::filesystem::path m_rootPath;
    bool                  m_size;
    bool                  m_showData;
};

std::string buildCommand(int argc, char** argv, bool& size, bool& showData)
{
    if (argc == 2) {
        return std::string(argv[1]);
    } else if (argc == 3) {
        std::string option(argv[1]);
        if (option == "-m") {
            return "du -m " + std::string(argv[2]);
        } else if (option == "-g") {
            return "du -g " + std::string(argv[2]);
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

    std::string command = buildCommand(argc, argv, size, showData);
    if (command.empty()) {
        LOG_CRITICAL("Usage: du [-m|-g|-s|-b] <path>");
        return 1;
    }

    DiskAnalyzer analyzer(command);
    analyzer.setFlags(size, showData);
    analyzer.analyze();

    return 0;
}