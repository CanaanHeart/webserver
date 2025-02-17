#include "Echo.h"

using namespace std;

EchoServer::EchoServer(const uint16_t port, const std::string *ip,const int tp_nums, const int work_ths):
            server(port, ip, tp_nums), m_tp("WORK", work_ths)
{
    server.SetNewConnetionCB(std::bind(&EchoServer::NewConnection, this, std::placeholders::_1));
    server.SetDeleteConnectionCB(std::bind(&EchoServer::DeleteConnection, this, std::placeholders::_1));
    server.SetErrorConnectionCB(std::bind(&EchoServer::ErrorConnection, this, std::placeholders::_1));
    server.SetOnMessageCB(std::bind(&EchoServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    server.SetSendCompleteCB(std::bind(&EchoServer::SendComplete, this, std::placeholders::_1));
}   

void EchoServer::Start()
{
    server.Start();
}

void EchoServer::Stop()
{
    // 停止工作线程
    m_tp.Stop();
    // 停止IO线程
    server.Stop();
}

void EchoServer::NewConnection(SP_Connection conn)
{
    INFO(conn->GetFd(), " ", conn->GetIp(), " ", conn->GetPort());
}

void EchoServer::DeleteConnection(SP_Connection conn)
{
    INFO(conn->GetFd(), " disconnected!");
}

void EchoServer::ErrorConnection(SP_Connection conn)
{
    INFO(conn->GetFd(), " error!");
}

void EchoServer::OnMessage(SP_Connection conn, Buffer &buf)
{   
    // INFO(msg);
    if(m_tp.Size() == 0){
        // 没有工作线程
        Task(conn, buf);
    }
    else{   
        m_tp.AddTask(std::bind(&EchoServer::Task, this, conn, buf));
    }
}

void EchoServer::SendComplete(SP_Connection conn)
{
    // INFO("server send complete!");
}

void EchoServer::Task(SP_Connection conn, Buffer &buf)
{
    for(auto it = buf.BeginRead(); it != buf.BeginWrite(); ++it)
        if(isalpha(*it))
            *it = toupper(*it);
    
    // string msg = buf.Str();

    // DEBUG(msg);

    conn->SetSendBuffer(buf);
    // sleep(0.1);
}
