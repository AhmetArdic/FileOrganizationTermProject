#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <unordered_map>

#include <thread>
#include <vector>

#define _DEBUG

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


class IndexingProcessorCls
{
private:
    void WriteToFile(const std::string& line, const std::string& fileName, const std::string& path)
    {
        std::ofstream outputFile(path + "\\" + fileName, std::ios::app);  // ilgili dosyayi append modunda ac
        outputFile << line << std::endl;
        outputFile.close(); // dosyayi kapat
    }

    std::string GenerateSubFolderName(char firstChar)
    {
        if(std::isdigit(firstChar)) 
        {
            // ilk karakter sayi ise   
            
            return std::string(1, firstChar);
        }
        else if(std::isalpha(firstChar))    
        {
            // ilk karakter alfabe ise

            return std::string(1, std::tolower(firstChar));    // klasor isimleri buyuk-kucuk harf duyarliligina sahip olmadigi icin duzenliyorum
        }
        else                                
        {
            // ilk karakter sayi ve alfabe disinde bir karakterse

            return "@";
        }
    }

    std::string MakeSubFolder(const std::string& path)
    {
        // path: okunan dosyadaki sifrenin ilk karakterine gore olusturulacak klasorun ismi ve yolu

        if (!std::filesystem::exists(path)) 
        {
            // eger dizin yok ise olustur

            std::filesystem::create_directory(path);
        }

        return path;
    }

    int CalculateFileNumber(const std::string& key)
    {
        if(++map_[key].totalLineNumberOfCurrentFile > IndexingProcessorCls::MAX_LINE_NUMBER)
        {
            // eger o an uzerinde calisilan karakteri iceren dizindeki dosyanin satir sayisi MAX_LINE_NUMBER'i gecerse

            ++map_[key].totalFileNumberOfIndexSubfolder;   // yeni satirlarin yazilacagi yeni dosyanin numarasi
            map_[key].totalLineNumberOfCurrentFile = 0;    // yeni dosyaya gecildigi icin satir sayisi sayacini sifirla
        }

        return map_[key].totalFileNumberOfIndexSubfolder;
    }

public:
    IndexingProcessorCls(const std::string& indexDir) : indexDir{indexDir} {}

    void Scanning(const std::filesystem::directory_entry& entry)
    {
        std::ifstream inputFile(entry.path());  // uzerinde islem yapiacak dosyayi okuma modunda ac
        std::string line;

        // okuma modunda acilan dosyanin satirlarında dolas
        while (std::getline(inputFile, line)) 
        {
            if (!line.empty())  
            {
                // eger satir bos degilse

                std::string subfolderName = GenerateSubFolderName(line[0]);

#ifdef DEBUG    
                beforeFilterForCaseSensitivityAndDuplications[subfolderName]++;
#endif /* DEBUG */

                passwords_[subfolderName][line] = entry.path().filename().string();
            }
        }
    }

    void Indexing()
    {
        std::vector<std::thread> threads;

        for (const auto& item : passwords_) 
        {
#ifdef DEBUG  
            afterFilterForCaseSensitivityAndDuplications[item.first]++;
#endif /* DEBUG */

            threads.emplace_back([this, item]()
            {
                for (const auto& password : item.second) 
                {
                    int fileNumber = CalculateFileNumber(item.first);

                    std::string subFolderPath = MakeSubFolder(indexDir + "\\" + item.first);

                    std::string fileName = "passwords" + std::to_string(fileNumber) + ".txt";   // okunan sifrenin yazilacagi dosyanin ismi

                    std::string line = password.first + "|" + CalculateMD5Hash() + "|" + CalculateSha128() + "|" + CalculateSha256() + "|" + password.second; 
                    WriteToFile(line, fileName, subFolderPath);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

public:
    static constexpr int MAX_LINE_NUMBER = 10000; 

private:
    std::string indexDir;

    struct IndexingProcessStruct
    {
        int totalLineNumberOfCurrentFile;
        int totalFileNumberOfIndexSubfolder;
    };

    std::unordered_map<std::string, IndexingProcessStruct> map_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> passwords_;

#ifdef DEBUG
    std::unordered_map<std::string, int> beforeFilterForCaseSensitivityAndDuplications;
    std::unordered_map<std::string, int> afterFilterForCaseSensitivityAndDuplications;
#endif /* DEBUG */
};

#include <chrono>
int main(void) 
{
    auto start = std::chrono::high_resolution_clock::now();

    const std::string unprocessedPasswordsDir = "C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Unprocessed-Passwords";
    const std::string indexDir = "C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Index";

    IndexingProcessorCls indexingProcessor{indexDir};

    // unprocessedPasswordsDir dizininde dolas
    for (const auto& entry : std::filesystem::directory_iterator(unprocessedPasswordsDir)) 
    {
        if (entry.is_regular_file())
        {
            // dizindeki obje regular file ise

#ifdef DEBUG
            std::cout << entry.path() << std::endl; // uzerinde islem yapilan dosya
#endif /* DEBUG */

            indexingProcessor.Scanning(entry);
        }
    }

    indexingProcessor.Indexing();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " milisecond" << std::endl;

    return 0;
}
