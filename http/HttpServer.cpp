 #include "HttpServer.h"

using namespace std;

HttpServer::HttpServer(const uint16_t port, const string &ip, const std::string &server_dir):server_dir_(server_dir)
{
    string file = "/home/zxc/MProj/webserver/web_server/conf/threadpool_work.conf";
    server_ = make_unique<TcpServer>(port, ip);
    work_tp_ = make_unique<ThreadPool>(file); 

    server_->SetNewConnetionCB(std::bind(&HttpServer::NewConnection, this, std::placeholders::_1));
    server_->SetDeleteConnectionCB(std::bind(&HttpServer::DeleteConnection, this, std::placeholders::_1));
    server_->SetErrorConnectionCB(std::bind(&HttpServer::ErrorConnection, this, std::placeholders::_1));
    server_->SetOnMessageCB(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    server_->SetSendCompleteCB(std::bind(&HttpServer::SendComplete, this, std::placeholders::_1));

    work_tp_->Start();
}

void HttpServer::Start()
{
    server_->Start();
}

void HttpServer::Stop()
{
    server_->Stop();
}

void HttpServer::NewConnection(SP_Connection conn)
{
    INFO(conn->GetFd(), " ", conn->GetIp(), " ", conn->GetPort());
    {
        // lock_guard<mutex> lock(m_mtx);
    }
}

void HttpServer::DeleteConnection(SP_Connection conn)
{
    INFO(conn->GetFd(), " disconnected!");
    {
        // lock_guard<mutex> lock(mtx_);
    }
}

void HttpServer::ErrorConnection(SP_Connection conn)
{
    INFO(conn->GetFd(), " error!");
    {
        // lock_guard<mutex> lock(mtx_);
    }
}

void HttpServer::OnMessage(SP_Connection conn, Buffer &buf)
{
    if(work_tp_->GetThreadNums() == 0){
        // 没有工作线程
        Task(conn, buf);
    }
    else{   
        work_tp_->AddTask(std::bind(&HttpServer::Task, this, conn, buf));
    }
}

void HttpServer::SendComplete(SP_Connection conn)
{
    // INFO("send over!");
}

void HttpServer::Task(SP_Connection conn, Buffer buf)
{
    SP_HRQ req = make_shared<HttpRequest>(server_dir_);
    SP_HRP rep = make_shared<HttpResponse>(server_dir_);

    if(req->Parse(buf)){

        Buffer sendbuf;
        rep->Init(req->GetPath(), req->IsKeepAlive(), 
                req->GetVersion(), req->IsLogin(), req->GetUserName(), req->IsSetCookie());

        if(rep->IsChunk()){
            // DEBUG("chunk!");
            rep->SetMode();
            rep->AddLine(sendbuf);
            rep->AddHeadChunk(sendbuf);
            conn->SetSendBuffer(sendbuf);
            sendbuf.Clear();
            while(!(rep->IsReadOver())){
                rep->AddBodyChunk(sendbuf);
                conn->SetSendBuffer(sendbuf);
                sendbuf.Clear();
            }
        }
        else{
            // DEBUG("unchunk!");
            rep->SetMode();
            rep->AddLine(sendbuf);
            rep->AddHead(sendbuf);
            rep->AddBody(sendbuf);
            conn->SetSendBuffer(sendbuf);
        }
    }
}