#ifndef __THREADLOADCONFIG_H__
#define __THREADLOADCONFIG_H__

#include "../load_config/LoadConfig.h"

class ThreadLoadConfig:public LoadConfig
{
private:
    LoadConfig lc_;
public:
    ThreadLoadConfig() = default;
};

#endif /* __THREADLOADCONFIG_H__ */