#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <algorithm>

#include <chrono>

#include "IndexingProcessor.h"


int IndexingProcessorCls::CalculateFileNumber(const std::wstring& key)
{
    if(++map_[key].totalLineNumberOfCurrentFile > IndexingProcessorCls::MAX_LINE_NUMBER)
    {
        // eger o an uzerinde calisilan karakteri iceren dizindeki dosyanin satir sayisi MAX_LINE_NUMBER'i gecerse

        ++map_[key].totalFileNumberOfIndexSubfolder;   // yeni satirlarin yazilacagi yeni dosyanin numarasi
        map_[key].totalLineNumberOfCurrentFile = 0;    // yeni dosyaya gecildigi icin satir sayisi sayacini sifirla
    }

    return map_[key].totalFileNumberOfIndexSubfolder;
}

void IndexingProcessorCls::ScanIndex(const std::filesystem::path& path)
{
    std::cout << "ScanIndex is started" << std::endl;

    std::wifstream inputFile(path);  // uzerinde islem yapiacak dosyayi okuma modunda ac
    std::wstring line;

    // okuma modunda acilan dosyanin satirlarında dolas
    while (std::getline(inputFile, line)) 
    {
        if (!line.empty())  
        {
            // eger satir bos degilse

            const auto& subfolderName = IndexingProcessorHelperCls::GetSubstringBetweenDelimiters(path, L'\\', L'\\');
            const auto& fileNumber = IndexingProcessorHelperCls::ExtractIntegerFromString(path.filename());
            const auto& password = IndexingProcessorHelperCls::ExtractSubstringBeforeFirstDelimiter(line, '|');
            const auto& filePath = IndexingProcessorHelperCls::ExtractSubstringAfterLastDelimiter(line, '|');

            passwords_[subfolderName][fileNumber][password] = filePath;
        }
    }

    std::cout << "ScanIndex is finished" << std::endl;
}

void IndexingProcessorCls::ScanFile(const std::filesystem::path& path)
{
    std::cout << "ScanFile is started" << std::endl;

    std::wifstream inputFile(path);  // uzerinde islem yapiacak dosyayi okuma modunda ac
    std::wstring password;

    auto start = std::chrono::high_resolution_clock::now();

    // okuma modunda acilan dosyanin satirlarında dolas
    while (std::getline(inputFile, password)) 
    {
        if (!password.empty())  
        {
            // eger satir bos degilse

            std::wstring subfolderName = IndexingProcessorHelperCls::GenerateSubFolderName(password[0]);
            int fileNumber = CalculateFileNumber(subfolderName);

            if(!Search(password))
            {
                writablePasswords_[subfolderName][fileNumber][password] = path.filename();
            }

            passwords_[subfolderName][fileNumber][password] = path.filename(); 
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " milisecond on " << path.filename() << std::endl;

    std::cout << "ScanFile is finished" << std::endl;
}

void IndexingProcessorCls::ScanFolder(const std::wstring& path, std::function<void(const std::filesystem::path&)> callback)
{
    std::cout << "ScanFolder is started" << std::endl;

    // unprocessedPasswordsDir dizininde dolas
    for (const auto& entry : std::filesystem::directory_iterator(path)) 
    {
        if (entry.is_regular_file())
        {
            // dizindeki obje regular file ise

            callback(entry.path());
        }
        else if(entry.is_directory())
        {
            ScanFolder(entry.path(), callback);
        }
    }

    std::cout << "ScanFolder is finished" << std::endl;
}

void IndexingProcessorCls::Indexing()
{
    std::cout << "Indexing is started" << std::endl;

    std::vector<std::thread> subFolderThreads;

    auto start = std::chrono::high_resolution_clock::now();

    for(const auto& item : writablePasswords_)
    {
        subFolderThreads.emplace_back([this, item]()
        {
            std::vector<std::thread> fileThreads;

            std::wstring subFolderPath = IndexingProcessorHelperCls::MakeSubFolder(indexDir + L"\\" + item.first);

            for(const auto& file : item.second)
            {
                fileThreads.emplace_back([this, subFolderPath, file]()
                {
                    std::wstring fileName = L"passwords" + std::to_wstring(file.first) + L".txt";   // okunan sifrenin yazilacagi dosyanin ismi

                    for(const auto& password : file.second)
                    {
                        std::wstring line = password.first + L"|" + 
                                            IndexingProcessorHelperCls::CalculateMD5Hash(password.first) + L"|" + 
                                            IndexingProcessorHelperCls::CalculateSha128(password.first) + L"|" + 
                                            IndexingProcessorHelperCls::CalculateSha256(password.first) + L"|" + 
                                            password.second;
                        IndexingProcessorHelperCls::WriteToFile(line, subFolderPath + L"\\" + fileName);
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

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Total elapsed time: " << elapsed.count() << " milisecond on Indexing" << std::endl;

    writablePasswords_.clear();

    std::cout << "Indexing is finished" << std::endl;
}

void IndexingProcessorCls::HandleNewPassword(const std::wstring& filePath)
{
    ScanFile(filePath);

    Indexing();
}

void IndexingProcessorCls::Run()
{   
    ScanFolder(indexDir, [this](auto&& PH1) { ScanIndex(std::forward<decltype(PH1)>(PH1)); });
    
    std::thread indexingThread([this]() {
        ScanFolder(unprocessedPasswordsDir, [this](auto&& PH1) { ScanFile(std::forward<decltype(PH1)>(PH1)); });
        Indexing();
    });
    indexingThread.detach();
}

bool IndexingProcessorCls::Search(const std::wstring& password)
{
    const auto& subFolderName = passwords_[IndexingProcessorHelperCls::GenerateSubFolderName(password[0])];
    bool hasPassword = std::any_of(subFolderName.begin(),
                                   subFolderName.end(),
                                   [&](const auto& file)
                                   {
                                       return file.second.find(password) != file.second.end();
                                   });

    return hasPassword;
}

void IndexingProcessorCls::Add(const std::wstring& password)
{
    std::wstring subfolderName = IndexingProcessorHelperCls::GenerateSubFolderName(password[0]);
    int fileNumber = CalculateFileNumber(subfolderName);

    writablePasswords_[subfolderName][fileNumber][password] = L"User Input";
    passwords_[subfolderName][fileNumber][password] = L"User Input";

    Indexing();
}