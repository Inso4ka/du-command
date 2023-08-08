#include "diskAnalyzer.hpp"

int main(int argc, char** argv) {
    
    conditions condition;
    std::string command = buildCommand(argc, argv, condition);
    if (command.empty()) {
        LOG_CRITICAL("Usage: du [-m|-g|-s|-b] <path>");
        return 1;
    }

    if (condition.list) {
        std::vector<std::filesystem::path> paths = readPathsFromFile(command);
        for (const auto& path : paths) {
            DiskAnalyzer analyzer(path, condition);
            analyzer.analyze();
        }
    } else {
        DiskAnalyzer analyzer(command, condition);
        analyzer.analyze();
    }

    return 0;
}

