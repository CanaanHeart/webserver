#ifndef _LOG_H_
#define _LOG_H_

#include "FileManager.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <map>
#include <mutex>
#include <iomanip>

#define MB2B 1048576

#define RED "\e[1;31m"
#define GREEN "\e[1;32m"
#define YELLOW "\e[1;33m"
#define BLUE "\e[1;34m"
#define PURPLE "\e[1;35m"
#define CYAN "\e[1;36m"
#define WHITE "\e[1;37m"
#define DEFA "\e[0m"

const std::string  SQUARE_BRACKETS_LEFT  = " [";
const std::string  SQUARE_BRACKETS_RIGHT = "] ";
const std::string SPACE = " ";
const std::string LINE_FEED = "\n";
const std::string COLON  = ":";
const std::string SLASH = "/";
const std::string SWITCH_OFF = "off";
const std::string SWITCH_ON = "on";

enum class CoutType:int {FATAL, ERROR, WARN, INFO, DEBUG};
enum class FileType:int {FATAL, ERROR, WARN, INFO, DEBUG};
enum class TerminalType:int {FATAL, ERROR, WARN, INFO, DEBUG};

template <typename T>
void msgs_comb(std::ostringstream &oss, T t)
{
    oss << t;
}

template <typename T, typename... Args>
void msgs_comb(std::ostringstream &oss, T t, Args... args)
{
    oss << t;
    msgs_comb(oss, args...);
}

struct LogSwitch{
    std::string logSwitch;              // 日志开关
    std::string logFileSwitch;          // 日志是否写入文件
    std::string logTerminalSwitch;      // 日志是否打印到终端
    std::string logFileQueueSwitch;     // 是否开启队列策略
    std::string logFileQueueSize;       // 缓冲队列大小
    std::string logName;                // 日志文件名
    std::string logFilePath;            // 日志文件保存路径
    std::string logMaxSize;             // 日志文件最大大小
    std::string logBehavior;            // 日志文件达到最大的行为
    std::string logOutputLevelFile;     // 日志输出等级
    std::string logOutputLevelTerminal; // 日志输出等级
};

class Logger{
private:
    LogSwitch log_switch_;
    std::map<std::string, std::string> cout_color_;
    std::map<CoutType, std::string> cout_type_map_;
    std::map<FileType, bool> file_type_map_;
    std::map<TerminalType, bool> terminal_type_map_;
    FileManager fmer_;
    static std::mutex mtx_file_;
    static std::mutex mtx_queue_;
    std::queue<std::string> qu_;
public:
    static std::mutex mtx_terminal_;
private:
    Logger();
    ~Logger();
public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
public:
    static Logger& GetInstance();
    void Init();
    void InitCoutColor();
    void InitCoutMap();
    void InitLogConfig();
    void InitFileMap();
    void InitTerminalMap();
    void ConfInfoPrint();
    std::string GetLogNameTime();   // 获取调用时间，用来组成日志文件名
    std::string GetLogCoutTime();   // 获取调用时间，作为输出的时间
    std::string GetLogCoutProcessID();  // 获取进程id
    std::string GetLogCoutThreadID();   // 获取线程id
    std::string GetLogCoutUserName();   // 获取用户名
    std::string GetCoutTypeColor(std::string cout_type);
    std::string GetCoutType(CoutType ctype);
    bool GetFileType(FileType ftype);
    bool GetTerminalType(TerminalType ttype);
    std::string GetLogSwitch();
    std::string GetLogFileSwitch();
    std::string GetLogTerminalSwitch();
    std::string GetCurrentPath();   // 获取当前进程工作路径名
    std::string GetLogFilePath();   // 获取日志文件的保存路径
    std::string GetLogFileName();   // 获取日志文件名
    std::string GetLogFileFullName();   // 获取日志文件绝对路径+文件全名
    std::string GetLogFileNewName();    // 获取日志文件绝对路径+新的带有时间戳的文件名
    bool LogFileWrite(std::string msgs, std::string msg, std::string line);
    void BindFileMap(std::string v1, FileType v2);
    void BindTerminalMap(std::string v1, TerminalType v2);
    bool InsertQueue(std::string msgs, std::string filename);
};

