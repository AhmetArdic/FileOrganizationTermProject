#include <iostream>
#include <filesystem>
#include <chrono>

#include "inc/IndexingProcessor.h"
#include "inc/FolderWatcherCls.h"


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
    Search(indexingProcessor);

}
