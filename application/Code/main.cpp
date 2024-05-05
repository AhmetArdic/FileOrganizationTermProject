#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <unordered_map>

#include <thread>
#include <vector>

#include <functional>

#include "./hashingLibs/md5.h"
#include "./hashingLibs/sha256.h"
#include "./hashingLibs/sha1.h"

#include "./folderWatcher/folderWatcher.h"

#define _DEBUG_
#define _BENCHMARK_

const std::wstring CalculateMD5Hash(std::wstring password)
{
    static MD5 md5;

    std::string result = md5(std::string(password.begin(), password.end()));
    return std::wstring(result.begin(), result.end()) ;
}

const std::wstring CalculateSha128(std::wstring password)
{
    static SHA1 sha1;
    
    std::string result = sha1(std::string(password.begin(), password.end()));
    return std::wstring(result.begin(), result.end()) ;
}


const std::wstring CalculateSha256(std::wstring password)
{
    static SHA256 sha256;

    std::string result = sha256(std::string(password.begin(), password.end()));
    return std::wstring(result.begin(), result.end()) ;
}


class IndexingProcessorCls
{
private:
    void WriteToFile(const std::wstring& line, const std::filesystem::path& path)
    {
        std::wofstream outputFile(path, std::ios::app);  // ilgili dosyayi append modunda ac
        outputFile << line << std::endl;
        outputFile.close(); // dosyayi kapat
    }

    std::wstring InvalidSignConvertor(wchar_t c)
    {
        switch (c)
        {
        case L'\\':
            return L"BackSlash";
        case L'/':
            return L"Slash";
        case L':':
            return L"Colon";
        case L'*':
            return L"Star";
        case L'?':
            return L"QuestionMark";
        case L'"':
            return L"QuotationMark";
        case L'<':
            return L"LessThanSign";
        case L'>':
            return L"GreaterThanSign";
        case L'|':
            return L"VerticalLineSign";
        case L'':
            return L"EscapeSequence";
        default:
            return std::wstring(1, c);
        }
    }
    std::wstring GenerateSubFolderName(wchar_t firstChar)
    {
        if(!std::iswalpha(firstChar) && !std::iswdigit(firstChar))
        {
            return L"@(" + InvalidSignConvertor(firstChar) + L")";
        }
        else if(std::iswupper(firstChar))
        {
            return L"_" + std::wstring(1, firstChar) + L"_";
        }
        else
        {
            return std::wstring(1, firstChar);
        }
    }

    std::wstring MakeSubFolder(const std::wstring& path)
    {
        // path: okunan dosyadaki sifrenin ilk karakterine gore olusturulacak klasorun ismi ve yolu

        if (!std::filesystem::exists(path)) 
        {
            // eger dizin yok ise olustur

            try
            {
                std::filesystem::create_directory(path);
            }
            catch(const std::exception& e)
            {
                throw;
            }
        }

        return path;
    }

    int CalculateFileNumber(const std::wstring& key)
    {
        if(++map_[key].totalLineNumberOfCurrentFile > IndexingProcessorCls::MAX_LINE_NUMBER)
        {
            // eger o an uzerinde calisilan karakteri iceren dizindeki dosyanin satir sayisi MAX_LINE_NUMBER'i gecerse

            ++map_[key].totalFileNumberOfIndexSubfolder;   // yeni satirlarin yazilacagi yeni dosyanin numarasi
            map_[key].totalLineNumberOfCurrentFile = 0;    // yeni dosyaya gecildigi icin satir sayisi sayacini sifirla
        }

        return map_[key].totalFileNumberOfIndexSubfolder;
    }

    void ScanFolder(void)
    {
#ifdef _DEBUG_
        std::cout << "ScanFolder is started" << std::endl;
#endif /* _DEBUG_ */

        // unprocessedPasswordsDir dizininde dolas
        for (const auto& entry : std::filesystem::directory_iterator(unprocessedPasswordsDir)) 
        {
            if (entry.is_regular_file())
            {
                // dizindeki obje regular file ise

#ifdef _DEBUG_
                std::cout << entry.path() << std::endl; // uzerinde islem yapilan dosya
#endif /* _DEBUG_ */

                ScanFile(entry);
            }
        }

#ifdef _DEBUG_
        std::cout << "ScanFolder is finished" << std::endl;
#endif /* _DEBUG_ */
    }

