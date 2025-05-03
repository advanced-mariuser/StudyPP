#include "Extractor.h"
#include <filesystem>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <climits>

namespace fs = std::filesystem;

Extractor::Extractor(std::string archiveName, std::string outputFolder)
{
    std::string exeDir = GetExecutableDirectory();

    m_archiveName = ResolvePath(archiveName, exeDir);
    m_outputFolder = ResolvePath(outputFolder, exeDir);

    EnsureDirectoryExists(m_outputFolder);

    if (!fs::exists(m_archiveName))
    {
        std::cerr << "Archive not found: " << m_archiveName << "\n";
        exit(EXIT_FAILURE);
    }
}

std::string Extractor::GetExecutableDirectory()
{
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len == -1)
    {
        std::cerr << "Failed to get executable path\n";
        exit(EXIT_FAILURE);
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

std::string Extractor::ResolvePath(const std::string &path, const std::string &baseDir)
{
    if (path[0] == '/')
    {
        return path;
    }
    return baseDir + "/" + path;
}

void Extractor::EnsureDirectoryExists(const std::string &path)
{
    if (!fs::exists(path))
    {
        if (!fs::create_directories(path))
        {
            std::cerr << "Failed to create directory: " << path << "\n";
            exit(EXIT_FAILURE);
        }
    }
}

void Extractor::ExtractArchive()
{
    size_t stripComponents = 0;
    std::string archivePath = m_archiveName;
    size_t pos = archivePath.find_last_of('/');
    if (pos != std::string::npos)
    {
        archivePath = archivePath.substr(0, pos);
    }

    for (char ch: archivePath)
    {
        if (ch == '/')
        {
            stripComponents++;
        }
    }

    std::string command = "tar -xf " + m_archiveName + " -C " + m_outputFolder + " --strip-components=" +
                          std::to_string(stripComponents);
    if (system(command.c_str()) != 0)
    {
        std::cerr << "Failed to extract archive: " << m_archiveName << "\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Archive extracted successfully to: " << m_outputFolder << "\n";
}

void Extractor::FindCompressedFiles()
{
    m_compressedFiles.clear();
    for (const auto &entry: fs::directory_iterator(m_outputFolder))
    {
        if (entry.path().extension() == ".gz")
        {
            m_compressedFiles.push_back(entry.path().string());
        }
    }
    if (m_compressedFiles.empty())
    {
        std::cerr << "No compressed files found in: " << m_outputFolder << "\n";
        exit(EXIT_FAILURE);
    }
}

void Extractor::DecompressFile(const std::string &file)
{
    std::string command = "gzip -df " + file;
    if (system(command.c_str()) != 0)
    {
        std::cerr << "Failed to decompress file: " << file << "\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "File decompressed: " << file << "\n";
}

void Extractor::SpawnChild(const std::string &file)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        DecompressFile(file);
        exit(EXIT_SUCCESS);
    } else if (pid > 0)
    {
        m_children.push_back(pid);
    } else
    {
        std::cerr << "Failed to spawn child process\n";
        exit(EXIT_FAILURE);
    }
}

void Extractor::WaitForChildren()
{
    for (pid_t child: m_children)
    {
        int status;
        waitpid(child, &status, 0);
        if (WIFEXITED(status))
        {
            std::cout << "Child process " << child << " exited with status " << WEXITSTATUS(status) << "\n";
        } else
        {
            std::cerr << "Child process " << child << " terminated abnormally\n";
        }
    }
    m_children.clear();
}

void Extractor::PrintExecutionTime(
        const std::chrono::high_resolution_clock::time_point &start,
        const std::chrono::high_resolution_clock::time_point &extractionEnd,
        const std::chrono::high_resolution_clock::time_point &end
)
{
    std::cout << "Extraction time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(extractionEnd - start).count()
              << " ms\n";
    std::cout << "Total execution time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";
}

void Extractor::RunSequential()
{
    auto start = std::chrono::high_resolution_clock::now();

    ExtractArchive();
    auto extractionEnd = std::chrono::high_resolution_clock::now();

    FindCompressedFiles();
    for (const auto &file: m_compressedFiles)
    {
        DecompressFile(file);
    }

    auto end = std::chrono::high_resolution_clock::now();
    PrintExecutionTime(start, extractionEnd, end);
}

void Extractor::RunParallel(int numProcesses)
{
    auto start = std::chrono::high_resolution_clock::now();

    ExtractArchive();
    auto extractionEnd = std::chrono::high_resolution_clock::now();

    FindCompressedFiles();
    size_t activeProcesses = 0;

    for (const auto &file: m_compressedFiles)
    {
        while (activeProcesses >= numProcesses)
        {
            int status;
            wait(&status);
            --activeProcesses;
        }
        SpawnChild(file);
        ++activeProcesses;
    }

    WaitForChildren();

    auto end = std::chrono::high_resolution_clock::now();
    PrintExecutionTime(start, extractionEnd, end);
}