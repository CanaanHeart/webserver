#include "LoadConfig.h"

using namespace std;

void LoadConfig::LoadConfigFile(const string &file)
{
    ifstream ifs;
    ifs.open(file);
    if(!ifs.is_open()){
        ERROR("file:", file, " is not exist!");
    }
    string line;
    string key, value;
    while(getline(ifs, line)){
        if(line.empty() || line[0] == '#')
            continue;
        size_t pos = line.find('=');
        if(pos == string::npos){
            WARN("config file format is incorrect!");
            continue;
        }
        size_t pos_le = pos - 1, pos_ri = pos + 1;
        while(pos_le > 0){
            if(line[pos_le] == ' ')
                --pos_le;
            else
                break;
        }
        while(pos_ri < line.size()){
            if(line[pos_ri] == ' ')
                ++pos_ri;
            else
                break;
        }
        key = line.substr(0, pos_le + 1);
        value = line.substr(pos_ri);
        conf_[key] = value;
    }
}

string LoadConfig::GetValue(const string &key)
{
    auto it = conf_.find(key);
    if(it == conf_.end())
        return "";
    else
        return it->second;
}