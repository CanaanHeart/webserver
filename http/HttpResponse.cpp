#include "HttpResponse.h"

using namespace std;

const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = 
{
    { ".html",  "text/html; charset=utf-8" },
    { ".htm",  "text/html; charset=utf-8" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain; charset=utf-8" },
    { ".css",   "text/css" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".ico",   "image/x-icon"},
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".mp4",   "video/mp4" },
    { ".mp3",   "audio/mpeg" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const unordered_map<int, string> HttpResponse::CODE_STATUS = 
{
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"}
};

const std::unordered_map<int, std::string> HttpResponse::ERROR_PATH = 
{
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"}
};

HttpResponse::HttpResponse(const string &server_dir):server_dir_(server_dir)
{

}

void HttpResponse::Init(const std::string &path, bool keep_alive, string version, 
                        bool is_login, string username, bool set_cookie)
{
    is_login_ = is_login;
    username_ = username;
    set_cookie_ = set_cookie;

    if(server_dir_.size() == 0){
        ERROR("服务器资源目录不存在!");
    }

    path_ = path;
    keep_alive_ = keep_alive;
    version_ = version;
    if(path_ == "/")
        abs_path_ = server_dir_;
    else if(path_ == "/User"){
        is_user_list_ = true;
    }
    else{
        if(IsUser())
            is_user_ = true;
        else
            abs_path_ = server_dir_ + path_;
    }

    if((stat(abs_path_.c_str(), &st_) == 0)){
        if(S_ISDIR(st_.st_mode))
            is_dir_ = true;
        else if(S_ISREG(st_.st_mode)){
            if(st_.st_size > CHUNK_SIZE)
                is_chunk_ = true;
            else
                is_chunk_ = false;
        }
        else{
            ERROR("file type error!");
        }
    }

    // DEBUG("path: ", path_);
    // DEBUG("is user: ", is_user_);
    // DEBUG("is user list: ", is_user_list_);
    // DEBUG("is dir: ", is_dir_);
    // DEBUG("is chunk: ", is_chunk_);
    // DEBUG("is read over: ", is_readover_);
    // DEBUG("is login: ", is_login_);
}

bool HttpResponse::IsChunk() const
{
    return is_chunk_;
}

bool HttpResponse::IsReadOver() const
{
    return is_readover_;
}

bool HttpResponse::IsUser() const
{
    string username = path_.substr(1);
    SP_MysqlConn sql_conn = MysqlConnPool::GetInstance().GetConn();
    ostringstream oss;
    oss << "select name from User where name='" << username << "'";

    if(sql_conn->Query(oss.str()))
        return true;
    return false;
}

void HttpResponse::Reset()
{
    code_ = -1;
    is_user_ = false;
    is_user_list_ = false;
    is_dir_ = false;
    is_chunk_ = false;
    is_readover_ = false;
    path_ = "";
    abs_path_ = "";
    offset_ = 0;
    memset(&st_, 0, sizeof(st_));
}

void HttpResponse::SetMode()
{
    int ret = stat(abs_path_.c_str(), &st_);
    if(ret == 0 || is_user_ || is_user_list_ || path_ == "/upload"){ 
        code_ = 200;
    }
    else
        code_ = 404;
}

void HttpResponse::AddLine(Buffer &buffer)
{
    string status;
    if(CODE_STATUS.count(code_) == 1)
        status = CODE_STATUS.find(code_)->second;
    else{
        DEBUG("400");
        code_ = 400;
        status = CODE_STATUS.find(code_)->second;
    }

    string line = version_ + " " + to_string(code_) + " " + status + "\r\n";
    buffer.AppendWithSep(line);
}

void HttpResponse::AddHead(Buffer &buffer)
{
    buffer.AppendWithSep("Connection: ");
    if(keep_alive_){
        buffer.AppendWithSep("Keep-Alive\r\n");
        buffer.AppendWithSep("Keep-Alive: max=6, timeout=5\r\n");
    }
    else
        buffer.AppendWithSep("Close\r\n");
    if(set_cookie_){
        string uuid = UpdateCookie();
        if(!uuid.empty())
            buffer.AppendWithSep("Set-Cookie: " + uuid + "\r\n");

    }
    if(code_ == 200){
        if(is_dir_)
            buffer.AppendWithSep("Content-Type: " + GetFIleType(".html") + "\r\n");
        else if(is_user_list_)
            buffer.AppendWithSep("Content-Type: " + GetFIleType(".html") + "\r\n");
        else if(is_user_)
            buffer.AppendWithSep("Content-Type: " + GetFIleType(".html") + "\r\n");
        else if(path_ == "/upload")
            buffer.AppendWithSep("Content-Type: " + GetFIleType(".html") + "\r\n");
        else
            buffer.AppendWithSep("Content-Type: " + GetFIleType() + "\r\n");
    }
    else
        buffer.AppendWithSep("Content-Type: " + GetFIleType(".html") + "\r\n");
}

void HttpResponse::AddBody(Buffer &buffer)
{
    if(code_ == 200){
        // if(stat(m_abs_path.c_str(), &m_st) != 0){
        //     WARN("file open error!");
        // }
        if(is_dir_){
            ReadDir(abs_path_, buffer);
        }
        else if(is_user_list_)
            ReadUserList(buffer);
        else if(is_user_)
            ReadUserInfo(buffer);
        else if(path_ == "/upload"){
            buffer.AppendWithSep("Content-Length: 0\r\n\r\n");
            return;
        }
        else{
            buffer.AppendWithSep("Content-Length: " + to_string(st_.st_size) + "\r\n\r\n");
            ReadFile(abs_path_, buffer);
        }
    }
    else{
        abs_path_ = server_dir_ + ERROR_PATH.find(code_)->second;
        if(stat(abs_path_.c_str(), &st_) != 0){
            WARN("file open error!");
        }
        buffer.AppendWithSep("Content-Length: " + to_string(st_.st_size) + "\r\n\r\n");
        ReadFile(abs_path_, buffer);
    }
}

void HttpResponse::AddHeadChunk(Buffer &buffer)
{
    buffer.AppendWithSep("Connection: ");
    if(keep_alive_){
        buffer.AppendWithSep("Keep-Alive\r\n");
        buffer.AppendWithSep("Keep-Alive: max=6, timeout=5\r\n");
    }
    else
        buffer.AppendWithSep("Close\r\n");
    if(set_cookie_){
        string uuid = UpdateCookie();
        if(!uuid.empty())
            buffer.AppendWithSep("Set-Cookie: " + GenerateUuid() + "\r\n");

    }
    buffer.AppendWithSep("Transfer-Encoding: chunked\r\n\r\n");
}

void HttpResponse::AddBodyChunk(Buffer &buffer)
{
    ReadChunk(abs_path_, buffer);
    if(IsReadOver()){
        int len = 0;
        ostringstream oss;
        oss << std::hex << len << "\r\n\r\n";
        buffer.AppendWithSep(oss.str());
        oss.str("");
        oss.clear();
    }
}

void HttpResponse::ReadFile(string path, Buffer &buffer)
{
    char temp_buf[4096];

    int fd = open(path.c_str(), O_RDONLY);
    if(fd == -1){
        WARN("open file error!");
        return;
    }

    while(1){
        memset(temp_buf, 0, sizeof(temp_buf));

        int n = read(fd, temp_buf, sizeof(temp_buf));
        if(n == -1){
            ERROR("file read error!");
        }
        else if(n == 0)
            break;
        buffer.Append(temp_buf, n);
    }
    close(fd);
}

void HttpResponse::ReadDir(string path, Buffer &buffer)
{
    int len = 0;
    ostringstream oss;
    if(path == server_dir_){
        oss << "<html><body><tr><td><a href=\"login.html\">登录</a></td><br/>";
        oss << "<td><a href=\"register.html\">注册</a></td></tr><br/>";
        oss << "<td><a href=\"User\">用户</a></td></tr>";
        oss <<  "<h1>当前目录: " << path_ << "</h1><hr><table>";
    }
    else
        oss << "<html><body><h1>当前目录: " << path_ << "</h1><hr><table>";

    DIR *dir = opendir(path.c_str());
    if(!dir){
        ERROR("open dir error!");
    }

    struct dirent *ptr = nullptr;

    while((ptr = readdir(dir)) != nullptr){
        struct stat st;
        string name = ptr->d_name;
        string temp_abs_path = abs_path_ + "/" + name;
        string sub_path;
        if(stat(temp_abs_path.c_str(), &st) != 0){
            WARN("stat error!");
        }
        if(abs_path_ == server_dir_)
            sub_path = path_.substr(1) + "/" + name;
        else
            sub_path = path_ + "/" + name;
        oss << "<tr><td><a href=\"" << sub_path << "\">" << name << "</a></td><td>" << static_cast<long>(st.st_size) << "</td></tr>";
    }
    oss << "</table></body></html>";

    len = oss.str().length();

    buffer.AppendWithSep("Content-Length: " + to_string(len) + "\r\n\r\n");

    buffer.AppendWithSep(oss.str());
}

void HttpResponse::ReadChunk(std::string path, Buffer &buffer)
{   
     int fd = open(path.c_str(), O_RDONLY);
     if(fd == -1){
        WARN("open file error!");
        return;
    }

    if(offset_ < st_.st_size){
        int offset = lseek(fd, offset_, SEEK_SET);
        if(offset == -1){
            WARN("lseek error!");
            return;
        }
    }
    Buffer temp_buffer;
    char temp_buf[4096];
    int chunk_size = 0;
    while(chunk_size < CHUNK_SIZE){
        memset(temp_buf, 0, sizeof(temp_buf));
        int n = read(fd, temp_buf, sizeof(temp_buf));
        if(n == -1){
            ERROR("file read error!");
        }
        else if(n == 0){
            is_readover_ = true;
            break;
        }
        chunk_size += n;
        temp_buffer.Append(temp_buf, n);
    }
    close(fd);
    offset_ += chunk_size;
    ostringstream oss;
    oss << std::hex << chunk_size << "\r\n";
    buffer.AppendWithSep(oss.str());
    oss.str("");
    oss.clear();
    buffer.AppendWithSep(temp_buffer);
    buffer.AppendWithSep("\r\n");
}

void HttpResponse::ReadUserList(Buffer &buffer)
{
    SP_MysqlConn sql_conn = MysqlConnPool::GetInstance().GetConn();

    int len = 0;
    ostringstream oss;
    oss << "select name from User";

    if(sql_conn->Query(oss.str())){
        oss.str("");
        oss.clear();
        oss << "<html><body><h1>用户列表</h1><hr><table>";
        while(sql_conn->Next()){
            string username = sql_conn->Value(0);
            if(is_login_ && username_ == username)
                oss << "<tr><td><a href=\"" << username << "\">" << username << "</a>(当前用户)</td></tr>";
            else
                oss << "<tr><td><a href=\"" << username << "\">" << username << "</a></td></tr>";
        }
    }
    oss << "</table></body></html>";

    len = oss.str().length();

    buffer.AppendWithSep("Content-Length: " + to_string(len) + "\r\n\r\n");

    buffer.AppendWithSep(oss.str());
}

void HttpResponse::ReadUserInfo(Buffer &buffer)
{
    SP_MysqlConn sql_conn = MysqlConnPool::GetInstance().GetConn();

    int len = 0;
    ostringstream oss1, oss2;

    string username = path_.substr(1);

    oss1 << "select name from File where id in (";
    oss1 << "select file_id from User_File where user_id=(";
    oss1 << "select id from User where name='" << username << "'))";

    oss2 << "<html><body><h1>" << username << "</h1><br/>";
    if(username_ == username){
        oss2 << "<input type=\"file\" id=\"fileInput\">";
        oss2 << "<button onclick=\"uploadFile()\">Upload</button>";
    }

    oss2 << "<hr><table>";

    if(sql_conn->Query(oss1.str())){
        while(sql_conn->Next()){
            string filename = sql_conn->Value(0);
            string rel_path = username + "/" + filename;
            oss2 << "<tr><td><a href=\"" << rel_path << "\">" << filename << "</a></td></tr>"; 
        }
    }

    oss2 << "</table>";
    if(username_ == username){
        ifstream ifs(server_dir_ + "/codes/upload.js");
        if(!ifs.is_open()){
            WARN("file open fail!");
        }
        string line;
        oss2 << "<script>";
        while(getline(ifs, line))
            oss2 << line;
        oss2 << "</script>";
    }
    oss2 << "</body></html>";

    len = oss2.str().length();

    buffer.AppendWithSep("Content-Length: " + to_string(len) + "\r\n\r\n");

    buffer.AppendWithSep(oss2.str());
}

string HttpResponse::GenerateUuid()
{
    char buf[GUID_LEN] = {0};

    uuid_t uu;
    uuid_generate(uu);
    int32_t index = 0;
    for(int32_t i = 0; i < 16; ++i){
        int32_t len = i < 15?
            sprintf(buf + index, "%02X-", uu[i]):
            sprintf(buf + index, "%02X", uu[i]);
        if(len < 0)
            return string("");
        index += len;
    }
    return string(buf);
}

string HttpResponse::UpdateCookie()
{
    string uuid = GenerateUuid();

    SP_MysqlConn sql_conn = MysqlConnPool::GetInstance().GetConn();
    ostringstream oss;
    oss << "select uuid from Cookie where uuid='" << uuid << "'";

    if(sql_conn->Query(oss.str()))
        return string("");
    
    oss.str("");
    oss.clear();
    oss << "insert into Cookie values('" << uuid << "', '" << username_ << "', '" << is_login_ << "')";
    if(sql_conn->TransAction()){
        if(sql_conn->Update(oss.str())){
            sql_conn->Commit();
            return uuid;
        }
        else{
            sql_conn->RoolBack();
            return string("");
        }
    }

    return string("");
}

string HttpResponse::GetFIleType()
{
    size_t pos = path_.find('.');
    if(pos == string::npos)
        return "text/plain";
    
    string suffix = path_.substr(pos);

    if(SUFFIX_TYPE.count(suffix) == 1)
        return SUFFIX_TYPE.at(suffix);
    
    return "text/plain";
}

string HttpResponse::GetFIleType(string key)
{
    if(SUFFIX_TYPE.count(key) == 1)
        return SUFFIX_TYPE.at(key);
    else
        return "text/plain";
}