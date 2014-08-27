#ifndef _DNSQUERY_H
#define _DNSQUERY_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

struct DnsARecord {
    string  address;
};

class DnsQuery
{
private:
    int     socketHandle;
    string  nameServer;
    char    buffer[1000+4];
    int     bufferPos;
    int     errorCode;
    vector<DnsARecord> aRecords;
    string  txtRecord;

    string  parseName(int &pos);
public:
    DnsQuery();
    ~DnsQuery();
    void    setNameServer(string ns);
    string  getErrorString();
    bool    connect();
    bool    queryA(string name);
    bool    queryTXT(string name);
    const string   getARecord();
    const string   getTXTRecord();
};

#endif //_DNSQUERY_H

