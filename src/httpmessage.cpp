#include "httpmessage.h"
#include <iostream>
#include <cstring>
using namespace std;

#define stricmp strcasecmp

HttpMessage::HttpMessage()
{
}

HttpMessage::~HttpMessage()
{
}

const string HttpMessage::getHeader(const string __name)
{
        for(size_t i=0; i<this->items.size(); i++)
		if(stricmp(__name.c_str(), this->items[i].name.c_str()) == 0)
			return this->items[i].value;
	return "";
}

bool HttpMessage::parseMessage(HttpSocket* __sock)
{
	string line;
	this->items.clear();
	while(__sock->receive(line) && line != ""){
		parseHeader(line);
	}
	return true;
}

void HttpMessage::setHeader(const string __name, const string __value)
{
        for(size_t i=0; i<this->items.size(); i++)
		if(stricmp(__name.c_str(), this->items[i].name.c_str()) == 0){
			this->items[i].value = __value;
			return;
		}
	/* Create new one */
	MessageItem item;
	item.name = __name;
	item.value = __value;
	this->items.push_back(item);
}

void HttpMessage::parseHeader(const string& __line)
{
	MessageItem item;
        size_t equPos = __line.find(':');
	if(equPos == string::npos)
		return;
	item.name = __line.substr(0, equPos);
	
	equPos ++;
	if(equPos > __line.length())
		return;
	while(__line[equPos] == ' ')
		equPos ++;
	item.value = __line.substr(equPos, __line.length() - equPos);
	
	this->items.push_back(item);
}

const string HttpMessage::getHeaders()
{
	string buf;
        for(size_t i=0; i<this->items.size(); i++)
		buf += this->items[i].name + ": " + this->items[i].value + "\r\n";
	return buf;
}

bool HttpMessage::removeHeader(const string __name)
{
        for(size_t i=0; i<this->items.size(); i++)
		if(stricmp(__name.c_str(), this->items[i].name.c_str()) == 0){
			this->items.erase(this->items.begin() + i);
			return true;
		}
	return false;
}

