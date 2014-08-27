#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "httpsocket.h"
#include "httpmessage.h"
#include <string>
using namespace std;

class HttpRequest {
private:
	string		version;
	string		command;
    string      uri;
	string		path;
	string		host;
	int			port;
	HttpMessage	message;
	HttpSocket*	clientSocket;
	bool		willClose;
    bool        keepProxy;
public:
	HttpRequest();
	~HttpRequest();
	bool		parseRequest(HttpSocket* __sock);
    bool		sendRequest(HttpSocket* __sock);
	void		parsePath(const string __path);
    void        setKeepProxy(bool __keep);
	const string&	getVersion();
	const string&	getPath();
	const string&	getHost();
	const string&	getCommand();
    int	        getPort();
	void		parseHost(const string __host);
	bool		wouldClose()const;
	HttpMessage&	getMessage();
};

#endif // HTTPREQUEST_H
