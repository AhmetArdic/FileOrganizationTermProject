#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <filesystem>
#include <chrono>

#include "inc/IndexingProcessor.h"
#include "inc/FolderWatcherCls.h"

namespace fs = std::filesystem;


void Search(IndexingProcessorCls& processor)
{
    while (true)
    {
        std::wstring userInput{};
        std::cout << "Aranacak sifre: ";
        std::wcin >> userInput;

        std::wcout << '"' << userInput << '"' << " araniyor...\n";

        auto start = std::chrono::high_resolution_clock::now();

        if(processor.Search(userInput))
        {
            std::cout << "Bulundu!\n";
        }
        else
        {
            std::cout << "Bulunamadi!!!!\n";

            processor.Add(userInput);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        std::cout << "Elapsed time for total search: " << elapsed.count() << " milisecond" << std::endl;
    }
}



// Rastgele bir dosyadan rastgele bir satır seçmek için bir fonksiyon
std::string randomSelectLineFromRandomFile(const std::wstring& directory) {
    std::vector<std::string> selectedFiles;

    // Dizin içindeki .txt uzantılı dosyaları bul
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.path().extension() == ".txt") {
            selectedFiles.push_back(entry.path().string());
        }
    }

    // Rastgele bir dosya seç
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, selectedFiles.size() - 1); // 0'dan dosya sayısının bir eksiğine kadar rastgele bir indeks seç
    int selectedIndex = dis(gen);
    const std::string& selectedFile = selectedFiles[selectedIndex];

    // Rastgele seçilen dosyadan rastgele bir satır seç
    std::ifstream file(selectedFile); // Dosyayı okuma modunda aç

    // Dosya açılırken bir hata olup olmadığını kontrol et
    if (!file.is_open()) {
        std::cerr << "Dosya acilamadi: " << selectedFile << std::endl;
        return "";
    }

    // Dosyadaki satır sayısını belirle
    int numLines = 0;
    std::string line;
    while (std::getline(file, line)) {
        ++numLines;
    }

    // Rastgele bir satır seç
    std::uniform_int_distribution<> lineDis(1, numLines); // 1'den numLines'a kadar rastgele bir sayı seç
    int selectedLineNum = lineDis(gen);

    // Dosyayı tekrar başa al
    file.clear();
    file.seekg(0);

    // Seçilen satırı bul ve döndür
    int currentLineNum = 0;
    while (std::getline(file, line)) {
        ++currentLineNum;
        if (currentLineNum == selectedLineNum) {
            file.close();
            return line;
        }
    }

    // Dosyayı kapat
    file.close();

    return "";
}

void Test(IndexingProcessorCls& processor, const std::wstring& path)
{
    const int numPasswords = 10; // Oluşturulacak şifre sayısı
    std::vector<std::string> testPasswords{};

    std::cout << "Getting " << numPasswords << " password randomly.\n";
    for (int i = 0; i < numPasswords; ++i) {
        std::string password = randomSelectLineFromRandomFile(path);
        if (!password.empty()) {
            testPasswords.push_back(password);
            std::cout << password << "\n";
        }
    }
    std::cout << "Getting password finished.\n";

    std::cout << "Performance test is starting...\n";

    auto start = std::chrono::high_resolution_clock::now();

    for(const auto& password : testPasswords)
    {
        if(processor.Search(std::wstring(password.begin(), password.end())))
        {
            std::cout << "'" << password << "' bulundu!\n";
        }
        else
        {
            std::cout << "'" << password << "' bulunamadi!!!!\n";

            processor.Add(std::wstring(password.begin(), password.end()));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Elapsed time on performance test: " << elapsed.count() << " milisecond" << std::endl;
}

int main()
{
    const std::wstring processedPasswordsDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\Processed-Passwords";
    const std::wstring unprocessedPasswordsDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\Unprocessed-Passwords";
    const std::wstring indexDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\Index";

    // Hazirlik Asamasi (5 Puan)
    std::filesystem::create_directory(processedPasswordsDir);
    std::filesystem::create_directory(indexDir);

    // Indexleme Islemi (35 Puan)
    IndexingProcessorCls indexingProcessor{indexDir, unprocessedPasswordsDir};
    indexingProcessor.Run();

    // Guncelleme ve Bakim (10 Puan)
    FolderWatcherCls updater(unprocessedPasswordsDir, [objectPtr = &indexingProcessor](auto&& PH1) { objectPtr->HandleNewPassword(std::forward<decltype(PH1)>(PH1)); });
    updater.Run();

    // Arama Fonksiyonu (25 Puan)
//    Search(indexingProcessor);

    // Performans Testi
    Test(indexingProcessor, unprocessedPasswordsDir);
}
