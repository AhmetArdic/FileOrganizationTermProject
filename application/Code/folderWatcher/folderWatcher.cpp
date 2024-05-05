#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

#include "folderWatcher.h"

void FolderWatcher::Start() 
{
    std::cout << "Folder Watcher is working..." << std::endl;

    running_ = true;
    thread_ = std::thread([this]() { Watch(); });
}

void FolderWatcher::Stop() 
{
    running_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
}

void FolderWatcher::Watch() 
{
    std::unordered_set<std::string> current_files;
    PopulateFileList(current_files);

    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(INTERVAL));

        std::unordered_set<std::string> new_files;
        PopulateFileList(new_files);

        // Yeni eklenen dosyaları kontrol et
        for (const auto& file : new_files) 
        {
            if (current_files.find(file) == current_files.end()) 
            {
                std::cout << "New file added: " << file << std::endl;
                // callback_(file);
                callback_();
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
                // callback_();
            }
        */

        current_files = std::move(new_files);
    }
}

void FolderWatcher::PopulateFileList(std::unordered_set<std::string>& file_list) const 
{
    for (const auto& entry : std::filesystem::directory_iterator(path_)) 
    {
        if (std::filesystem::is_regular_file(entry)) 
        {
            file_list.insert(entry.path().string());
        }
    }
}