#include "Logger.h"

std::mutex Logger::mtx_file_;
std::mutex Logger::mtx_queue_;
std::mutex Logger::mtx_terminal_;

using namespace std;

Logger::Logger()
{
    Init();
}

Logger::~Logger()
{

}

Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

void Logger::Init()
{
    InitCoutColor();
    InitCoutMap();
    InitLogConfig();
    InitFileMap();
    InitTerminalMap();

    string filename = GetLogFileFullName();
    string filepath = GetLogFilePath();

    if(log_switch_.logFileSwitch == SWITCH_ON){
        fmer_.CreateFilePath(filepath);
        if(!fmer_.IsFileExist(filename)){
            fmer_.CreateFile(filename);
        }
        else{
            long filesize = fmer_.FileSize(filename);
            if(filesize > stol(log_switch_.logMaxSize) * MB2B){
                string new_filename = GetLogFileNewName();
                int k = fmer_.FileRename(filename, new_filename);
                if(!k)
                    cout << "file rename failed!" << endl;
                
                fmer_.CreateFile(filename);
            }
        }
    }
}

void Logger::InitCoutColor()
{
    cout_color_["FATAL"] = string(RED);
    cout_color_["ERROR"] = string(YELLOW);
    cout_color_["WARN"] = string(PURPLE);
    cout_color_["INFO"] = string(BLUE);
    cout_color_["DEBUG"] = string(GREEN);
}

void Logger::InitCoutMap()
{
    cout_type_map_[CoutType::FATAL] = "FATAL";
    cout_type_map_[CoutType::ERROR] = "ERROR";
    cout_type_map_[CoutType::WARN] = "WARN";
    cout_type_map_[CoutType::INFO] = "INFO";
    cout_type_map_[CoutType::DEBUG] = "DEBUG";
}

void Logger::InitLogConfig()
{
    map<string, string*> flogConfInfo;

    flogConfInfo["logSwitch"] = &log_switch_.logSwitch;
    flogConfInfo["logFileSwitch"] = &log_switch_.logFileSwitch;
    flogConfInfo["logTerminalSwitch"] = &log_switch_.logTerminalSwitch;
    flogConfInfo["logFileQueueSwitch"] = &log_switch_.logFileQueueSwitch;
    flogConfInfo["logFileQueueSize"] = &log_switch_.logFileQueueSize;
    flogConfInfo["logName"] = &log_switch_.logName;
    flogConfInfo["logFilePath"] = &log_switch_.logFilePath;
    flogConfInfo["logMaxSize"] = &log_switch_.logMaxSize;
    flogConfInfo["logBehavior"] = &log_switch_.logBehavior;
    flogConfInfo["logOutputLevelFile"] = &log_switch_.logOutputLevelFile;
    flogConfInfo["logOutputLevelTerminal"] = &log_switch_.logOutputLevelTerminal;

    string line;
    ifstream ifs;

    ifs.open("../conf/log.conf");
    if(!ifs.is_open())
        cout << "file log.conf open fail!" << endl;

    while(getline(ifs, line)){
        if(line.empty() || line[0] == '#')
            continue;

        string line_copy;

        for(size_t i = 0; i < line.size(); ++i){
            if(line[i] == ' ')
                continue;
            line_copy += line[i];
        }

        size_t pos = line_copy.find('=');
        if(pos == std::string::npos){
            cout << "config file error!" << endl;
            continue;
        }

        string key_str = line_copy.substr(0, pos);
        string value_str = line_copy.substr(pos + 1);

        auto it = flogConfInfo.find(key_str);
        if(it != flogConfInfo.end())
            *(it->second) = value_str;
    }

    ifs.close();
}

void Logger::InitFileMap()
{
    BindFileMap("5", FileType::FATAL);
    BindFileMap("4", FileType::ERROR);
    BindFileMap("3", FileType::WARN);
    BindFileMap("2", FileType::INFO);
    BindFileMap("1", FileType::DEBUG);
}

void Logger::InitTerminalMap()
{
    BindTerminalMap("5", TerminalType::FATAL);
    BindTerminalMap("4", TerminalType::ERROR);
    BindTerminalMap("3", TerminalType::WARN);
    BindTerminalMap("2", TerminalType::INFO);
    BindTerminalMap("1", TerminalType::DEBUG);
}

void Logger::ConfInfoPrint()
{
    cout << CYAN << ::left << setw(25) << "  日志开关" << log_switch_.logSwitch << DEFA << endl;
    cout << CYAN << ::left << setw(25) << "  文件输出" << log_switch_.logFileSwitch << DEFA << endl;
    cout << CYAN << ::left << setw(25) << "  终端输出开关" << log_switch_.logTerminalSwitch << DEFA << endl;
    cout << CYAN << ::left << setw(25) << "  日志队列策略" << log_switch_.logTerminalSwitch << DEFA << endl;
    cout << CYAN << ::left << setw(25) << "  文件输出等级" << log_switch_.logOutputLevelFile << DEFA << endl;    
    cout << CYAN << ::left << setw(25) << "  终端输出等级" << log_switch_.logOutputLevelTerminal << DEFA << endl;    
    cout << CYAN << ::left << setw(25) << "  日志文件名称" << log_switch_.logName << DEFA << endl;
    cout << CYAN << ::left << setw(25) << "  日志保存路径" << log_switch_.logFilePath << DEFA << endl;
    cout << CYAN << ::left << setw(25) << "  日志文件大小" << log_switch_.logMaxSize << "M" << DEFA << endl;
}

