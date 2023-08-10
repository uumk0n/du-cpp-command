#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>

void processDirectory(const std::filesystem::path &path, uint64_t &totalSize, bool printSize, bool printAllFiles, bool blockSize, bool printOnlyFullsize)
{
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        try
        {
            if (std::filesystem::is_regular_file(entry))
            {
                uint64_t fileSize = std::filesystem::file_size(entry);
                totalSize += fileSize;

                if (printSize)
                {
                    if (blockSize)
                    {
                        std::cout << (fileSize + 511) / 512 << "\t";
                    }
                    else
                    {
                        std::cout << fileSize << "\t";
                    }
                }

                if (!printOnlyFullsize)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    std::cout << entry.path();
                }
                if (printAllFiles)
                {
                    std::cout << "\t" << fileSize << (blockSize ? " blocks" : " bytes");
                }

                if (!printOnlyFullsize)
                    std::cout << std::endl;
            }
            else if (std::filesystem::is_directory(entry))
            {
                processDirectory(entry, totalSize, printSize, printAllFiles, blockSize, printOnlyFullsize);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error processing: " << entry.path() << " - " << e.what() << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    bool printSize = false;
    bool printAllFiles = false;
    bool printOnlyFullsize = false;
    bool printFullsize = false;

    bool blockSize = true; // По умолчанию вывод в 512-байтных блоках
    uint64_t totalSize = 0;
    int startIndex = 1;

    // Обработка опций
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-b") == 0)
        {
            printSize = true;
            blockSize = false; // Параметр -b выводит точный размер в байтах
        }
        else if (strcmp(argv[i], "-a") == 0)
        {
            printAllFiles = true;
        }
        else if (strcmp(argv[i], "-c") == 0)
        {
            printFullsize = true;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            printOnlyFullsize = true;
        }
        else if (strncmp(argv[i], "--files-from=", 13) == 0)
        {
            // Извлечь путь к файлу с путями
            std::string filePath = argv[2];
            std::ifstream fileList(filePath);
            if (fileList.is_open())
            {
                std::string line;
                while (std::getline(fileList, line))
                {
                    processDirectory(line, totalSize, printSize, printAllFiles, blockSize, printOnlyFullsize);
                }
            }
            else
            {
                std::cerr << "Unable to open file: " << filePath << std::endl;
                return 0;
            }
            fileList.close();
            return 0;
        }
        else
        {
            // Этот аргумент не является опцией, начинается обработка путей
            startIndex = i;
            break;
        }
    }

    // Обработка путей
    for (int i = startIndex; i < argc; ++i)
    {
        std::filesystem::path targetPath(argv[i]);

        // Обработка пути
        if (!std::filesystem::exists(targetPath))
        {
            std::cerr << "Path does not exist: " << targetPath << std::endl;
            continue;
        }

        processDirectory(targetPath, totalSize, printSize, printAllFiles, blockSize, printOnlyFullsize);
    }

    if (printOnlyFullsize || printFullsize)
    {
        std::cout << "Total size: " << totalSize << (blockSize ? " blocks" : " bytes") << std::endl;
    }

    return 0;
}
