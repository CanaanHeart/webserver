#ifndef __THREADLOADCONFIG_H__
#define __THREADLOADCONFIG_H__

#include "../load_config/LoadConfig.h"

class ThreadLoadConfig
{
private:
    LoadConfig lc_;
public:
    void LoadConfigFile(const std::string &file);
    std::string GetValue(const std::string &key);
};

#endif /* __THREADLOADCONFIG_H__ */