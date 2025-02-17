#include "../logger/Logger.h"
#include "../http/HttpServer.h"
#include "../time/TimeStamp.h"
#include "../http/HttpLoadConfig.h"
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

HttpServer *http_server;

void Stop(int sig)
{
    // DEBUG("sig:", sig);
    http_server->Stop();
    delete http_server;
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    INFO("-----B/S-----");

    signal(SIGTERM, Stop);
    signal(SIGINT, Stop);

    string conf_path = "/home/zxc/MProj/webserver/web_server/conf/serv.conf";

    HttpLoadConfig http_lc_;
    http_lc_.LoadConfigFile(conf_path);
    
    uint16_t port = static_cast<uint16_t>(stoi(http_lc_.GetValue("serv_port")));
    string ip = http_lc_.GetValue("serv_ip");
    string server_dir = http_lc_.GetValue("server_dir");

    http_server = new HttpServer(port, ip, server_dir);

    http_server->Start();

    return 0;
}