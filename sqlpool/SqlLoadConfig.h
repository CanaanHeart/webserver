#ifndef _SQLLOADCONFIG_H_
#define _SQLLOADCONFIG_H_

#include "../load_config/LoadConfig.h"

class SqlLoadConfig:public LoadConfig
{
public:
    SqlLoadConfig() = default;
};

#endif // _SQLLOADCONFIG_H_