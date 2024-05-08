#ifndef _FOLDER_WATCHER_H_
#define _FOLDER_WATCHER_H_

#include <functional>
#include <unordered_set>
#include <thread>

class FolderWatcherCls 
{
public:
    using CallbackFunction = std::function<void(const std::wstring&)>;

public:
    FolderWatcherCls(const std::wstring& path, CallbackFunction callback) : path_(path), callback_(callback), running_(false) {}

    void Run();

    void Stop();

private:
    void Watch();

    void PopulateFileList(std::unordered_set<std::wstring>& file_list) const;

private:
    static constexpr int INTERVAL = 1;

    std::wstring path_;
    CallbackFunction callback_;
    std::thread thread_;
    bool running_;
};

#endif /* _FOLDER_WATCHER_H_ */
