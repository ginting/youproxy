#include "youconfig.h"
#include "dnscache.h"
#include "httpsocket.h"
#include "cppthread.h"
#include "dnsquery.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>

using namespace std;

static YouConfig* config = 0;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

class DownloadConfig: public CppThread{
private:
    string yhostsServer;
    string yhostsServerIP;
    string yhostsPath;
public:
    void process(){
        yhostsServer = "icefox.googlecode.com";
        yhostsPath = "/svn/trunk/yhosts";

        cout << "Checking dns records...\n";
        DnsQuery q;
        q.setNameServer(string("8.8.") + string("4.4"));
        if(!q.connect()){
            cerr << "Failed to connect to dns server.\n";
            return;
        }

        q.queryTXT(string("conf.you") + string("proxy.us"));
        string conf = q.getTXTRecord();
        vector<string> confList;
        split(conf, ' ', confList);
        for(size_t i=0; i<confList.size(); i++){
            if(confList[i] == "dns"){
                ++i;
                DnsCache::instance()->setNameServer(confList[i]);
            }else if(confList[i] == "yhosts"){
                ++i;
                yhostsServer = confList[i];
            }else if(confList[i] == "yhosts_ip"){
                ++i;
                yhostsServerIP = confList[i];
            }else if(confList[i] == "yhosts_path"){
                ++i;
                yhostsPath = confList[i];
            }else if(confList[i] == "fuck_ver"){
                ++i;
                YouConfig::instance()->setFuckingVersion(atoi(confList[i].c_str()));
            }
        }
        if(yhostsServerIP.empty())
            yhostsServerIP = yhostsServer;

        cout << "Downloading config file from web...\n";
        HttpSocket * http = new HttpSocket();
        if(!http->connect(yhostsServerIP, 80)){
            cerr << "Failed to connect to " << yhostsServer << endl;
            return;
        }

        http->send(string("GET ") + yhostsPath + string(" HTTP/1.0\r\nHost: ") + yhostsServer 
                    + string("\r\nConnection: close\r\n")
                   );
        string line;
        while(http->receive(line)){
            if(line.empty())
                break;
        }
        char* ptr = new char[1024 * 1024 + 1];
        int pos = 0;
        while(true){
            int ret = http->receive(ptr + pos, 1024*1024 - pos);
            if(ret <= 0)
                break;
            pos += ret;
        }
        ptr[pos] = '\0';
        cout << "Read http config content:\n" << ptr << endl;
        YouConfig::instance()->parseConfig(ptr);
        delete[] ptr;
        delete http;
    }
};

YouConfig::YouConfig()
{
    pthread_mutex_init(&this->lock, NULL);
    setFuckingVersion(1);
    loadDefaults();
}

void YouConfig::loadFromNetwork()
{
    DownloadConfig * config = new DownloadConfig();
    config->startThread();
}

void YouConfig::setFuckingVersion(int ver)
{
    fuckingVersion = ver;
    cout << "Set fucking verision " << ver << endl;
}

void YouConfig::loadDefaults()
{
    const static char* defaults =
        "www.youtube.com             dns,fuck\n"
        "ytimg.com                   dns,fuck\n"
        ".youtube                    dns,fuck\n"
        ".google                     dns,fuck\n"
        "google.com                  ip,203.208.45.206\n"
        "www.youtube.com             ip,208.117.255.74,fuck\n"//203.208.47.20\n"
        "ytimg.com                   ip,208.117.255.74,fuck\n"//203.208.47.20\n"
        ".youtube.com                ip,208.117.255.74,fuck\n"//dns,fuck,proxy\n"
        "plus.google.com             ip,203.208.45.206\n"
        "doubleclick                 dns,fuck\n"
        "googleusercontent.com       ip,203.208.45.206\n"
        "gstatic.com                 ip,203.208.45.206\n"
        "ggpht.com                   ip,203.208.45.206\n"
        "appspot.com                 dns,fuck\n"
        "blogspot.com                ip,61.30.127.2,fuck,proxy\n"
        "blogspot.tw                 ip,61.30.127.2,fuck,proxy\n"
        "blogger.com                 dns,fuck\n"
        "googleapis.com              dns,fuck\n"
        "googlecode.com              ip,203.208.45.206\n"
        "google.com.hk               ip,203.208.45.206\n"
        "www.facebook.com            ip,66.220.158.16,fuck,proxy\n"
        ".facebook.com               dns,fuck\n"
        "fbcdn.net                   dns,fuck\n"
        "dropbox.com                 dns,fuck\n"
        "wikipedia.com               dns,fuck\n"
        "vimeo.com                   dns,fuck\n"
        "vimeocdn.com                dns,fuck\n"
        "twitter.com                 ip,61.30.127.2,fuck,proxy\n"
        "twimg.com                   ip,61.30.127.2,fuck,proxy\n"
        "t.co                        ip,61.30.127.2,fuck,proxy\n";
    parseConfig(defaults);
}

void YouConfig::parseConfig(string content)
{
    cout << "Loading config ...\n";

    this->lockConfig();
    ruleList.clear();

    vector<string> lines;
    split(content, '\n', lines);

    for(size_t i=0; i<lines.size(); i++){
        string line = lines[i];
        if(line.length()<1 || line[0]=='#')
            continue;
        YouRule r;
        memset(&r, 0, sizeof(r));

        char* tmp = new char[line.length() + 1];
        memcpy(tmp, line.data(), line.length());
        tmp[line.length()] = '\0';
        char* p = strtok(tmp, " ");

        strcpy(r.suffix, p);
        while((p = strtok(NULL, " ,\r"))){
            if(strcmp(p, "dns") == 0){
                r.dns = 1;
            }else if(strcmp("fuck", p) == 0){
                r.fuck = fuckingVersion;
            }else if(strcmp("ip", p) == 0){
                p = strtok(NULL, " ,");
                strcpy(r.ip, p);
            }else if(strcmp("proxy", p) == 0){
                r.proxy = 1;
            }
        }
        ruleList.push_back(r);
    }
    this->unlockConfig();
    cout << "Loaded" << ruleList.size() << "rules.\n";
}

YouConfig* YouConfig::instance()
{
    if(config != 0){
        return config;
    }
    config = new YouConfig();
    return config;
}

string YouConfig::getAddressByHost(const string &host, int * fuck, int * proxy)
{
    this->lockConfig();

    string targetIP;
    int dns = 0;
    vector<YouRule>::iterator i;
    for (i = this->ruleList.begin(); i != this->ruleList.end(); ++i){
        YouRule& r = *i;
        if(host.find(r.suffix) != string::npos){
            *fuck = r.fuck;
            *proxy = r.proxy;
            if(r.ip[0]){
                targetIP = r.ip;
            }
            dns = r.dns;
            break;
        }
    }

    this->unlockConfig();

    if(targetIP.empty()){
        if(dns){
            targetIP = DnsCache::instance()->queryA(host);
        }else{
            return host;
        }
    }
    return targetIP;
}

void YouConfig::lockConfig()
{
    pthread_mutex_lock(&this->lock);
}

void  YouConfig::unlockConfig()
{
    pthread_mutex_unlock(&this->lock);
}
