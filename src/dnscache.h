#ifndef DNSCACHE_H
#define DNSCACHE_H

#include <string>
#include <map>
#include <pthread.h>
#include "httpsocket.h"
using namespace std;

class DnsCache
{
private:
    DnsCache();
    map<string, string> records;
    pthread_mutex_t lock;

    string doQueryA(const string& host);
    string  nameServer;

public:
    static DnsCache* instance();

    string queryA(const string& host);
    void releaseItem(const string& ip);
    void setNameServer(string host);

};

#endif // DNSCACHE_H
