#include "diskAnalyzer.hpp"

DiskAnalyzer::DiskAnalyzer(const std::filesystem::path& path, const conditions& condition) : m_rootPath(path), m_condition(condition) { }

void DiskAnalyzer::analyze()
{
    std::uintmax_t totalBlocks = 0;
    if (std::filesystem::is_directory(m_rootPath)) {
        if (m_condition.showAll) {
            processDirectoryWithAllContent(m_rootPath, totalBlocks);
        } else {
            processDirectory(m_rootPath, totalBlocks);
        }
    } else {
        processFile(m_rootPath, totalBlocks);
    }
    if (m_condition.size) {
        LOG_INFO(std::to_string(totalBlocks) + "\t" + m_rootPath.string());
    }
}

void DiskAnalyzer::processFile(const std::filesystem::path& filePath, std::uintmax_t& totalBlocks)
{
    try {
        std::uintmax_t fileSize = std::filesystem::file_size(filePath);
        if (m_condition.bytes) {
            LOG_INFO(getFileSizeInfo(filePath, fileSize));
        } else {
            std::uintmax_t blocks = (fileSize + 511) / 512;
            totalBlocks += blocks;
            if (m_condition.showData) {
                LOG_INFO(getFileSizeInfo(filePath, blocks));
            }
        }

    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Error 1 ", e.what());
    }
}

void DiskAnalyzer::processDirectory(const std::filesystem::path& directoryPath, std::uintmax_t& totalBlocks)
{
    try {
        std::uintmax_t directorySize = 0;
        if (std::filesystem::is_empty(directoryPath)) {
            totalBlocks += 1;
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                if (entry.is_directory()) {
                    std::uintmax_t subdirectorySize = 0;
                    processDirectory(entry.path(), subdirectorySize);
                    totalBlocks += subdirectorySize;
                    directorySize += subdirectorySize;
                } else {
                    directorySize += entry.file_size();
                }
            }
        }
        std::uintmax_t directoryBlocks = (directorySize + 511) / 512;
        totalBlocks += directoryBlocks;

        if (m_condition.showData) {
            std::string sizeInfo;
            if (m_condition.bytes) {
                sizeInfo = std::to_string(directorySize) + "\t";
            } else {
                sizeInfo = std::to_string(totalBlocks) + "\t";
            }
            LOG_INFO(sizeInfo + directoryPath.string());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Error 2 ", e.what());
    }
}

void DiskAnalyzer::processDirectoryWithAllContent(const std::filesystem::path& directoryPath, std::uintmax_t& totalBlocks)
{
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
            if (entry.is_directory()) {
                totalBlocks += 1;
            } else {
                processFile(entry.path(), totalBlocks);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Error 3 ", e.what());
    }
}

std::string DiskAnalyzer::getFileSizeInfo(const std::filesystem::path& filePath, const std::uintmax_t size) const
{
    std::uintmax_t fileSize = std::filesystem::file_size(filePath);
    return std::to_string(size) + "\t" + filePath.string();
}

std::vector<std::filesystem::path> readPathsFromFile(const std::string& filePath)
{
    std::vector<std::filesystem::path> paths;
    std::ifstream                      file(filePath);

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

std::string buildCommand(int argc, char** argv, conditions& condition)
{
    if (argc == 2) {
        return argv[1];
    } else if (argc == 3) {
        std::string option(argv[1]);
        if (option == "-b") {
            condition.bytes = true;
        } else if (option == "--files") {
            condition.list = true;
        } else if (option == "-s") {
            condition.showData = false;
            condition.size     = true;
        } else if (option == "-c") {
            condition.size = true;
        } else if (option == "-a") {
            condition.showAll = true;
        }
        return argv[2];
    }
    return "";
}