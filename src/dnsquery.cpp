#include <stdio.h>
#ifdef __WIN32__
#include <winsock.h>
#include <wininet.h>
#define SHUT_RDWR SD_BOTH
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#define closesocket close
#endif
#include <assert.h>
#include <string.h>
#include "dnsquery.h"
#include <cstdlib>

string  DnsQuery::parseName(int &pos) {
    string name;
    char temp[256];
    while(buffer[pos]) {
        int length = (unsigned char)buffer[pos ++];
        if(length == 0xC0) {
            int newPos = (unsigned char)buffer[pos ++];
            if(name.empty())
                name = parseName(newPos);
            else
                name = name + "." + parseName(newPos);
            break;
        } else {
            memcpy(temp, &buffer[pos], length);
            pos += length;
            temp[length] = '\0';
            if(name.empty())
                name = temp;
            else
                name = name + "." + temp;
        }
    }
    return name;
}

DnsQuery::DnsQuery() {
    socketHandle = -1;
}

DnsQuery::~DnsQuery() {
  if(this->socketHandle != -1){
    ::shutdown(this->socketHandle, SHUT_RDWR);
#ifdef __WIN32__
    closesocket(this->socketHandle);
    this->socketHandle = -1;
#else
    close(this->socketHandle);
    this->socketHandle = -1;
#endif
  }
}

void    DnsQuery::setNameServer(string ns) {
    this->nameServer = ns;
}

string  DnsQuery::getErrorString() {
    static char errors[][32] = {
        "No error.",    "Incorrect message format.",    "DNS error.",
        "Domain name not found.",   "Unimplemented command.",   "Denied."
    };
    if(errorCode < 6)
        return errors[errorCode];
    else
        return "Unknown error.";
}

bool    DnsQuery::connect() {
    /* Create a TCP socket */
    socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(socketHandle != -1);
    /* Connect to the name server */
    struct sockaddr_in addr = {0,};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(nameServer.c_str());
    addr.sin_port = htons(53);
    return (0 == ::connect(socketHandle, (sockaddr*)&addr, sizeof(addr)));
}

bool    DnsQuery::queryA(string name) {
    printf("queryA: %s\n", name.c_str());
    errorCode = 0;
    /* Initialize A query header */
    static const char queryHeader[] = {0x00,0x01,0x01,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,0x00,0x00
    };
    memcpy(buffer+2, queryHeader, sizeof(queryHeader));
    bufferPos = 2+sizeof(queryHeader);
    /* put domain name */
    for(size_t next, current = 0; current<name.length(); ) {
        /* divide by dot */
        next = name.find('.', current);
        if(next == string::npos)
            next = name.length();
        /* length of part */
        int length = next - current;
        buffer[bufferPos ++] = length;
        memcpy(buffer + bufferPos, name.c_str() + current, length);
        bufferPos += length;
        current = next + 1;
    }
    buffer[bufferPos ++] = '\0';
    /* query type and class */
    *((unsigned int*)&buffer[bufferPos]) = 0x01000100;
    bufferPos += sizeof(unsigned int);
    *((unsigned short*)&buffer[0]) = htons(bufferPos - 2);
    /* send & wait for reply */
    send(this->socketHandle, buffer, bufferPos, 0);

    int ret = recv(this->socketHandle, buffer, 1000, 0);
    if(ret < bufferPos) {
        errorCode = 2;
        return false;
    }
    if((errorCode = buffer[2+3] & 0xf) != 0)
        return false;
    /* Parse A record data */
    aRecords.clear();
    int aCount = ntohs(*((unsigned short*)&buffer[2+6]));
    for(int i=0; i<aCount; i++) {
        string domain = parseName(bufferPos);
        int type = ntohs(*((unsigned short*)&buffer[bufferPos]));
        /* pass class & ttl */
        bufferPos += 8;
        int rdLength = ntohs(*((unsigned short*)&buffer[bufferPos]));
        bufferPos += 2;
        if(type == 0x1) {
            DnsARecord item;
            in_addr addr;
            memcpy(&addr.s_addr, &buffer[bufferPos], rdLength);
            item.address = inet_ntoa(addr);
            bufferPos += rdLength;
            aRecords.push_back(item);
            printf("found A record: %s\n", item.address.c_str());
        } else {
            bufferPos += rdLength;
        }
    }
    return true;
}

bool    DnsQuery::queryTXT(string name) {
    printf("queryTXT: %s\n", name.c_str());
    errorCode = 0;
    /* Initialize a standard query header */
    static const char queryHeader[] = {0x00,0x01,0x01,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,0x00,0x00
    };
    memcpy(buffer+2, queryHeader, sizeof(queryHeader));
    bufferPos = 2+sizeof(queryHeader);
    /* put domain name */
    for(size_t next, current = 0; current<name.length(); ) {
        /* divide by dot */
        next = name.find('.', current);
        if(next == string::npos)
            next = name.length();
        /* length of part */
        int length = next - current;
        buffer[bufferPos ++] = length;
        memcpy(buffer + bufferPos, name.c_str() + current, length);
        bufferPos += length;
        current = next + 1;
    }
    buffer[bufferPos ++] = '\0';
    /* query type and class */
    *((unsigned int*)&buffer[bufferPos]) = 0x01001000;
    bufferPos += sizeof(unsigned int);
    *((unsigned short*)&buffer[0]) = htons(bufferPos - 2);
    /* send & wait for reply */
    send(this->socketHandle, buffer, bufferPos, 0);

    int ret = recv(this->socketHandle, buffer, 1000, 0);
    if(ret < bufferPos) {
        errorCode = 2;
        return false;
    }
    if((errorCode = buffer[2+3] & 0xf) != 0)
        return false;
    /* Parse TXT record data */
    int aCount = ntohs(*((unsigned short*)&buffer[2+6]));
    for(int i=0; i<aCount; i++) {
        string domain = parseName(bufferPos);
        int type = ntohs(*((unsigned short*)&buffer[bufferPos]));
        /* pass class & ttl */
        bufferPos += 8;
        int rdLength = ntohs(*((unsigned short*)&buffer[bufferPos]));
        bufferPos += 2;
        if(type == 0x10) {
            unsigned char strLength = (unsigned char)buffer[bufferPos];
            bufferPos += 1;
            txtRecord = string(&buffer[bufferPos], strLength);
            printf("found TXT record: %s\n", txtRecord.c_str());
        } else {
            bufferPos += rdLength;
        }
    }
    return true;
}


const string DnsQuery::getARecord()
{
    if(aRecords.size() > 0)
        return aRecords[rand() % aRecords.size()].address;
    return "";
}

const string DnsQuery::getTXTRecord()
{
    return txtRecord;
}


