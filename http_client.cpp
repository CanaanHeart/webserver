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
    // uint16_t port;
    // string ip = "192.168.1.119";

    // if(argc == 3){
    //     port = atoi(argv[1]);
    //     ip = string(argv[2]);
    // }
    // else
    //     port = SERV_PORT;

    // Socket cli_sock;
    // InetAddress serv_addr(port, &ip);
    // cli_sock.Connect(serv_addr);

    // string head = "GET /home HTTP1.1\r\nConnection: keep-alive\r\nHost: 192.168.1.119:9533\r\n\r\n";

    // if(send(cli_sock.GetFd(), head.c_str(), head.length(), 0) <= 0)
    //     ERROR("client send error!");

    return 0;
}