#include <iostream>

#include "IndexingProcessor.h"
#include "FolderWatcherCls.h"

int main(void) 
{
    const std::wstring unprocessedPasswordsDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Unprocessed-Passwords";
    const std::wstring indexDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Index";

    IndexingProcessorCls indexingProcessor{indexDir, unprocessedPasswordsDir};
    indexingProcessor.Run();

    FolderWatcherCls updater(unprocessedPasswordsDir, std::bind(&IndexingProcessorCls::HandleNewPassword, &indexingProcessor, std::placeholders::_1));
    updater.Run();

    while (true) 
    {
        std::wstring userInput{};
        std::cout << "Aranacak sifre: ";
        std::wcin >> userInput;

        std::wcout << '"' << userInput << '"' << " araniyor...\n";
        if(indexingProcessor.Search(userInput))
        {
            std::cout << "Bulundu!\n";
        }
        else
        {
            std::cout << "Bulunamadi!!!!\n";
            
            indexingProcessor.Add(userInput);
        }
    }
    
    return 0;
}
