#ifndef _HTTPRESPONSE_H_
#define _HTTPRESPONSE_H_

#include "../logger/Logger.h"
#include "../inc/Buffer.h"
#include "../sqlpool/MysqlConn.h"
#include "../sqlpool/MysqlConnPool.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <uuid/uuid.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>
#include <algorithm>
#include <sstream>

#define GUID_LEN 64

class HttpResponse;

using SP_HRP = std::shared_ptr<HttpResponse>;

class HttpResponse
{
private:
    int code_ = -1;
    std::string server_dir_;  // 服务器工作目录
    std::string path_;
    bool keep_alive_;
    std::string version_;
    std::string abs_path_; 
    std::string username_;
    struct stat st_;
    bool is_user_ = false;
    bool is_user_list_ = false;
    bool is_login_ = false;    // 是否在登录状态
    bool set_cookie_ = false;
    bool is_dir_ = false;  
    bool is_chunk_ = false;    // 是否分块传输
    bool is_readover_ = false; // 分块传输，判断文件是否读完
    off_t offset_ = 0;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀类型
    static const std::unordered_map<int, std::string> CODE_STATUS;  // 编码状态
    static const std::unordered_map<int, std::string> ERROR_PATH;   // 请求错误时传递的文件
public:
    HttpResponse(const std::string &server_dir);
    ~HttpResponse() = default;

    void Init(const std::string &path, bool keep_alive, 
                std::string version, bool is_login, const std::string username, bool set_cookie);
    bool IsChunk() const;
    bool IsReadOver() const;
    bool IsUser() const;
    void Reset();
    void SetMode();
    void AddLine(Buffer &buffer);
    void AddHead(Buffer &buffer);
    void AddBody(Buffer &buffer);
    void AddHeadChunk(Buffer &buffer);
    void AddBodyChunk(Buffer &buffer);
    
    void ReadFile(std::string path, Buffer &buffer);
    void ReadDir(std::string path, Buffer &buffer);
    void ReadChunk(std::string path, Buffer &buffer);
    void ReadUserList(Buffer &buffer);
    void ReadUserInfo(Buffer &buffer);

    std::string GenerateUuid();
    std::string UpdateCookie();

    std::string GetFIleType();
    std::string GetFIleType(std::string key);
};

#endif // _HTTPRESPONSE_H_