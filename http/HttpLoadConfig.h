#ifndef _HTTPLOADCONFIG_H_
#define _HTTPLOADCONFIG_H_

#include "../logger/Logger.h"
#include "../load_config/LoadConfig.h"

class HttpLoadConfig:public LoadConfig
{
public:
    HttpLoadConfig() = default;
};

#endif // _HTTPLOADCONFIG_H_