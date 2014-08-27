#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>

static char old_dns1[104];

void get_dns1(char* buf)
{
    memset(buf, 0, 100);

    FILE* fp = popen("getprop net.dns1", "r");
    if(fp == NULL)
        return;
    fgets(buf, 100, fp);
    pclose(fp);
}

void set_dns1(const char* dns)
{
    char buf[100];
    sprintf(buf, "setprop net.dns1 %s", dns);
    system(buf);
}

void disable_proxy();
void enable_proxy()
{
    char buf[1000];
    sprintf(buf, "./iptables -t nat -A OUTPUT -m owner --uid-owner root -j RETURN");

    printf("enable_proxy\n");
    get_dns1(old_dns1);
    set_dns1("127.0.0.1");

    system("./redsocks");
    system("./iptables -t nat -F");
    if(system(buf) != 0){
        disable_proxy();
        errx(12, "not supported iptables modules.");
        return;
    }
    system("./iptables -t nat -A OUTPUT -p tcp -d 127.0.0.1 -j RETURN");
    system("./iptables -t nat -A OUTPUT -p tcp --dport 80 -j REDIRECT --to 1996");
    system("./iptables -t nat -A OUTPUT -p tcp --dport 443 -j REDIRECT --to 1997");
}

void disable_proxy()
{
    printf("disable_proxy()\n");
    set_dns1(old_dns1);

    system("./iptables -t nat -F");
    
    // Fix me!!!
    // Some phone does not support pkill command ??
    system("pkill redsocks");
}

