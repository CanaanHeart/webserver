#include "../logger/Logger.h"
#include "InetAddress.h"
#include "Socket.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>

#include <iostream>
#include <string>

using namespace std;

#define SERV_PORT 9533
#define INT_BYTES 4

int main(int argc, char *argv[])
{
    uint16_t port;
    string ip = "192.168.1.119";

    if(argc == 3){
        port = atoi(argv[1]);
        ip = string(argv[2]);
    }
    else
        port = SERV_PORT;

    Socket cli_sock;
    InetAddress serv_addr(port, &ip);
    cli_sock.Connect(serv_addr);

    // 测试粘包和分包
    // for(int i = 0; i < 100000; ++i){
    //     string body = "这是第" + to_string(i + 1) + "个玩家！";

    //     char temp_buf[1024];
    //     memset(temp_buf, 0, sizeof(temp_buf));
    //     int len = body.size();
    //     memcpy(temp_buf, &len, 4);
    //     memcpy(temp_buf + 4, body.c_str(), len);

    //     if(send(cli_sock.GetFd(), temp_buf, len + 4, 0) <= 0){
    //         ERROR("client end error!");
    //     }
    // }

    // char buf[1024];
    // for(int i = 0; i < 100000; ++i){
    //     int len = 0;
    //     if(recv(cli_sock.GetFd(), &len, 4, 0) <= 0){
    //         ERROR("client recv head error!");
    //     }

    //     memset(buf, 0, sizeof(buf));
    //     if(recv(cli_sock.GetFd(), buf, len, 0) <= 0){
    //         ERROR("client recv body error!");
    //     }
    //     // cout << buf << endl;
    // }
    // 测试粘包和分包

    // echo测试
    // string line;
    // char buf[1024];

    // while(getline(cin, line)){
    //     memset(buf, 0, sizeof(buf));

    //     int len = line.size();

    //     memcpy(buf, &len, INT_BYTES);
    //     memcpy(buf + INT_BYTES, line.c_str(), line.size());

    //     if(send(cli_sock.GetFd(), buf, len + INT_BYTES, 0) <= 0){
    //         ERROR("client end error!");
    //     }

    //     if(recv(cli_sock.GetFd(), &len, 4, 0) <= 0){
    //         ERROR("client recv head error!");
    //     }
    //     cout << "len:" << len << endl;
    //     memset(buf, 0, sizeof(buf));
    //     if(recv(cli_sock.GetFd(), buf, len, 0) <= 0){
    //         ERROR("client recv body error!");
    //     }
    //     cout << buf << endl;
    // }

    // // 压力测试
    // char buf[1024];
    // INFO("begin!");
    // for(int i = 0; i < 100000; ++i){
    //     memset(buf, 0, sizeof(buf));
    //     string body = "这是第" + to_string(i + 1) + "个玩家！";
    //     int len = body.size();
    //     memcpy(buf, &len, 4);
    //     memcpy(buf + INT_BYTES, body.c_str(), len);
    //     if(send(cli_sock.GetFd(), buf, len + INT_BYTES, 0) <= 0){
    //         ERROR("client end error!");
    //     }

    //     if(recv(cli_sock.GetFd(), &len, 4, 0) <= 0){
    //         ERROR("client recv head error!");
    //     }
    //     memset(buf, 0, sizeof(buf));
    //     if(recv(cli_sock.GetFd(), buf, len, 0) <= 0){
    //         ERROR("client recv body error!");
    //     }
    //     // cout << buf << endl;
    // }
    // INFO("end!")

    // 网络测试
    string head = "GET /home HTTP1.1\r\nHost: 192.168.1.119:9533\r\n\r\n";

    cout << head.length() << endl;

    if(send(cli_sock.GetFd(), head.c_str(), head.length(), 0) <= 0)
        ERROR("client send error!");

    while(1);

    return 0;
}