#include "ArchiveCreator.h"

#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <climits>
#include <fstream>

ArchiveCreator::ArchiveCreator(std::string archiveName, const std::vector<std::string> &inputFiles)
{
    std::string exeDir = GetExecutableDirectory();
    m_archiveName = ResolveArchivePath(archiveName, exeDir);
    ValidateAndResolveInputFiles(inputFiles, exeDir);
}

std::string ArchiveCreator::GetExecutableDirectory()
{
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len == -1)
    {
        std::cerr << "Failed to get executable path\n";
        exit(EXIT_FAILURE);
        //убрать exit
    }
    exePath[len] = '\0';

    std::string exeDir = std::string(exePath);
    size_t pos = exeDir.find_last_of('/');
    if (pos != std::string::npos)
    {
        exeDir = exeDir.substr(0, pos);
    }

    return exeDir;
}

std::string ArchiveCreator::ResolveArchivePath(const std::string &archiveName, const std::string &exeDir)
{
    return exeDir + "/" + archiveName;
}

void ArchiveCreator::ValidateAndResolveInputFiles(const std::vector<std::string> &inputFiles, const std::string &exeDir)
{
    for (const auto &file: inputFiles)
    {
        if (file[0] == '~')
        {
            std::cerr << "HOME-absolute paths are not supported, use relative paths instead.\n";
            exit(EXIT_FAILURE);
        } else
        {
            m_inputFiles.push_back(exeDir + "/" + file);
        }
    }
}

void ArchiveCreator::CompressFile(const std::string &file)
{
    std::string command = "gzip -kf \"" + file + "\"";
    if (system(command.c_str()) != 0)
    {
        std::cerr << "Failed to compress " << file << "\n";
    } else
    {
        std::cout << file + ".gz created" << std::endl;

        std::ofstream out(m_compressedFilesList, std::ios::app);
        out << file + ".gz" << std::endl;
    }
}

void ArchiveCreator::SpawnChild(const std::string &file)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        CompressFile(file);
        exit(EXIT_SUCCESS);
    } else if (pid > 0)
    {
        m_children.push_back(pid);
    } else
    {
        std::cerr << "Failed to spawn child process\n";
    }
}

void ArchiveCreator::WaitForChildren()
{
    for (pid_t child: m_children)
    {
        waitpid(child, nullptr, 0);
    }
    m_children.clear();
}

void ArchiveCreator::CreateArchive()
{
    std::stringstream tarCommand;
    tarCommand << "tar -cf \"" << m_archiveName << "\"";

    for (const auto &file: m_compressedFiles)
    {
        tarCommand << " \"" << file << "\"";
    }

    if (system(tarCommand.str().c_str()) == 0)
    {
        std::cout << "Archive saved to " << m_archiveName << "\n";

        for (const auto &file: m_compressedFiles)
        {
            std::string removeCmd = "rm \"" + file + "\"";
            system(removeCmd.c_str());
        }
    } else
    {
        std::cerr << "Failed to create archive\n";
    }
}

void ArchiveCreator::PrintExecutionTime(
        const std::chrono::high_resolution_clock::time_point &start,
        const std::chrono::high_resolution_clock::time_point &compressionEnd,
        const std::chrono::high_resolution_clock::time_point &end)
{
    std::cout << "Compression time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(compressionEnd - start).count()
              << " ms\n";
    std::cout << "Total execution time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";
}

void ArchiveCreator::RunSequential()
{
    auto start = std::chrono::high_resolution_clock::now();

    for (const auto &file: m_inputFiles)
    {
        CompressFile(file);
    }

    auto compressionEnd = std::chrono::high_resolution_clock::now();
    m_compressedFiles.clear();
    std::ifstream in(m_compressedFilesList);
    std::string gzFile;
    while (std::getline(in, gzFile))
    {
        m_compressedFiles.push_back(gzFile);
    }
    CreateArchive();
    auto end = std::chrono::high_resolution_clock::now();

    PrintExecutionTime(start, compressionEnd, end);
}

void ArchiveCreator::RunParallel(int numberProcesses)
{
    auto start = std::chrono::high_resolution_clock::now();
    size_t activeProcesses = 0;

    for (const auto &file: m_inputFiles)
    {
        while (activeProcesses >= numberProcesses)
        {
            int status;
            wait(&status);
            --activeProcesses;
        }
        SpawnChild(file);
        ++activeProcesses;
    }
    WaitForChildren();

    auto compressionEnd = std::chrono::high_resolution_clock::now();
    m_compressedFiles.clear();
    std::ifstream in(m_compressedFilesList);
    std::string gzFile;
    while (std::getline(in, gzFile))
    {
        m_compressedFiles.push_back(gzFile);
    }
    CreateArchive();
    auto end = std::chrono::high_resolution_clock::now();

    PrintExecutionTime(start, compressionEnd, end);
}
