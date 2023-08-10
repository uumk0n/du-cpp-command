#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cstring>

constexpr uint64_t BLOCK_SIZE = 512;
constexpr int FILE_PATH_INDEX = 13;

struct Options
{
    bool printSize = false;
    bool printAllFiles = false;
    bool printOnlyFullsize = false;
    bool printFullsize = false;
    bool blockSize = true;
    bool fromFile = false;
};

void processFile(const std::filesystem::path &filePath, uint64_t &totalSize, const Options &options)
{
    uint64_t fileSize = std::filesystem::file_size(filePath);
    totalSize += fileSize;

    if (options.printSize)
        std::cout << (options.blockSize ? (fileSize + BLOCK_SIZE - 1) / BLOCK_SIZE : fileSize) << "\t";

    if (!options.printOnlyFullsize)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << filePath;
    }

    if (options.printAllFiles)
        std::cout << "\t" << fileSize << (options.blockSize ? " blocks" : " bytes");

    if (!options.printOnlyFullsize)
        std::cout << std::endl;
}

void processDirectory(const std::filesystem::path &path, uint64_t &totalSize, const Options &options)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        try
        {
            if (std::filesystem::is_regular_file(entry))
            {
                processFile(entry.path(), totalSize, options);
            }
            else if (std::filesystem::is_directory(entry))
            {
                processDirectory(entry, totalSize, options);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error processing: " << entry.path() << " - " << e.what() << std::endl;
        }
    }
}

// Function to process command-line arguments
Options processCommandLineArguments(int argc, char *argv[], uint64_t totalSize)
{
    Options options;
    int startIndex = 1;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-b")
        {
            options.printSize = true;
            options.blockSize = false;
        }
        else if (arg == "-a")
        {
            options.printAllFiles = true;
        }
        else if (arg == "-c")
        {
            options.printFullsize = true;
        }
        else if (arg == "-s")
        {
            options.printOnlyFullsize = true;
        }
        else if (arg.find("--files-from=") == 0)
        {
            options.fromFile = true;
            std::string filePath = arg.substr(FILE_PATH_INDEX);
            std::ifstream fileList(filePath);
            if (fileList.is_open())
            {
                std::string line;
                while (std::getline(fileList, line))
                {
                    processDirectory(line, totalSize, options);
                }
            }
            else
            {
                std::cerr << "Unable to open file: " << filePath << std::endl;
                exit(1);
            }
            fileList.close();
            exit(1);
        }
        else
        {
            startIndex = i;
            break;
        }
    }

    return options;
}

void processPaths(int argc, char *argv[], const Options &options, uint64_t &totalSize)
{
    for (int i = options.printOnlyFullsize || options.printFullsize ? 2 : 1; i < argc; ++i)
    {
        std::filesystem::path targetPath(argv[i]);

        if (!std::filesystem::exists(targetPath))
        {
            std::cerr << "Path does not exist: " << targetPath << std::endl;
            continue;
        }

        processDirectory(targetPath, totalSize, options);
    }
}

int main(int argc, char *argv[])
{
    uint64_t totalSize = 0;
    Options options = processCommandLineArguments(argc, argv, totalSize);

    if (!options.fromFile)
        processPaths(argc, argv, options, totalSize);

    if (options.printOnlyFullsize || options.printFullsize)
        std::cout << "Total size: " << totalSize << (options.blockSize ? " blocks" : " bytes") << std::endl;

    return 0;
}
