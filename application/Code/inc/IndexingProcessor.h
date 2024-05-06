#ifndef _INDEXING_PROCESSOR_H_
#define _INDEXING_PROCESSOR_H_

#include <filesystem>
#include <unordered_map>
#include <functional>

#include "md5.h"
#include "sha256.h"
#include "sha1.h"

class IndexingProcessorCls
{
private:
    void WriteToFile(const std::wstring& line, const std::filesystem::path& path);

    std::wstring MakeSubFolder(const std::wstring& path);

    int CalculateFileNumber(const std::wstring& key);

    void ScanIndex(const std::filesystem::path& path);

    void ScanFile(const std::filesystem::path& path);

    void ScanFolder(const std::wstring& path, std::function<void(const std::filesystem::path&)> callback);

    void Indexing();

public:
    IndexingProcessorCls(const std::wstring& indexDir, const std::wstring& unprocessedPasswordsDir) : indexDir{indexDir}, unprocessedPasswordsDir{unprocessedPasswordsDir} {}

    void HandleNewPassword(const std::wstring& filePath);

    void Run(void);

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
};

class IndexingProcessorHelperCls
{
public:
    static std::wstring CalculateMD5Hash(std::wstring password)
    {
        static MD5 md5;

        std::string result = md5(std::string(password.begin(), password.end()));
        return std::wstring(result.begin(), result.end()) ;
    }

    static std::wstring CalculateSha128(std::wstring password)
    {
        static SHA1 sha1;
        
        std::string result = sha1(std::string(password.begin(), password.end()));
        return std::wstring(result.begin(), result.end()) ;
    }


    static std::wstring CalculateSha256(std::wstring password)
    {
        static SHA256 sha256;

        std::string result = sha256(std::string(password.begin(), password.end()));
        return std::wstring(result.begin(), result.end()) ;
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

        for (char c : str) 
        {
            if (std::isdigit(c)) 
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
            return std::wstring(1, c);
        }
    }

    static std::wstring GenerateSubFolderName(wchar_t firstChar)
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
};

#endif /* _INDEXING_PROCESSOR_H_ */
