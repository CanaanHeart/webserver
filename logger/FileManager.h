#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

#include <iostream>
#include <string>
#include <fstream>
#include <mutex>

class FileManager{
private:
    std::mutex mtx_;
public:
    FileManager() = default;
    ~FileManager() {}
    bool CreateFilePath(std::string path);
    bool CreateFile(std::string filename);
    bool IsFileExist(std::string filename);
    bool FileRename(std::string oldname, std::string newname);
    long FileSize(std::string filename);
};

#endif // _FILEMANAGER_H_