string Logger::GetLogNameTime()
{
    time_t present_time;
    time(&present_time);

    char tmp_buf[64];
    strftime(tmp_buf, sizeof(tmp_buf), "%Y-%m-%d-%H:%M:%S", localtime(&present_time));

    return string(tmp_buf);
}

string Logger::GetLogCoutTime()
{
    time_t present_time;
    time(&present_time);

    char tmp_buf[64];
    strftime(tmp_buf, sizeof(tmp_buf), "%Y-%m-%d-%H:%M:%S", localtime(&present_time));

    return SQUARE_BRACKETS_LEFT + string(tmp_buf) + SQUARE_BRACKETS_RIGHT;
}

string Logger::GetLogCoutProcessID()
{
    return to_string(getpid());
}

string Logger::GetLogCoutThreadID()
{
    return to_string(gettid());
}

string Logger::GetLogCoutUserName()
{
    struct passwd *my_info;
    my_info = getpwuid(getuid());
    string name = string(my_info->pw_name);
    return SPACE + name + SPACE;
}

string Logger::GetCoutTypeColor(std::string cout_type)
{
    return cout_color_[cout_type];
}

string Logger::GetCoutType(CoutType ctype)
{
    return cout_type_map_[ctype];
}

bool Logger::GetFileType(FileType ftype)
{
    return file_type_map_[ftype];
}

bool Logger::GetTerminalType(TerminalType ttype)
{
    return terminal_type_map_[ttype];
}

string Logger::GetLogSwitch()
{
    return log_switch_.logSwitch;
}

string Logger::GetLogFileSwitch()
{
    return log_switch_.logFileSwitch;
}

string Logger::GetLogTerminalSwitch()
{
    return log_switch_.logTerminalSwitch;
}

string Logger::GetCurrentPath()
{
    char buf[1024];
    char *ret = getcwd(buf, sizeof(buf));
    if(!ret){
        cout << "getcwd error!" << endl;
        exit(EXIT_FAILURE);
    }

    return static_cast<string>(buf); 
}

string Logger::GetLogFilePath()
{
    return log_switch_.logFilePath;
}

string Logger::GetLogFileName()
{
    return log_switch_.logName;
}

string Logger::GetLogFileFullName()
{
    return GetLogFilePath() + SLASH + GetLogFileName() + ".log";
}

string Logger::GetLogFileNewName()
{
    return GetLogFilePath() + SLASH + GetLogFileName() + "_" + GetLogNameTime() + ".log";
}

bool Logger::LogFileWrite(string msgs, string msg, string line)
{
    string filename = GetLogFileFullName();
    long filesize = fmer_.FileSize(filename);
 
    if((filesize > stol(log_switch_.logMaxSize) * MB2B) && log_switch_.logBehavior == "1" ){
        string new_filename = GetLogFileNewName();
        int k = fmer_.FileRename(filename, new_filename);
        if(!k)
            cout << "file rename failed!" << endl;

        fmer_.CreateFilePath(GetLogFilePath());
        fmer_.CreateFile(filename);
    }

    if(log_switch_.logFileQueueSwitch != SWITCH_ON){
        unique_lock<mutex> uq(mtx_file_);
        ofstream ofs(filename, ios::app | ios::out);
        if(!ofs){
            cout << "文件打开失败！" << endl;
            return false;
        }

        ofs << msgs << msg << line << endl;
        return true;
    }
    else
        return InsertQueue(msgs + msg + line, filename);
}

bool Logger::InsertQueue(std::string msgs, std::string filename)
{
    unique_lock<mutex> uq(mtx_queue_);
    qu_.push(msgs);
    
    if(qu_.size() >= stoi(log_switch_.logFileQueueSize)){
        ofstream ofs(filename, ios::app | ios::out);
        if(!ofs){
            cout << "文件打开失败！" << endl;
            return false;
        }

        while(!qu_.empty()){
            ofs << qu_.front();
            qu_.pop();
        }
        ofs.close();
    }
    return true;
}

void Logger::BindFileMap(string v1, FileType v2)
{
    if(log_switch_.logOutputLevelFile.find(v1) != string::npos)
        file_type_map_[v2] = true;
    else
        file_type_map_[v2] = false;
}

void Logger::BindTerminalMap(string v1, TerminalType v2)
{
    if(log_switch_.logOutputLevelTerminal.find(v1) != string::npos)
        terminal_type_map_[v2] = true;
    else
        terminal_type_map_[v2] = false;
}