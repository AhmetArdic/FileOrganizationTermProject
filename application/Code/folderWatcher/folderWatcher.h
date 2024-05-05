#include <functional>
#include <unordered_set>

class FolderWatcher 
{
public:
    using CallbackFunction = std::function<void(void)>;
    // using CallbackFunction = std::function<void(const std::string&)>;

public:
    FolderWatcher(const std::string& path, CallbackFunction callback) : path_(path), callback_(callback), running_(false) {}

    void Start();

    void Stop();

private:
    void Watch();

    void PopulateFileList(std::unordered_set<std::string>& file_list) const;

private:
    static constexpr int INTERVAL = 1;

    std::string path_;
    CallbackFunction callback_;
    std::thread thread_;
    bool running_;
};