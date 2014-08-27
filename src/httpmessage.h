#ifndef HTTPMESSAGE_H
#define HTTPMESSAGE_H

#include <string>
#include <vector>
#include "httpsocket.h"
using namespace std;

struct MessageItem{
	string			name;
	string			value;
};

class HttpMessage {
private:
	vector<MessageItem>	items;
public:
	HttpMessage();
	~HttpMessage();

	bool			parseMessage(HttpSocket* __sock);
	void			parseHeader(const string& __line);
	
	const string	getHeader(const string __name);
	void			setHeader(const string __name, const string __value);
	const string	getHeaders();
	bool			removeHeader(const string __name);
};

#endif // HTTPMESSAGE_H
