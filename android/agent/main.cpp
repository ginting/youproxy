#include <iostream>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <err.h>
#include <unistd.h>

using namespace std;

#include "httpserver.h"
#include "youconfig.h"
#include "cppthread.h"

static const char* DNS_SERVER = "203.80.96.10";

void enable_proxy();
void disable_proxy();

class DnsServer: public CppThread{
public:
    void process(){
        int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(fd < 0)
           err(1, "failed to create socket");
        int reuse = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(53);
        addr.sin_addr.s_addr = INADDR_ANY;

        if(bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
            err(2, "Failed to bind dns port");
        printf("Local dns server started at 0.0.0.0:53\n");

        char buffer[4000 + 4];
        for(;;){
            memset(&addr, 0, sizeof(addr));
            socklen_t remote_len = sizeof(addr);
            int ret = recvfrom(fd, buffer + 2, 4000, 0, (struct sockaddr*)&addr, &remote_len);
            if(ret <= 0){
                printf("dns server socket closed.\n");
                break;
            }
            printf("recv dns request\n");
    
            /* Add size to front */
            unsigned short packet_size = ret, nsize = htons(ret);
            memcpy(buffer, &nsize, 2);

            int sb = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            // i think sb > 0.
            sockaddr_in proxy_addr;
            memset(&proxy_addr, 0, sizeof(proxy_addr));
            proxy_addr.sin_family = AF_INET;
            proxy_addr.sin_port = htons(53);
            proxy_addr.sin_addr.s_addr = inet_addr(DNS_SERVER);
            if(0 != connect(sb, (struct sockaddr*)&proxy_addr, sizeof proxy_addr)){
                printf("failed to connect to dns server.\n");
                close(sb);
                continue;
            }
            
            if( (ret=send(sb, buffer, packet_size + 2, 0)) <= 0){
                close(sb);
                continue;
            }

            ret = recv(sb, &packet_size, 2, 0);
            if(ret != 2){
                close(sb);
                continue;
            }

            packet_size = ntohs(packet_size);
            printf("recv dns packet size: %d\n", packet_size);
            int nread = 0;
            while(nread < packet_size){
                ret = recv(sb, buffer + nread, packet_size - nread, 0);
                if(ret <= 0){
                    close(sb);
                    break;
                }
                nread += ret;
            }

            if(sendto(fd, buffer, packet_size, 0, (struct sockaddr*)&addr, sizeof addr) != packet_size){
                printf("failed to reply dns to local client.\n");
            }

            close(sb);
            continue;
        }

    }
};

int main()
{
    chdir("/data/data/icefox.youproxy/files");

    system("pkill redsocks");
    signal(SIGPIPE, SIG_IGN);

    YouConfig::instance()->loadFromNetwork();
    HttpServer* server = new HttpServer();
    server->startThread();

    DnsServer* dns = new DnsServer();
    dns->startThread();

    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(fd < 0)
        err(1, "Failed to create socket");
    //int reuse = 1;
    //setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0x1998);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        err(2, "Failed to bind service port");

    atexit(disable_proxy);
    enable_proxy();

   for(;;){
        printf("waiting for command...\n");
        char buf[100];
        memset(&addr, 0, sizeof(addr));
        socklen_t remote_len = sizeof(addr);
        int ret = recvfrom(fd, buf, 100, 0, (struct sockaddr*)&addr, &remote_len);
        if(ret <= 0){
            printf("service socket closed.\n");
            break;
        }
        if(buf[0] == 'q' && buf[1] == 'u' && buf[2] == 'i' && buf[3] == 't'){
            printf("quit\n");
            break;
        }else if(buf[0] == 'n' && buf[1] == 'o' && buf[2] == 'o' && buf[3] == 'p'){
            printf("noop\n");
            sendto(fd, "noop", 4, 0, (struct sockaddr*)&addr, remote_len);
            continue;
        }
        buf[99] = 0;
        printf("unknown command %s\n", buf);
    }
    return 0;
}

