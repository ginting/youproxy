#include <iostream>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <err.h>

using namespace std;

#include "httpserver.h"
#include "youconfig.h"
#include "systemutil.h"

void enable()
{
    SystemUtil::setSystemProxy("127.0.0.1", 1998);
    SystemUtil::enableSystemProxy();
}

void disable()
{
    printf("disable()\n");
    SystemUtil::disableSystemProxy();
}

int main()
{
    signal(SIGPIPE, SIG_IGN);

    YouConfig::instance()->loadFromNetwork();
    HttpServer* server = new HttpServer();
    server->startThread();

    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd < 0)
        err(1, "Failed to create socket");

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0x1998);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        err(2, "Failed to bind service port");

    //enable();
    //atexit(disable);
    printf("HTTP proxy server is started at localhost:1998.\n");

    for(;;){
        printf("waiting for command...\n");
        char buf[100];
        int ret = recv(fd, buf, 100, 0);
        if(ret <= 0){
            printf("service socket closed.\n");
            break;
        }
        if(buf[0] == 'q' && buf[1] == 'u' && buf[2] == 'i' && buf[3] == 't'){
            printf("quit\n");
            break;
        }
        buf[99] = 0;
        printf("unknown command %s\n", buf);
    }
    return 0;
}

