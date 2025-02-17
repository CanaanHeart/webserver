#include "FileManager.h"

using namespace std;

bool FileManager::CreateFilePath(string path)
{
    DIR *mydir = nullptr;

    unique_lock<mutex> uq(mtx_);

    mydir = opendir(path.c_str());

    if(!mydir){
        

        int k = mkdir(path.c_str(), 0755);

        uq.unlock();

        if(k != 0)
            return false;
        else
            return true;
    }
    else
        return true;
}

bool FileManager::CreateFile(string filename)
{
    ofstream ofs;

    unique_lock<mutex> uq(mtx_);

    ofs.open(filename, ios::app | ios::out);

    if(!ofs){
        cout << "create file error!" << endl;
        return false;
    }

    return true;
}

bool FileManager::IsFileExist(string filename)
{
    unique_lock<mutex> uq(mtx_);
    ifstream ifs(filename);
    return ifs.good();
}

bool FileManager::FileRename(string oldname, string newname)
{
    unique_lock<mutex> uq(mtx_);

    int k = rename(oldname.c_str(), newname.c_str());

    if(k != 0)
        return false;
    else
        return true;
}

long FileManager::FileSize(string filename)
{
    struct stat buf;

    unique_lock<mutex> uq(mtx_);

    int k = stat(filename.c_str(), &buf);
    if(k != 0)
        return -1;
    else
        return buf.st_size;
}