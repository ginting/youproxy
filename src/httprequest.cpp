#include "httprequest.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

HttpRequest::HttpRequest()
{
    this->keepProxy = false;
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::parseRequest(HttpSocket* __sock)
{
	string line;
	while(line.empty())
		if(!__sock->receive(line))
			return false;
	
	char cmd[10]="", path[9096+4]="", version[10]="";
	sscanf(line.c_str(), "%9s %9096s %9s", cmd, path, version);

	this->command = cmd;
	this->version = version;
	/* parse Message body */
	this->message.parseMessage(__sock);
	/* parse path */
	this->parsePath(path);
	if(this->message.getHeader("proxy-connection") == "close"){
		this->willClose = true;
		this->message.setHeader("Connection", "close");
	}else{
		this->willClose = false;
		this->message.setHeader("Connection", "keep-alive");
        }

    // temporarily do this
    // we are not able to process multi requests per connection now!
    // fix me!!!
    this->message.setHeader("Connection", "close");
	this->message.removeHeader("proxy-connection");

	this->clientSocket = __sock;
	return true;
}

void HttpRequest::setKeepProxy(bool __keep)
{
    this->keepProxy = __keep;
}

const string& HttpRequest::getCommand()
{
	return this->command;
}

const string& HttpRequest::getHost()
{
	return this->host;
}

const string& HttpRequest::getPath()
{
	return this->path;
}

int HttpRequest::getPort()
{
	return this->port;
}

const string& HttpRequest::getVersion()
{
	return this->version;
}

void HttpRequest::parsePath(const string __path)
{
    this->uri = __path;
    if(strncmp(__path.c_str(), "http://", 7) == 0 || strncmp(__path.c_str(), "HTTP://", 7) == 0){
        const size_t hostEnd = __path.find('/', 8);
        if(hostEnd == string::npos)
            return;
        this->host = __path.substr(7, hostEnd - 7);
        this->path = __path.substr(hostEnd, __path.length() - hostEnd);

        parseHost(this->host);
    }else{
        if(this->command == "CONNECT"){
            parseHost(__path);
            this->path = "";
        }else{
            this->path = __path;
            this->host = this->message.getHeader("Host");
            parseHost(this->host);
        }
    }
}

void HttpRequest::parseHost(const string __host)
{
	/* Case localhost:8000 */
    size_t hostEnd = __host.find(':');
	
	/* this->message.setHeader("Host", this->host); */
	
	if(hostEnd == string::npos){
		this->host = __host;
		this->port = 80;
	}else{
		string port = __host.substr(hostEnd + 1, this->host.length() - 
			hostEnd - 1);
		this->port = atoi(port.c_str());
		this->host =__host.substr(0, hostEnd);
	}
}

bool HttpRequest::sendRequest(HttpSocket* __sock)
{
    if(this->keepProxy){
        this->path = this->uri;
        this->message.setHeader("Proxy-Connection", "close");
	}


    string buf = this->command + " " + this->path + " " + this->version + "\r\n";
	buf += this->message.getHeaders();
	if( !__sock->send(buf))
		return false;
	
	/* If we have POST data */
	if(this->command == "POST"){
		int length = atoi(this->message.getHeader("content-length").c_str());
		const int bufferSize = 8192;
		char* buffer = new char[bufferSize];
		while(length > 0){
			int sendSize = length < bufferSize ? length : bufferSize;
			if((sendSize = this->clientSocket->receive(buffer, sendSize)) <= 0)
				break;
			if(__sock->send(buffer, sendSize) != sendSize)
				break;
			length -= sendSize;
		}
        /* bugfix: delete -> delete[] 120917*/
		delete[] buffer;
	}
	return true;
}

bool HttpRequest::wouldClose() const
{
	return this->willClose;
}

HttpMessage& HttpRequest::getMessage()
{
	return this->message;
}

