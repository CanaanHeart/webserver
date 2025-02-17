#include "../logger/Logger.h"
#include "../echo/Echo.h"
#include "../time/TimeStamp.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h"
#include "Channel.h"
#include "EventLoop.h"
#include "TcpServer.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>    // TCP_NODELAY解决网络拥塞
#include <signal.h>

#include <iostream>
#include <string>
#include <memory>
#include <vector>

using namespace std;

#define SERV_PORT 9533
#define MAX_EVENT 1024

EchoServer *echo;

void Stop(int sig)
{
    DEBUG("sig:", sig);
    echo->Stop();
    delete echo;
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    INFO("-----C/S-----");
    
    uint16_t port;
    string *ip = nullptr;

    if(argc == 3){
        port = atoi(argv[1]);
        *ip = string(argv[2]);
    }
    else
        port = SERV_PORT;

    signal(SIGTERM, Stop);
    signal(SIGINT, Stop);

    echo = new EchoServer(port, ip, 2, 2);

    echo->Start();

    return 0;
}