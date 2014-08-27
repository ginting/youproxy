#include "httpsocket.h"
#include <cstring>
#include <iostream>
#include <unistd.h>


using namespace std;

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifdef __WIN32
typedef int socklen_t;
#endif

HttpSocket::HttpSocket()
{
	this->socketHandle = -1;
}

HttpSocket::~HttpSocket()
{
  this->shutdown();
}

HttpSocket* HttpSocket::accept()
{
	sockaddr_in addr;
	socklen_t length = sizeof(sockaddr_in);
#ifdef __WIN32
	int ret = ::accept(this->socketHandle, (sockaddr*)&addr, &length);
#else
	int ret = ::accept(this->socketHandle, (sockaddr*)&addr, 
		&length);
#endif
	if(ret < 0)
		return NULL;
	HttpSocket* httpSocket = new HttpSocket;
	httpSocket->socketHandle = ret;
	httpSocket->remoteAddress = addr;
	return httpSocket;
}

bool HttpSocket::bind(int __port)
{
	if(this->socketHandle == -1){
		this->socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(this->socketHandle == -1)
			return false;
		/* Socket reuse */
		int ret = 1;
#ifdef __WIN32__
		setsockopt(this->socketHandle, SOL_SOCKET, SO_REUSEADDR, (char*)&ret, 
			sizeof(ret));
#else
		setsockopt(this->socketHandle, SOL_SOCKET, SO_REUSEADDR, (void*)&ret, 
			sizeof(ret));
#endif
	}
	
	memset(&this->localAddress, 0, sizeof(this->localAddress));
	this->localAddress.sin_family = AF_INET;
	this->localAddress.sin_port = htons(__port);
	this->localAddress.sin_addr.s_addr = INADDR_ANY;
	return 0 == ::bind(this->socketHandle, (sockaddr*)&this->localAddress, 
		sizeof this->localAddress);
}

bool HttpSocket::connect(string __host, int __port)
{
	if(this->socketHandle == -1){
		this->socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(this->socketHandle == -1)
			return false;
	}
	
	memset(&this->remoteAddress, 0, sizeof(this->remoteAddress));
	this->remoteAddress.sin_family = AF_INET;
	this->remoteAddress.sin_port = htons(__port);
	if( (this->remoteAddress.sin_addr.s_addr = 
        inet_addr(__host.c_str()) ) == (unsigned int)-1 ){
		/* domain name maybe */
		struct hostent *host;
		host = gethostbyname(__host.c_str());
		if( host ){
			this->remoteAddress.sin_addr.s_addr = 
				*(size_t*)host->h_addr_list[0];
			if(0 == ::connect(this->socketHandle, 
				(sockaddr*)&this->remoteAddress, 
				sizeof this->remoteAddress))
				return true;
			return false;
		}else
			return false;
        }else{
            int ret = ::connect(this->socketHandle,
                        (sockaddr*)&this->remoteAddress, sizeof this->remoteAddress);
            return 0 == ret;
	}
}

bool HttpSocket::listen()
{
	::listen(this->socketHandle, 5);
	return true;
}

bool HttpSocket::receive(string& __buffer)
{
	__buffer.clear();
	while(1){
		char ch;
		if(::recv(this->socketHandle, &ch, 1, 0) <= 0){
			return false;
		}
		switch(ch){
		case '\r':
			break;
                case '\n':
			return true;
		default:
			__buffer.push_back(ch);
		}
	}
	return true;
}

bool HttpSocket::send(const string& __buffer)
{
	return ::send(this->socketHandle, (__buffer + "\r\n").c_str(), 
		__buffer.length() + 2, 0) > 0;
}

void HttpSocket::shutdown()
{
	if(this->socketHandle != -1){
//                ::shutdown(this->socketHandle, SHUT_RDWR);
#ifdef __WIN32__
		closesocket(this->socketHandle);
#else
        close(this->socketHandle);
#endif
        this->socketHandle = -1;
	}
}

int HttpSocket::receive(char* __data, int __size)
{
        return ::recv(this->socketHandle, __data, __size, MSG_NOSIGNAL);
}

int HttpSocket::send(const char* __data, int __size)
{
        return ::send(this->socketHandle, __data, __size, MSG_NOSIGNAL);
}

int HttpSocket::getSocketHandle() const
{
	return this->socketHandle;
}