#define KV(value) " " << #value << "=" << value

#define __GTIME__ Logger::GetInstance().GetLogCoutTime()
#define __GPID__ Logger::GetInstance().GetLogCoutProcessID()
#define __GTID__ Logger::GetInstance().GetLogCoutThreadID()
#define __GUSER__ Logger::GetInstance().GetLogCoutUserName()
#define __GFILE__ __FILE__
#define __GFUNC__ __func__
#define __GLINE__ __LINE__
#define __GNAME__(name) #name

#define __FATAL__ __GNAME__(FATAL)
#define __ERROR__ __GNAME__(ERROR)
#define __WARN__ __GNAME__(WARN)
#define __INFO__ __GNAME__(INFO)
#define __DEBUG__ __GNAME__(DEBUG)

#define INFO_FILE(InfoType, ...) \
    do{\
        std::string msgs = __GTIME__ + InfoType + __GUSER__ + __GTID__ + SQUARE_BRACKETS_LEFT +\
            __GFILE__ + SPACE + __GFUNC__ + COLON + std::to_string(__GLINE__) + SQUARE_BRACKETS_RIGHT;\
        std::ostringstream oss;\
        msgs_comb(oss, __VA_ARGS__);\
        Logger::GetInstance().LogFileWrite(msgs, oss.str(), LINE_FEED);\
    }while(0);


#define INFO_TERMINAL(InfoType, ...) \
    do{\
        std::string color = Logger::GetInstance().GetCoutTypeColor(InfoType);\
        std::string msgs = __GTIME__ + color + InfoType + DEFA + __GUSER__ + __GTID__ + SQUARE_BRACKETS_LEFT +\
            __GFILE__ + SPACE + __GFUNC__ + COLON + std::to_string(__GLINE__) + SQUARE_BRACKETS_RIGHT;\
        std::ostringstream oss;\
        msgs_comb(oss, __VA_ARGS__);\
        std::unique_lock<std::mutex> uq(Logger::mtx_terminal_);\
        std::cout << msgs << oss.str() << LINE_FEED;\
    }while(0);


#define LOG_COUT(CoutType, InfoType, FileType, TerminalType, ...) \
    do{\
        std::string cout_type = Logger::GetInstance().GetCoutType(CoutType);\
        if(Logger::GetInstance().GetLogSwitch() == SWITCH_ON){\
            if(Logger::GetInstance().GetLogFileSwitch() == SWITCH_ON)\
                    INFO_FILE(InfoType, __VA_ARGS__);\
            if(Logger::GetInstance().GetLogTerminalSwitch() == SWITCH_ON)\
                if(Logger::GetInstance().GetTerminalType(TerminalType))\
                    INFO_TERMINAL(InfoType, __VA_ARGS__);\
        }\
    }while(0);

#define FATAL(...) \
    do{\
        LOG_COUT(CoutType::FATAL, __FATAL__, FileType::FATAL, TerminalType::FATAL, __VA_ARGS__)\
        exit(EXIT_FAILURE);\
    }while(0);

#define ERROR(...) \
    do{\
        LOG_COUT(CoutType::ERROR, __ERROR__, FileType::ERROR, TerminalType::ERROR, __VA_ARGS__)\
        exit(EXIT_FAILURE);\
    }while(0);

#define WARN(...) \
    do{\
        LOG_COUT(CoutType::WARN, __WARN__, FileType::WARN, TerminalType::WARN, __VA_ARGS__)\
    }while(0);

#define INFO(...) \
    do{\
        LOG_COUT(CoutType::INFO, __INFO__, FileType::INFO, TerminalType::INFO, __VA_ARGS__)\
    }while(0);

#define DEBUG(...) \
    do{\
        LOG_COUT(CoutType::DEBUG, __DEBUG__, FileType::DEBUG, TerminalType::DEBUG, __VA_ARGS__)\
    }while(0);

#endif // _LOG_H_