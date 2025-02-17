#include "TcpServer.h"

using namespace std;

TcpServer::TcpServer(const uint16_t port, const string &ip):
            main_loop_(make_unique<EventLoop>(true)), 
            accp_(main_loop_.get(), port, ip)
{
    accp_.SetNewConnection(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));

    std::string file = "/home/zxc/MProj/webserver/web_server/conf/threadpool_io.conf";

    tp_ = make_unique<ThreadPool>(file);
    tp_->Start();

    for(size_t i = 0; i < tp_->GetThreadNums(); ++i){
        sub_loops_.emplace_back(make_unique<EventLoop>(false, 30, 150));
        sub_loops_[i]->SetConnTimeoutCB(std::bind(&TcpServer::DeleteConnectionEp, this, std::placeholders::_1));
        tp_->AddTask(std::bind(&EventLoop::Loop, sub_loops_[i].get()));
    }
}

void TcpServer::Start()
{
    main_loop_->Loop();
}

void TcpServer::Stop()
{
    main_loop_->Stop();

    for(size_t i = 0; i < sub_loops_.size(); ++i){
        sub_loops_[i]->Stop();
    }
}

void TcpServer::NewConnection(UP_Socket cli_sock)
{
    int index = cli_sock->GetFd() % tp_->GetThreadNums();

    SP_Connection cli_conn = make_shared<Connection>(sub_loops_[index].get(), std::move(cli_sock));

    cli_conn->SetCloseCB(std::bind(&TcpServer::DeleteConnection, this, std::placeholders::_1));
    cli_conn->SetErrorCB(std::bind(&TcpServer::ErrorConnection, this, std::placeholders::_1));
    cli_conn->SetOnMessageCB(std::bind(&TcpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    cli_conn->SetSendCompleteCB(std::bind(&TcpServer::SendComplete, this, std::placeholders::_1));
    // INFO(cli_conn->GetFd(), " ", cli_conn->GetIp(), " ", cli_conn->GetPort());
    {
        // lock_guard<mutex> lock(m_mtx);
        conns_[cli_conn->GetFd()] = cli_conn;
    }
    sub_loops_[index]->AddConnectionMap(cli_conn);

    if(new_connection_cb_)
        new_connection_cb_(cli_conn);  // 回调EchoServer::NewConnection
}

void TcpServer::DeleteConnection(SP_Connection conn)
{   
    if(delete_connection_cb_)
        delete_connection_cb_(conn);
    // INFO(conn->GetFd(), " disconnected!");
    {
        lock_guard<mutex> lock(mtx_);
        conns_.erase(conn->GetFd());
    }
}

void TcpServer::DeleteConnectionEp(SP_Connection conn)
{
    if(delete_connection_cb_)
        delete_connection_cb_(conn);
    {
        lock_guard<mutex> lock(mtx_);
        conns_.erase(conn->GetFd());
    }
}

void TcpServer::ErrorConnection(SP_Connection conn)
{   
    if(error_connection_cb_)
        error_connection_cb_(conn);
    // INFO(conn->GetFd(), " error!");
    {
        lock_guard<mutex> lock(mtx_);
        conns_.erase(conn->GetFd());
    }
}

void TcpServer::OnMessage(SP_Connection conn, Buffer &buf)
{
    if(on_message_cb_)
        on_message_cb_(conn, buf);
}

void TcpServer::SendComplete(SP_Connection conn)
{
    // INFO("server send complete!");
    if(send_complete_cb_)
        send_complete_cb_(conn);
}

void TcpServer::SetNewConnetionCB(std::function<void(SP_Connection)> fn)
{
    new_connection_cb_ = fn;
}

void TcpServer::SetDeleteConnectionCB(std::function<void(SP_Connection)> fn)
{   
    delete_connection_cb_ = fn;
}

void TcpServer::SetErrorConnectionCB(std::function<void(SP_Connection)> fn)
{
    error_connection_cb_ = fn;
}

void TcpServer::SetOnMessageCB(std::function<void(SP_Connection, Buffer&)> fn)
{
    on_message_cb_ = fn;
}

void TcpServer::SetSendCompleteCB(std::function<void(SP_Connection)> fn)
{
    send_complete_cb_ = fn;
}