#include "MysqlConnPool.h"

using namespace std;

MysqlConnPool::MysqlConnPool(): max_conns_(0), min_conns_(0), curr_conns_(0)
{
    Init();

    for(int i = 0; i < min_conns_; ++i)
        CreateConn();

    thread producer(&MysqlConnPool::ProduceMysqlConn, this);
    thread destructor(&MysqlConnPool::DestructMysqlConn, this);

    producer.detach();
    destructor.detach();
}

MysqlConnPool::~MysqlConnPool()
{
    flag_ = false;
    
    cv_.notify_all();

    lock_guard<mutex> lock(mtx_);

    while(!conns_.empty()){
        MysqlConn *conn = conns_.front();
        conns_.pop();
        delete conn;
    }
}

MysqlConnPool& MysqlConnPool::GetInstance()
{
    static MysqlConnPool mysql_cp;
    return mysql_cp;
}

void MysqlConnPool::Init()
{
    map<string, string> conf = InitConfig();
    
    ip_ = conf.at("ip");
    user_ = conf.at("user");
    passwd_ = conf.at("passWd");
    db_name_ = conf.at("dbName");
    port_ = stoi(conf.at("port"));
    max_conns_ = stoi(conf.at("maxConns"));
    min_conns_ = stoi(conf.at("minConns"));
    timeout_ = stoi(conf.at("timeout"));
    idle_time_ = stoi(conf.at("idleTime"));
}

map<string, string> MysqlConnPool::InitConfig()
{
    map<string, string> sqlpoolConf;

    string line;
    ifstream ifs;

    ifs.open("../conf/sqlpool.conf");
    if(!ifs.is_open()){
        ERROR("file sqlpool.conf open fail!");
    }

    while(getline(ifs, line)){
        if(line.empty() || line[0] == '#')
            continue;
        
        string line_copy;
        for (size_t i = 0; i < line.size(); i++)
        {
            if(line[i] == ' ')
                continue;
            line_copy += line[i];
        }

        size_t pos = line_copy.find('=');
        if(pos == std::string::npos){
            WARN("config file error!");
            continue;
        }

        string key_str = line_copy.substr(0, pos);
        string value_str = line_copy.substr(pos + 1);
        
        sqlpoolConf[key_str] = value_str;
    }

    return sqlpoolConf;
}

void MysqlConnPool::CreateConn()
{
    MysqlConn *conn = new MysqlConn();
    if(conn->Connect(user_, passwd_, db_name_, ip_, port_)){
        conn->RefreshAliveTime();
        conns_.push(conn);
    }
}

void MysqlConnPool::ProduceMysqlConn()
{
    while(1){
        unique_lock<mutex> lock(mtx_);
        while(conns_.size() >= static_cast<size_t>(min_conns_) || (curr_conns_ + min_conns_) > max_conns_){
            cv_.wait(lock);
        }
        CreateConn();
    }
}

void MysqlConnPool::DestructMysqlConn()
{
    while(1){
        this_thread::sleep_for(chrono::seconds(1));
        lock_guard<mutex> lock(mtx_);
        while(conns_.size() > static_cast<size_t>(min_conns_)){
            MysqlConn *conn = conns_.front();
            if(conn->GetAliveTime() >= idle_time_){
                conns_.pop();
            }
            else
                break;
        }
    }
}

SP_MysqlConn MysqlConnPool::GetConn()
{
    unique_lock<mutex> lock(mtx_);
    if(conns_.empty()){
        return nullptr;
    }
    SP_MysqlConn conn(conns_.front(), [this](MysqlConn* p){
        lock_guard<mutex> lock(this->mtx_);
        p->RefreshAliveTime();
        this->conns_.push(p);
        --this->curr_conns_;
    });
    conns_.pop();
    ++this->curr_conns_;
    cv_.notify_one();
    return conn;
}

void MysqlConnPool::Print()
{
    cout << ip_ << endl;
    cout << user_ << endl;
    cout << passwd_ << endl;
    cout << db_name_ << endl;
    cout << port_ << endl;
    cout << max_conns_ << endl;
    cout << min_conns_ << endl;
    cout << timeout_ << endl;
    cout << idle_time_ << endl;
}