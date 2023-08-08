#pragma once

#include <iostream>
#include <filesystem>
#include "../shared/log.hpp"
#include <fstream>

struct conditions
{
    bool size     = false;
    bool showData = true;
    bool bytes    = false;
    bool list     = false;
    bool showAll  = false;
};

class DiskAnalyzer
{
  public:
    DiskAnalyzer(const std::filesystem::path& path, const conditions& condition);
    void analyze();

  private:
    void        processFile(const std::filesystem::path& filePath, std::uintmax_t& totalBlocks);
    void        processDirectory(const std::filesystem::path& directoryPath, std::uintmax_t& totalBlocks);
    std::string getFileSizeInfo(const std::filesystem::path& filePath, const std::uintmax_t size) const;
    void        processDirectoryWithAllContent(const std::filesystem::path& directoryPath, std::uintmax_t& totalBlocks);

  private:
    std::filesystem::path m_rootPath;
    conditions            m_condition;
};

std::vector<std::filesystem::path> readPathsFromFile(const std::string& filePath);
std::string                        buildCommand(int argc, char** argv, conditions& condition);