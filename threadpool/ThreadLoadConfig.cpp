#include "ThreadLoadConfig.h"

using namespace std;

void ThreadLoadConfig::LoadConfigFile(const string &file)
{
    lc_.LoadConfigFile(file);
}

string ThreadLoadConfig::GetValue(const string &key)
{
    return lc_.GetValue(key);
}