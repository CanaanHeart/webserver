#include "HttpRequest.h"

using namespace std;

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0}, 
    {"/login.html", 1}
};

HttpRequest::HttpRequest(const string &server_dir):server_dir_(server_dir)
{
    state_ = State::LINE;
    is_login_ = false;
    set_cookie_ = false;
    header_.clear();
    post_.clear();
}

bool HttpRequest::IsKeepAlive() const
{
    if(header_.count("Connection") == 1)
        return header_.find("Connection")->second == "keep-alive";
    return false;
}

bool HttpRequest::IsLogin() const
{
    return is_login_;
}

bool HttpRequest::IsSetCookie() const
{
    return set_cookie_;
}

const string& HttpRequest::GetMethod() const
{
    return method_;
}

const string& HttpRequest::GetPath() const
{
    return path_;
}

const string& HttpRequest::GetVersion() const
{
    return version_;
}

const std::string& HttpRequest::GetUserName() const
{
    return user_name_;
}

bool HttpRequest::Parse(Buffer &buffer)
{
    const string end_mark = "\r\n";
    
    if(buffer.ReadableBytes() <= 0)
        return false;
    
    while((buffer.ReadableBytes() > 0) && (state_ != State::FINISH)){
        const char *line_end = search(buffer.BeginRead(), buffer.BeginWrite(), end_mark.begin(), end_mark.end());
        string line;
        if(state_ != State::BODY)
            line = string(buffer.Peek(), line_end);
        else
            line = string(buffer.Peek(), buffer.ReadableBytes());
        switch(state_){
            case State::LINE:{
                // DEBUG("LINE");
                if(!ParseLine(line))
                    return false;
                // DEBUG(method_, " ", path_, " ", version_);
                break;
            }
            case State::HEADER:{
                // DEBUG("HEADER");
                ParseHeader(line);
                if(buffer.ReadableBytes() <= 2)
                    state_ = State::FINISH;
                break;
            }
            case State::BODY:{
                // DEBUG("BODY");
                ParseBody(line);
                state_ = State::FINISH;
                break;
            }
            case State::FINISH:{
                break;
            }
        }

        if(line_end == buffer.BeginWrite()){
            break;
        }

        buffer.ReadUntil(line_end + end_mark.length());
    }
    return true;
}

bool HttpRequest::ParseLine(const string &line)
{
    regex re("^([^ ]*) ([^ ]*) ([^ ]*)$");
    smatch sub_match;

    if(regex_match(line, sub_match, re)){
        method_ = sub_match[1];
        path_ = sub_match[2];
        version_ = sub_match[3];

        state_ = State::HEADER;
        return true;
    }
    return false;
}

void HttpRequest::ParseHeader(const string &line)
{
    regex re("^([^:]*): ?(.*)$");
    smatch sub_match;

    if(regex_match(line, sub_match, re)){
        header_[sub_match[1]] = sub_match[2];
        if(sub_match[1] == "Cookie")
            ParseCookie();
    }
    else
        state_ = State::BODY;
}

void HttpRequest::ParseBody(const string &lines)
{

    if(path_ == "/upload"){
        if(IsLogin()){
            ParseFormData(lines);
            MergeFile();
        }
        else
            path_ = "/";
    }
    else
        ParsePost(lines);
}

void HttpRequest::ParseFormData(const string &lines)
{
    DEBUG("data: ", lines);

    if(method_ != "POST")
        return;
    if(header_.count("Content-Type") != 1){
        WARN("Content-Type is not exit!");
        return;
    }
    string flag = "boundary";
    size_t pos = header_.at("Content-Type").find(flag) + flag.size() + 1;
    if(pos == string::npos){
        WARN("no boundary");
        return;
    }
    
    string boundary = header_.at("Content-Type").substr(pos);
    string sep1 = "\r\n\r\n";
    string sep2 = "name=\"";

    DEBUG("boundary: ", boundary);

    pos = 0;
    while(pos != string::npos){
        string key, value;
        pos = lines.find(boundary, pos);
        size_t sep2_pos = lines.find(sep2, pos);
        if(sep2_pos == string::npos){
            WARN("sep name=\" not found");
            return;
        }
        for(size_t i = sep2_pos + sep2.length(); ; ++i){
            if(lines[i] == '\"')
                break;
            key += lines[i];
        }
        if(key == "File"){
            size_t sep1_pos = lines.find(sep1, sep2_pos);
            ParseData(lines, boundary, sep1_pos + sep1.length());
            break;
        }
        size_t sep1_pos = lines.find(sep1, sep2_pos);
        if(pos == string::npos){
            WARN("sep '\\r\\n\\r\\n' not found")
            return;
        }
        pos = lines.find(boundary, sep2_pos);
        if(pos == string::npos)
            return;
        for(size_t i = sep1_pos + sep1.length(); i < pos - 4; ++i)
            value += lines[i];

        post_[key] = value;
    }
}

