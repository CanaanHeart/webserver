#ifndef _HTTPREQUEST_H_
#define _HTTPREQUEST_H_

#include "../logger/Logger.h"
#include "../inc/Buffer.h"
#include "../sqlpool/MysqlConn.h"
#include "../sqlpool/MysqlConnPool.h"

#include <cstdio>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <algorithm>
#include <fstream>

// const std::string server_dir = "/home/zxc/MProj/webserver/server_dir";

class HttpRequest;

using SP_HRQ = std::shared_ptr<HttpRequest>;

enum class State{
    LINE,
    HEADER,
    BODY,
    FINISH
};

class HttpRequest
{
private:
    std::string server_dir_;
    State state_;
    std::string method_;
    std::string path_;
    std::string version_;
    bool is_login_;
    bool set_cookie_;
    std::string user_name_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;
    std::map<int, bool> chunk_status_;

    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
public:
    HttpRequest(const std::string &server_dir);
    ~HttpRequest() = default;

    bool IsKeepAlive() const;
    bool IsLogin() const;
    bool IsSetCookie() const;
    const std::string& GetMethod() const;
    const std::string& GetPath() const;
    const std::string& GetVersion() const;
    const std::string& GetUserName() const;
    bool Parse(Buffer &buffer);
    
    void PrintHeader();
    void PrintPost();
private:
    bool ParseLine(const std::string &line);
    void ParseHeader(const std::string &line);
    void ParseBody(const std::string &lines);
    void ParseFormData(const std::string &lines);
    void ParseData(const std::string &lines, const std::string &boundary, size_t pos);
    void ParseCookie();
    void SaveData(const std::string &lines, size_t beg, size_t end);
    void MergeFile();
    void AddFile2MySql();
    void ParsePost(const std::string &lines);
    void ParseURL(const std::string &lines);
    bool LoginVerify(std::string name, std::string password);
    bool RegisterVerify(std::string name, std::string password);
    void CreateUserDir(std::string name);
    int Hex2Dec(char ch);
};

#endif // _HTTPREQUEST_H_