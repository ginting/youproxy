#ifndef YOUCONFIG_H
#define YOUCONFIG_H

#include <string>
#include <vector>
#include <pthread.h>
using namespace std;

struct YouRule{
    char suffix[100];
    char ip[50];
    char fuck;
    char dns;
    char proxy;
};

class YouConfig
{
private:
    YouConfig();
    vector<YouRule> ruleList;
    pthread_mutex_t lock;
    int fuckingVersion;

    void lockConfig();
    void unlockConfig();
public:
    static YouConfig* instance();

    string getAddressByHost(const string& host, int* fuck, int *proxy);
    void loadFromNetwork();
    void loadDefaults();
    void setFuckingVersion(int ver);
    void parseConfig(string content);

};

#endif // YOUCONFIG_H
