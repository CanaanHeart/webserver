#ifndef _LOAD_CONFIG_H_
#define _LOAD_CONFIG_H_

#include "../logger/Logger.h"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <algorithm>

// 读取配置模块
class LoadConfig
{
private:
    std::unordered_map<std::string, std::string> conf_;

public:
    LoadConfig() = default;
    void LoadConfigFile(const std::string &file);
    std::string GetValue(const std::string &key);
};

#endif // _LOAD_CONFIG_H_