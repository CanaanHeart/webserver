#include "Connection.h"
#include "Channel.h"
#include "EventLoop.h"

using namespace std;

Connection::Connection(EventLoop *loop, UP_Socket cli_sock):
            loop_(loop), cli_sock_(std::move(cli_sock)), is_disconnetced_(false),
            read_buf_(make_unique<Buffer>(Msg_Format::HTTP)), send_buf_(make_unique<Buffer>(Msg_Format::HTTP))         
{
    cli_ch_ = make_unique<Channel>(loop_, cli_sock_->GetFd());
    cli_ch_->SetReadCB(std::bind(&Connection::OnMessage, this));
    cli_ch_->SetCloseCB(std::bind(&Connection::CloseCB, this));
    cli_ch_->SetErrorCB(std::bind(&Connection::ErrorCB, this));
    cli_ch_->SetWriteCB(std::bind(&Connection::WriteCB, this));
    cli_ch_->UseET();
    cli_ch_->EnableRead();
}

int Connection::GetFd() const
{
    return cli_sock_->GetFd();
}

uint16_t Connection::GetPort() const
{
    return cli_sock_->GetPort();
}

std::string Connection::GetIp() const
{
    return cli_sock_->GetIp();
}

void Connection::SetSendBuffer(Buffer &buf)
{
    if(is_disconnetced_){
        return;
    }

    if(loop_->IsLoopThread()){ // I/O线程
        // DEBUG("在I/O线程");
        SendInLoop(buf);
    }
    else{
        // 将发送数据的操作交给I/O线程
        // DEBUG("不在I/O线程");
        loop_->AddTask(std::bind(&Connection::SendInLoop, this, buf));
    }
}

void Connection::SendInLoop(Buffer &buf)
{
    send_buf_->AppendWithSep(buf.Peek(), buf.ReadableBytes());
    cli_ch_->EnableWrite();
}

void Connection::SetCloseCB(function<void(SP_Connection)> fn)
{
    close_cb_ = fn;
}

void Connection::SetErrorCB(function<void(SP_Connection)> fn)
{
    error_cb_ = fn;
}

void Connection::SetOnMessageCB(std::function<void(SP_Connection, Buffer&)> fn)
{
    on_message_cb_ = fn;
}

void Connection::SetSendCompleteCB(std::function<void(SP_Connection)> fn)
{
    send_complete_cb_ = fn;
}

void Connection::OnMessage()
{
    char buf[1024];
            
    while(1){
        memset(buf, 0, sizeof(buf));
        
        int n = recv(GetFd(), buf, sizeof(buf), 0);

        if(n > 0){
            read_buf_->Append(buf, n);
        }
        else if(n == -1 && errno == EINTR){
            // 读取数据被信号中断
            continue;
        }
        else if(n == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){
            // 数据读取完毕
            Buffer msg_buf;

            while(1){
                if(!(read_buf_->PickMsg(msg_buf)))
                    break;

                tim_ = TimeStamp::Now();
                on_message_cb_(shared_from_this(), msg_buf);
                msg_buf.Clear();
            }
            break;
        }
        else if(n == 0){
            // 客户端断开
            cli_ch_->DisableAll();
            CloseCB();
            break;
        }
    }
}

void Connection::CloseCB()
{
    is_disconnetced_ = true;
    loop_->ClearConnectionFromMap(GetFd());
    close_cb_(shared_from_this());
}

void Connection::ErrorCB()
{
    is_disconnetced_ = true;
    error_cb_(shared_from_this());
}

void Connection::WriteCB()
{
    int n = send(GetFd(), send_buf_->Peek(), send_buf_->ReadableBytes(), 0);
    tim_ = TimeStamp::Now();
    if(n == -1){
        // WARN("server send error!");
    }
    else{
        send_buf_->HasRead(n);
    }
    if(send_buf_->ReadableBytes() == 0){
        cli_ch_->DisableWrite();
        send_buf_->Clear();
        send_complete_cb_(shared_from_this());
    }
}

bool Connection::IsTimeout(time_t now, time_t timeout)
{
    return now - tim_.GetTimeInt() > timeout;
}