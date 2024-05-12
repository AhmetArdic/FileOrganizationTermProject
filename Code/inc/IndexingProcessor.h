#ifndef _INDEXING_PROCESSOR_H_
#define _INDEXING_PROCESSOR_H_

#include <fstream>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <utility>

#include "md5.h"
#include "sha256.h"
#include "sha1.h"

class IndexingProcessorCls
{
private:
    int CalculateFileNumber(const std::wstring& key);

    void ScanIndex(const std::filesystem::path& path);

    void ScanFile(const std::filesystem::path& path);

    void ScanFolder(const std::wstring& path, std::function<void(const std::filesystem::path&)> callback);

    void Indexing();

public:
    IndexingProcessorCls(std::wstring  indexDir, std::wstring  unprocessedPasswordsDir) : indexDir{std::move(indexDir)}, unprocessedPasswordsDir{std::move(unprocessedPasswordsDir)} {}

    void HandleNewPassword(const std::wstring& filePath);

    void Run();

    bool Search(const std::wstring& password);
    bool SearchInIndexFolder(const std::wstring& password);

    void Add(const std::wstring& password);

private:
    static constexpr int MAX_LINE_NUMBER = 10000; 

private:
    const std::wstring indexDir;
    const std::wstring unprocessedPasswordsDir;

    struct FileInfoStruct
    {
        int totalLineNumberOfCurrentFile;
        int totalFileNumberOfIndexSubfolder;
    };

    std::unordered_map<std::wstring, FileInfoStruct> fileInfo_;

    //first: subfolderName, second.first: file number, second.second.first: password, second.second.second: path 
    std::unordered_map<std::wstring, std::unordered_map<int, std::unordered_map<std::wstring, std::wstring>>> passwords_;
    std::unordered_map<std::wstring, std::unordered_map<int, std::unordered_map<std::wstring, std::wstring>>> writablePasswords_;
};

class IndexingProcessorHelperCls
{
public:
    static std::wstring CalculateMD5Hash(std::wstring password)
    {
        static MD5 md5;

        std::string result = md5(std::string(password.begin(), password.end()));
        return { result.begin(), result.end() };
    }

    static std::wstring CalculateSha128(std::wstring password)
    {
        static SHA1 sha1;
        
        std::string result = sha1(std::string(password.begin(), password.end()));
        return { result.begin(), result.end() };
    }

    static std::wstring CalculateSha256(std::wstring password)
    {
        static SHA256 sha256;

        std::string result = sha256(std::string(password.begin(), password.end()));
        return { result.begin(), result.end() };
    }

    static void WriteToFile(const std::wstring& line, const std::filesystem::path& path)
    {
        std::wofstream outputFile(path, std::ios::app);  // ilgili dosyayi append modunda ac
        outputFile << line << std::endl;
        outputFile.close(); // dosyayi kapat
    }

    static std::wstring ExtractSubstringAfterLastDelimiter(const std::wstring& str, wchar_t delimiter)
    {
        size_t lastPipePos = str.rfind(delimiter);

        if (lastPipePos == std::string::npos) {
            return L"";
        }

        return str.substr(lastPipePos + 1);
    }

    static std::wstring ExtractSubstringBeforeFirstDelimiter(const std::wstring& str, wchar_t delimiter) 
    {
        size_t pos = str.find(delimiter);
        if (pos != std::string::npos) {
            return str.substr(0, pos);
        }
        return str;
    }

    static std::wstring GetSubstringBetweenDelimiters(const std::wstring& str, wchar_t startDelimiter, wchar_t endDelimiter) 
    {
        auto endPos = str.rfind(endDelimiter);
        if (endPos == std::wstring::npos) 
        {
            return L"";
        }

        auto startPos = str.rfind(startDelimiter, endPos - 1);
        if (startPos == std::wstring::npos) 
        {
            return L"";
        }

        return str.substr(startPos + 1, endPos - startPos - 1);
    }

    static int ExtractIntegerFromString(const std::wstring& str) 
    {
        int result = 0;

        for (wchar_t c : str)
        {
            if (iswdigit(c))
            {
                result = result * 10 + (c - '0');
            }
        }

        return result;
    }

    static std::wstring InvalidSignConvertor(wchar_t c)
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
            return { c };
        }
    }

    static std::wstring GenerateSubFolderName(wchar_t firstChar)
    {
        if(!iswalpha(firstChar) && !iswdigit(firstChar))
        {
            return L"@(" + InvalidSignConvertor(firstChar) + L")";
        }
        else if(iswupper(firstChar))
        {
            return L"_" + std::wstring(1, firstChar) + L"_";
        }
        else
        {
            return { firstChar };
        }
    }
};

#endif /* _INDEXING_PROCESSOR_H_ */
