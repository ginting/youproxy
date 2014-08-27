#include "httpresponse.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

HttpResponse::HttpResponse()
{
}

HttpResponse::~HttpResponse()
{
}

const string& HttpResponse::getReason()
{
	return this->reason;
}

const string& HttpResponse::getStatus()
{
	return this->status;
}

const string& HttpResponse::getVersion()
{
	return this->version;
}

bool HttpResponse::isChunked() const
{
	return this->chunked;
}

bool HttpResponse::parseResponse(HttpSocket* __sock)
{
	string line;
	while(line.empty())
		if(!__sock->receive(line))
			return false;
	
	char version[10]="", reason[128+4]="", status[10]="";
	sscanf(line.c_str(), "%9s %9s %128s", version, status, reason);
	
	this->status = status;
	this->version = version;
	this->reason = reason;
	/* parse Message body */
	this->message.parseMessage(__sock);
	/* chunked */
	if(this->message.getHeader("transfer-encoding") == "chunked"){
		this->chunked = true;
		this->message.removeHeader("transfer-encoding");
	}else{
		this->chunked = false;
	}
	
        this->willClose = false;
	/* check content length */
	string length = this->message.getHeader("content-length");
	if(length == ""){
		this->willClose = true;
		this->contentLength = 0xffffffff / 2;
	}else{
                this->contentLength = atol(length.c_str());
	}
	
	/* check keep-alive */
	if(this->message.getHeader("connection") == "close")
                this->willClose = true;
	
	this->remoteSocket = __sock;
	return true;
}

bool HttpResponse::sendResponse(HttpSocket* __sock)
{
	string buf = this->version + " " + this->status + " " + this->reason + 
		"\r\n";
	buf += this->message.getHeaders();
	
	return __sock->send(buf);
}

bool HttpResponse::wouldClose() const
{
	return this->willClose;
}

unsigned int HttpResponse::getContentLength() const
{
	return this->contentLength;
}

HttpMessage& HttpResponse::getMessage()
{
	return this->message;
}

