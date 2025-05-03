#ifndef ARCHIVE_CREATOR_H
#define ARCHIVE_CREATOR_H

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sys/wait.h>
#include <unistd.h>

class ArchiveCreator
{
public:
    ArchiveCreator(std::string archiveName, const std::vector<std::string> &inputFiles);

    void RunSequential();

    void RunParallel(int numberProcesses);

private:
    //не использовать глобальных путей
    std::string m_compressedFilesList = "/tmp/compressed_files.txt";
    std::string m_archiveName;
    std::vector<std::string> m_inputFiles;
    std::vector<pid_t> m_children;
    std::vector<std::string> m_compressedFiles;

    void CompressFile(const std::string &file);

    void WaitForChildren();

    void CreateArchive();

    static void PrintExecutionTime(
            const std::chrono::high_resolution_clock::time_point &start,
            const std::chrono::high_resolution_clock::time_point &compressionEnd,
            const std::chrono::high_resolution_clock::time_point &end
    );

    void SpawnChild(const std::string &file);

    static std::string GetExecutableDirectory();
    static std::string ResolveArchivePath(const std::string& archiveName, const std::string& exeDir);
    void ValidateAndResolveInputFiles(const std::vector<std::string>& inputFiles, const std::string& exeDir);
};

#endif // ARCHIVE_CREATOR_H
