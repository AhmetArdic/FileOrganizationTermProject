#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <locale>

#include <chrono>

#include "IndexingProcessor.h"

void IndexingProcessorCls::WriteToFile(const std::wstring& line, const std::filesystem::path& path)
{
    std::wofstream outputFile(path, std::ios::app);  // ilgili dosyayi append modunda ac
    outputFile << line << std::endl;
    outputFile.close(); // dosyayi kapat
}

std::wstring IndexingProcessorCls::MakeSubFolder(const std::wstring& path)
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
            const auto& path = IndexingProcessorHelperCls::ExtractSubstringAfterLastDelimiter(line, '|');

            passwords_[subfolderName][fileNumber][password] = path;
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

            std::wstring subFolderPath = MakeSubFolder(indexDir + L"\\" + item.first);   

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

void IndexingProcessorCls::Run(void)
{   
    ScanFolder(indexDir, std::bind(&IndexingProcessorCls::ScanIndex, this, std::placeholders::_1));
    
    std::thread indexingThread([this]() {
        ScanFolder(unprocessedPasswordsDir, std::bind(&IndexingProcessorCls::ScanFile, this, std::placeholders::_1));
        Indexing();
    });
    indexingThread.detach();
}

bool IndexingProcessorCls::Search(const std::wstring& password)
{
    for(const auto& file : passwords_[IndexingProcessorHelperCls::GenerateSubFolderName(password[0])])
    {
        if(file.second.find(password) != file.second.end())
        {
            return true;
        }
    }

    return false;
}

void IndexingProcessorCls::Add(const std::wstring& password)
{
    std::wstring subfolderName = IndexingProcessorHelperCls::GenerateSubFolderName(password[0]);
    int fileNumber = CalculateFileNumber(subfolderName);

    writablePasswords_[subfolderName][fileNumber][password] = L"User Input";
    passwords_[subfolderName][fileNumber][password] = L"User Input";

    Indexing();
}