void HttpRequest::ParseData(const string &lines, const string &boundary, size_t pos)
{    
    string end_boundary = boundary + "--";
    size_t end_pos = lines.find(end_boundary, pos);
    if(end_pos == string::npos){
        WARN("end boundary not found!");
        return;
    }

    SaveData(lines, pos, end_pos);
}

void HttpRequest::ParseCookie()
{
    string uuid = header_["Cookie"];

    SP_MysqlConn sql_conn = MysqlConnPool::GetInstance().GetConn(); 

    ostringstream oss;
    oss << "select name, is_login from Cookie where uuid=\'" << uuid << "\'";
    if(sql_conn->Query(oss.str())){
        if(sql_conn->Next()){
            user_name_ = sql_conn->Value(0);
            is_login_ = static_cast<bool>(stoi(sql_conn->Value(1)));
        }
    }
}

void HttpRequest::SaveData(const string &lines, size_t beg, size_t end)
{
    string filename = post_.at("FileName").substr(0, post_.at("FileName").find('.'));
    string curr_chunk = post_.at("CurrChunk");
    string path = server_dir_ + "/" + GetUserName() + "/" + filename + "_" + curr_chunk;

    ofstream tmp_ofs(path, ios::out);
    if(!tmp_ofs.is_open()){
        WARN("open file fail!");
        return;
    }
    tmp_ofs << lines.substr(beg, end - beg - 4);
}

void HttpRequest::MergeFile()
{
    string path = server_dir_ + "/" + GetUserName() + "/" + post_.at("FileName");
    string chunk_name = post_.at("FileName").substr(0, post_.at("FileName").find('.'));

    int total_chunks = stoi(post_.at("TotalChunks"));
    int chunks = 0;
    for(int i = 0; i < total_chunks; ++i){
        string temp_path = server_dir_ + "/" + GetUserName() + "/" + chunk_name + "_" + to_string(i);
        ifstream ifs(temp_path, ios::binary);
        if(!ifs.is_open()){
            return;
        }
        ifs.close();
        ++chunks;
    }
    if(chunks != total_chunks)
        return;

    ofstream ofs(path, ios::out | ios::binary);
    if(!ofs.is_open()){
        WARN("open file fail!");
        return;
    }
    for(int i = 0; i < total_chunks; ++i){
        string temp_path = server_dir_ + "/" + GetUserName() + "/" + chunk_name + "_" + to_string(i);
        ifstream ifs(temp_path, ios::binary);
        if(!ifs.is_open()){
            WARN("file chunk ", i, " open fail!");
            return;
        }

        ofs << ifs.rdbuf();

        ifs.close();

        if(remove(temp_path.c_str()) != 0){
            WARN("file chunk ", i, " remove fail!");
        }
    }
    AddFile2MySql();
}

void HttpRequest::AddFile2MySql()
{
    string file_path = server_dir_ + "/" + GetUserName();
    string file_name = post_.at("FileName");

    // DEBUG("path: ", file_path);
    // DEBUG("filename: ", file_name);
    // DEBUG("user: ", GetUserName);

    SP_MysqlConn sql_conn = MysqlConnPool::GetInstance().GetConn(); 

    ostringstream oss;
    oss << "select path, name from File where path='" << file_path << "' and name='" << file_name << "'";

    if(sql_conn->Query(oss.str()))
        return;
    
    oss.str("");
    oss.clear();
    oss << "select id from User where name='" << GetUserName() << "'";

    string user_id;
    string file_id;

    if(sql_conn->Query(oss.str()))
        if(sql_conn->Next())
            user_id = sql_conn->Value(0);

    oss.str("");
    oss.clear();
    oss << "set session transaction isolation level read uncommitted";
    if(!sql_conn->Update(oss.str())){
        WARN("isolation level set fail!");
    }
    oss.str("");
    oss.clear();
    
    if(sql_conn->TransAction()){
        oss << "insert into File(path, name) values('" << file_path << "', '" << file_name << "')";
        if(sql_conn->Update(oss.str())){
            oss.str("");
            oss.clear();
            oss << "select id from File where path='" << file_path << "' and name='" << file_name << "'";
            if(sql_conn->Query(oss.str()))
                if(sql_conn->Next())
                    file_id = sql_conn->Value(0);

            oss.str("");
            oss.clear();
            oss << "insert into User_File(user_id, file_id) values(" << user_id << ", " << file_id << ")";

            if(sql_conn->Update(oss.str()))
                sql_conn->Commit();
            else
                sql_conn->RoolBack();
        }
        else
            sql_conn->RoolBack();
    }
}