    void Indexing()
    {
#ifdef _DEBUG_
        std::cout << "Indexing is started" << std::endl;
#endif /* _DEBUG_ */
        std::vector<std::thread> subFolderThreads;

        for(auto item = passwords_.begin() ; item != passwords_.end() ; ++item)
        {
#ifdef _DEBUG_ 
            afterFilterForCaseSensitivityAndDuplications[item->first]++;
#endif /* _DEBUG_ */

            subFolderThreads.emplace_back([this, item]()
            {
                std::vector<std::thread> fileThreads;

                std::wstring subFolderPath = MakeSubFolder(indexDir + L"\\" + item->first);   

                for(auto file = item->second.begin() ; file != item->second.end() ; ++file)
                {
                    fileThreads.emplace_back([this, subFolderPath, file]()
                    {
                        std::wstring fileName = L"passwords" + std::to_wstring(file->first) + L".txt";   // okunan sifrenin yazilacagi dosyanin ismi

                        for(auto password = file->second.begin() ; password != file->second.end() ; ++password)
                        {
                            std::wstring line = password->first + L"|" + CalculateMD5Hash(password->first) + L"|" + CalculateSha128(password->first) + L"|" + CalculateSha256(password->first) + L"|" + password->second; 
                            WriteToFile(line, subFolderPath + L"\\" + fileName);
                        }
                    });   
                }

                for (auto& thread : fileThreads) 
                {
                    thread.join();
                }
            });
        }


        for (auto& thread : subFolderThreads) 
        {
            thread.join();
        }

#ifdef _DEBUG_
        std::cout << "Indexing is finished" << std::endl;
#endif /* _DEBUG_ */
    }

public:
    IndexingProcessorCls(const std::wstring& indexDir, const std::wstring& unprocessedPasswordsDir) : indexDir{indexDir}, unprocessedPasswordsDir{unprocessedPasswordsDir} {}

    void ScanFile(const std::filesystem::directory_entry& entry)
    {
#ifdef _DEBUG_
        std::cout << "ScanFile is started" << std::endl;
#endif /* _DEBUG_ */

        std::wifstream inputFile(entry.path());  // uzerinde islem yapiacak dosyayi okuma modunda ac
        std::wstring line;

#ifdef _BENCHMARK_
        auto start = std::chrono::high_resolution_clock::now();
#endif /* _BENCHMARK_ */

        // okuma modunda acilan dosyanin satirlarında dolas
        while (std::getline(inputFile, line)) 
        {
            if (!line.empty())  
            {
                // eger satir bos degilse

                std::wstring subfolderName = GenerateSubFolderName(line[0]);
                int fileNumber = CalculateFileNumber(subfolderName);

#ifdef _DEBUG_   
                beforeFilterForCaseSensitivityAndDuplications[subfolderName]++;
#endif /* _DEBUG_ */

                passwords_[subfolderName][fileNumber][line] = entry.path().filename().wstring(); 
            }
        }

#ifdef _BENCHMARK_
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "Elapsed time: " << elapsed.count() << " milisecond on " << entry.path().filename() << std::endl;
#endif /* _BENCHMARK_ */

#ifdef _DEBUG_
        std::cout << "ScanFile is finished" << std::endl;
#endif /* _DEBUG_ */
    }

    void Run(void)
    {
        ScanFolder();

        Indexing();
    }

private:
    static constexpr int MAX_LINE_NUMBER = 10000; 

private:
    const std::wstring indexDir;
    const std::wstring unprocessedPasswordsDir;

    struct IndexingProcessStruct
    {
        int totalLineNumberOfCurrentFile;
        int totalFileNumberOfIndexSubfolder;
    };

    std::unordered_map<std::wstring, IndexingProcessStruct> map_;

    //first: subfolderName, second.first: file number, second.second.first: password, second.second.second: path 
    std::unordered_map<std::wstring, std::unordered_map<int, std::unordered_map<std::wstring, std::wstring>>> passwords_;

#ifdef _DEBUG_
    std::unordered_map<std::wstring, int> beforeFilterForCaseSensitivityAndDuplications;
    std::unordered_map<std::wstring, int> afterFilterForCaseSensitivityAndDuplications;
#endif /* _DEBUG_ */
};

void deneme(void)
{
    std::cout << "deneme\n";
}

#include <chrono>
int main(void) 
{
    const std::wstring unprocessedPasswordsDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Unprocessed-Passwords";
    const std::wstring indexDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Index";

#ifdef _BENCHMARK_
    auto start = std::chrono::high_resolution_clock::now();
#endif /* _BENCHMARK_ */

    IndexingProcessorCls indexingProcessor{indexDir, unprocessedPasswordsDir};
    indexingProcessor.Run();

#ifdef _BENCHMARK_
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Total elapsed time: " << elapsed.count() << " milisecond" << std::endl;
#endif /* _BENCHMARK_ */

    // auto callback = std::bind(&IndexingProcessorCls::Run, &indexingProcessor);
    // FolderWatcher watcher(std::string(unprocessedPasswordsDir.begin(), unprocessedPasswordsDir.end()), callback);
    // watcher.Start();
    
    // while(1) {}
    
    return 0;
}
