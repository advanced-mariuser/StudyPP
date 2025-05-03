#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sys/wait.h>
#include <unistd.h>

class Extractor
{
public:
    Extractor(std::string archiveName, std::string outputFolder);

    void RunSequential();

    void RunParallel(int numProcesses);

private:
    std::string m_archiveName;
    std::string m_outputFolder;
    std::vector<std::string> m_compressedFiles;
    std::vector<pid_t> m_children;

    void ExtractArchive();

    void FindCompressedFiles();

    static void DecompressFile(const std::string &file);

    void SpawnChild(const std::string &file);

    void WaitForChildren();

    void PrintExecutionTime(
            const std::chrono::high_resolution_clock::time_point &start,
            const std::chrono::high_resolution_clock::time_point &extractionEnd,
            const std::chrono::high_resolution_clock::time_point &end
    );

    static std::string GetExecutableDirectory();
    static std::string ResolvePath(const std::string& path, const std::string& baseDir);
    static void EnsureDirectoryExists(const std::string& path);
};

#endif // EXTRACTOR_H