void HttpRequest::ParsePost(const string &lines)
{
    if(method_ != "POST")
        return;

    ParseURL(lines);
    if(DEFAULT_HTML_TAG.count(path_) == 1){
        int tag = DEFAULT_HTML_TAG.find(path_)->second;
        if(tag == 0){
            if(RegisterVerify(post_.at("name"), post_.at("password"))){
                path_ = "/";
                is_login_ = true;
                user_name_ = post_.at("name");
                set_cookie_ = true;
                CreateUserDir(post_.at("name"));
            }
            else{
                DEBUG("user is exited!");
                path_ = "/register.html";
            }
        }
        else if(tag == 1){
            if(LoginVerify(post_.at("name"), post_.at("password"))){
                path_ = "/";
                is_login_ = true;
                user_name_ = post_.at("name");
                set_cookie_ = true;
            }
            else{
                DEBUG("username or password error!");
                path_ = "/login.html";
            }
        }
    }
}

void HttpRequest::ParseURL(const string &lines)
{
    if(lines.size() == 0)
        return;
    
    string key, value;
    int n = lines.size();
    int i = 0, j = 0;
    for(; i < n; ++i){
        char ch = lines[i];
        switch(ch){
            case '=':{
                key = lines.substr(j, i- j);
                j = i + 1;
                break;
            }
            case '&':{
                value = lines.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                break;
            }
            default:
                break;
        } 
    }

    if(post_.count(key) == 0 && j < i){
        value = lines.substr(j, i - j);
        post_[key] = value;
    }
}

bool HttpRequest::LoginVerify(std::string name, std::string password)
{
    if(name.empty() || password.empty())
        return false;
    SP_MysqlConn sql_conn = MysqlConnPool::GetInstance().GetConn();

    ostringstream oss;
    oss << "select name, password from User where name='" << name << "' and password='" << password << "'";

    if(!sql_conn->Query(oss.str()))
        return false;
    
    sql_conn->Next();
    if(name == sql_conn->Value(0) && password == sql_conn->Value(1))
        return true;
    return false;

}   

bool HttpRequest::RegisterVerify(std::string name, std::string password)
{
    if(name.empty() || password.empty())
        return false;
    SP_MysqlConn sql_conn = MysqlConnPool::GetInstance().GetConn();
    ostringstream oss;
    oss << "select name from User where name='" << name << "'";

    if(sql_conn->Query(oss.str()))
        return false;
    
    oss.str("");
    oss.clear();
    oss << "insert into User(name, password) values('" << name << "', '" << password << "')";

    if(sql_conn->TransAction()){
        if(sql_conn->Update(oss.str())){
            sql_conn->Commit();
            return true;
        }
        else{
            sql_conn->RoolBack();
            return false;
        }
    }

    return false;
}

void HttpRequest::CreateUserDir(string name)
{
    string path = server_dir_ + "/" + name;
    int ret = mkdir(path.c_str(), 0775);
    if(ret == 0)
        return;
    else{
        if(errno == EEXIST)
            return;
        else{
            ERROR("mkdir fail!");
        }
    }
}

int HttpRequest::Hex2Dec(char ch)
{
    if(ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if(ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return ch;
}

void HttpRequest::PrintHeader()
{
    for(auto it = header_.begin(); it != header_.end(); ++it){
        cout << it->first << ": " << it->second << endl;
    }
}

void HttpRequest::PrintPost()
{
    for(auto it = post_.begin(); it != post_.end(); ++it){
        cout << it->first << ": " << it->second << endl;
    }
}