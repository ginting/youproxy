#include <iostream>
#include <cstring>
#include "dnscache.h"
#include "dnsquery.h"

static const char* DEFAULT_DNS_SERVER = "202.181.202.140";
static DnsCache* dns = 0;

DnsCache::DnsCache()
{
    pthread_mutex_init(&this->lock, NULL);
    this->records.clear();
    this->setNameServer(DEFAULT_DNS_SERVER);
}

void DnsCache::setNameServer(string host)
{
    this->nameServer = host;
}

DnsCache* DnsCache::instance()
{
    if(dns)
        return dns;
    dns = new DnsCache();
    return dns;
}

string DnsCache::doQueryA(const string &host)
{
    DnsQuery query;
    query.setNameServer(this->nameServer);
    if(!query.connect()){
        cerr << "Failed to connect to dns server." << endl;
        return host;
    }
    if(!query.queryA(host)){
        cerr << "Failed to query domain: " << query.getErrorString() << endl;
        return host;
    }
    return query.getARecord();
    
}

string DnsCache::queryA(const string &host)
{
    string targetIP = "";
    pthread_mutex_lock(&this->lock);

    if(this->records.find(host) != this->records.end()){
        targetIP = this->records[host];
    }else{
        pthread_mutex_unlock(&this->lock);
        targetIP = this->doQueryA(host);
        pthread_mutex_lock(&this->lock);
    
        this->records[host] = targetIP;
        cout << "dns resolve " << host << " => " << targetIP << endl;
    }

    pthread_mutex_unlock(&this->lock);
    return targetIP;
}

void DnsCache::releaseItem(const string &ip)
{
    pthread_mutex_lock(&this->lock);
    this->records.erase(ip);
    pthread_mutex_unlock(&this->lock);
}
