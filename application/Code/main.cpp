#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>

#include <thread>
#include <mutex>
#include <vector>

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


class IndexingProcessCls
{
private:
    void WriteToFile(const std::string& line, const std::string& fileNumber, const std::string& path)
    {
        const std::string fileName = "passwords" + fileNumber + ".txt"; // okunan sifrenin yazilacagi dosyanin ismi

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

    int CalculateFileNumber(std::string& key)
    {
        if(++map_[key].totalLineNumberOfCurrentFile > IndexingProcessCls::MAX_LINE_NUMBER)
        {
            // eger o an uzerinde calisilan karakteri iceren dizindeki dosyanin satir sayisi MAX_LINE_NUMBER'i gecerse

            ++map_[key].totalFileNumberOfIndexSubfolder;   // yeni satirlarin yazilacagi yeni dosyanin numarasi
            map_[key].totalLineNumberOfCurrentFile = 0;    // yeni dosyaya gecildigi icin satir sayisi sayacini sifirla
        }

        return map_[key].totalFileNumberOfIndexSubfolder;
    }

public:
    IndexingProcessCls(const std::string& indexDir) : indexDir{indexDir} {}

    void Indexing(const std::filesystem::directory_entry& entry)
    {
        if (entry.is_regular_file())
        {
            // dizindeki obje regular file ise
            
            std::cout << entry.path() << std::endl; // uzerinde islem yapilan dosya

            std::ifstream inputFile(entry.path());  // uzerinde islem yapiacak dosyayi okuma modunda ac
            std::string line;

            // okuma modunda acilan dosyanin satirlarında dolas
            while (std::getline(inputFile, line)) 
            {
                if (!line.empty())  
                {
                    // eger satir bos degilse

                    std::string subfolderName = GenerateSubFolderName(line[0]);

                    int fileNumber = CalculateFileNumber(subfolderName);
                    std::string subFolderPath = MakeSubFolder(indexDir + "\\" + subfolderName);
                    WriteToFile(line+"|"+CalculateMD5Hash()+"|"+CalculateSha128()+"|"+CalculateSha256()+"|"+entry.path().filename().string(), 
                                std::to_string(fileNumber), subFolderPath);
                }
            }
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

    std::map<std::string, IndexingProcessStruct> map_;
};

#include <chrono>
int main(void) 
{
    auto start = std::chrono::high_resolution_clock::now();

    const std::string unprocessedPasswordsDir = "C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Unprocessed-Passwords";
    const std::string indexDir = "C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Index";

    IndexingProcessCls indexingProcess{indexDir};

    // unprocessedPasswordsDir dizininde dolas
    for (const auto& entry : std::filesystem::directory_iterator(unprocessedPasswordsDir)) 
    {
        indexingProcess.Indexing(entry);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Geçen süre: " << elapsed.count() << " milisaniye" << std::endl;

    return 0;
}
