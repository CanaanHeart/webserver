#ifndef _HTTPLOADCONFIG_H_
#define _HTTPLOADCONFIG_H_

#include "../logger/Logger.h"
#include "../load_config/LoadConfig.h"

class HttpLoadConfig
{
private:
    LoadConfig lc_;
public:
    HttpLoadConfig() = default;
    void LoadConfigFile(const std::string &file);
    std::string GetValue(const std::string &key);
};

#endif // _HTTPLOADCONFIG_H_