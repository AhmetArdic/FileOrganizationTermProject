#include <iostream>

#include "IndexingProcessor.h"
#include "FolderWatcherCls.h"

int main(void) 
{
    const std::wstring unprocessedPasswordsDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Unprocessed-Passwords";
    const std::wstring indexDir = L"C:\\Users\\AhmetArdic\\Desktop\\FileOrgTermProject\\application\\Index";

    IndexingProcessorCls indexingProcessor{indexDir, unprocessedPasswordsDir};
    indexingProcessor.Run();

    FolderWatcherCls watcher(unprocessedPasswordsDir, std::bind(&IndexingProcessorCls::HandleNewPassword, &indexingProcessor, std::placeholders::_1));
    watcher.Start();

    while (true) {}
    
    return 0;
}
