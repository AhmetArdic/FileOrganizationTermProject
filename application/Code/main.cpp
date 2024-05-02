#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <unordered_map>

#include <thread>
#include <vector>

#define _DEBUG

const std::wstring CalculateMD5Hash(void)
{
    return L"MD5Hash";
}

const std::wstring CalculateSha128(void)
{
    return L"Sha128";
}


const std::wstring CalculateSha256(void)
{
    return L"Sha256";
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

    void Scanning(const std::filesystem::directory_entry& entry)
    {
        std::wifstream inputFile(entry.path());  // uzerinde islem yapiacak dosyayi okuma modunda ac
        std::wstring line;

        // okuma modunda acilan dosyanin satirlarında dolas
        while (std::getline(inputFile, line)) 
        {
            if (!line.empty())  
            {
                // eger satir bos degilse

                std::wstring subfolderName = GenerateSubFolderName(line[0]);

#ifdef DEBUG    
                beforeFilterForCaseSensitivityAndDuplications[subfolderName]++;
#endif /* DEBUG */

                passwords_[subfolderName][line] = entry.path().filename().wstring();
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

                    std::wstring subFolderPath = MakeSubFolder(indexDir + L"\\" + item.first);

                    std::wstring fileName = L"passwords" + std::to_wstring(fileNumber) + L".txt";   // okunan sifrenin yazilacagi dosyanin ismi

                    std::wstring line = password.first + L"|" + CalculateMD5Hash() + L"|" + CalculateSha128() + L"|" + CalculateSha256() + L"|" + password.second; 
                    WriteToFile(line, subFolderPath + L"\\" + fileName);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

public:
    IndexingProcessorCls(const std::wstring& indexDir, const std::wstring& unprocessedPasswordsDir) : indexDir{indexDir}, unprocessedPasswordsDir{unprocessedPasswordsDir} {}

    void Run(void)
    {
        // unprocessedPasswordsDir dizininde dolas
        for (const auto& entry : std::filesystem::directory_iterator(unprocessedPasswordsDir)) 
        {
            if (entry.is_regular_file())
            {
                // dizindeki obje regular file ise

#ifdef DEBUG
                std::cout << entry.path() << std::endl; // uzerinde islem yapilan dosya
#endif /* DEBUG */

                Scanning(entry);
            }
        }

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
    std::unordered_map<std::wstring, std::unordered_map<std::wstring, std::wstring>> passwords_;

#ifdef DEBUG
    std::unordered_map<std::wstring, int> beforeFilterForCaseSensitivityAndDuplications;
    std::unordered_map<std::wstring, int> afterFilterForCaseSensitivityAndDuplications;
#endif /* DEBUG */
};

#include <chrono>
int main(void) 
{
    auto start = std::chrono::high_resolution_clock::now();

    const std::wstring unprocessedPasswordsDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Unprocessed-Passwords";
    const std::wstring indexDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Index";

    IndexingProcessorCls indexingProcessor{indexDir, unprocessedPasswordsDir};
    indexingProcessor.Run();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " milisecond" << std::endl;

    return 0;
}
