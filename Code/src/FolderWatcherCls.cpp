#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

#include "FolderWatcherCls.h"

void FolderWatcherCls::Run() 
{
    std::cout << "Folder Watcher is working..." << std::endl;

    running_ = true;
    thread_ = std::thread([this]() { 
        Watch(); 
    });
    thread_.detach();
}

void FolderWatcherCls::Stop() 
{
    running_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void FolderWatcherCls::Watch() 
{
    std::unordered_set<std::wstring> current_files;
    PopulateFileList(current_files);

    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(INTERVAL));

        std::unordered_set<std::wstring> new_files;
        PopulateFileList(new_files);

        // Yeni eklenen dosyaları kontrol et
        for (const auto& file : new_files) 
        {
            if (current_files.find(file) == current_files.end()) 
            {
                std::wcout << "New file added: " << file << std::endl;
                callback_(file);
            }
        }

        // Kaldırılan dosyaları kontrol et
        /*
        for (const auto& file : current_files) 
        {
            if (new_files.find(file) == new_files.end()) 
            {
                std::cout << "File removed: " << file << std::endl;
                callback_(file);
            }
        */

        current_files = std::move(new_files);
    }
}

void FolderWatcherCls::PopulateFileList(std::unordered_set<std::wstring>& file_list) const 
{
    for (const auto& entry : std::filesystem::directory_iterator(path_)) 
    {
        if (std::filesystem::is_regular_file(entry)) 
        {
            file_list.insert(entry.path().wstring());
        }
    }
}