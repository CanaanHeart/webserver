#include "HttpLoadConfig.h"

using namespace std;

void HttpLoadConfig::LoadConfigFile(const std::string &file)
{
    lc_.LoadConfigFile(file);
}

std::string HttpLoadConfig::GetValue(const std::string &key)
{
    return lc_.GetValue(key);
}

