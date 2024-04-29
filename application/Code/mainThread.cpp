#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

// Mutex tanımı
std::mutex mtx;

const std::string CalculateMD5Hash(void)
{
    return "MD5Hash";
}

const std::string CalculateSha128(void)
{
    return "Sha128";
}

const std::string CalculateSha256(void)
{
    return "Sha256";
}

void processFile(const fs::path& filePath, const std::string& indexDir)
{
    std::ifstream inputFile(filePath);
    std::string line;
    
    int totalLineNumberOfCurrentFile = 0;
    int totalFileNumberOfIndexSubfolder = 0;
    while (std::getline(inputFile, line))
    {
        if (!line.empty())
        {
            totalLineNumberOfCurrentFile++;
            if (totalLineNumberOfCurrentFile > 10000)
            {
                totalFileNumberOfIndexSubfolder++;
                totalLineNumberOfCurrentFile = 0;
            }

            char firstChar = line[0];
            std::string subfolderName;
            if (!std::isalpha(firstChar) && !std::isdigit(firstChar))
            {
                subfolderName = "@";
            }
            else
            {
                subfolderName = firstChar;
            }

            std::string indexSubfolderPath = indexDir + "\\" + subfolderName;

            // Mutex kilidi
            mtx.lock();
            if (!fs::exists(indexSubfolderPath))
            {
                fs::create_directory(indexSubfolderPath);
            }
            mtx.unlock(); // Mutex kilidini aç

            const std::string fileName = "passwords" + std::to_string(totalFileNumberOfIndexSubfolder) + ".txt";
            std::ofstream outputFile(indexSubfolderPath + "\\" + fileName, std::ios::app);

            // Mutex kilidi
            mtx.lock();
            outputFile << line << "|" << CalculateMD5Hash() << "|" << CalculateSha128() << "|" << CalculateSha256() << "|" << filePath.filename() << std::endl;
            mtx.unlock(); // Mutex kilidini aç

            outputFile.close();
        }
    }
}

int main()
{
    const std::string unprocessedPasswordsDir = "C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Unprocessed-Passwords";
    const std::string indexDir = "C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Index";

    std::vector<std::thread> threads;

    for (const auto& entry : fs::directory_iterator(unprocessedPasswordsDir))
    {
        if (entry.is_regular_file())
        {
            // Her dosya için bir iş parçacığı oluştur
            threads.emplace_back(processFile, entry.path(), indexDir);
        }
    }

    // Tüm iş parçacıklarını bitirme
    for (auto& thread : threads)
    {
        thread.join();
    }

    return 0;
}
