#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "httpsocket.h"
#include "httpmessage.h"

class HttpResponse {
private:
	string			status;
	string			version;
	string			reason;
	HttpMessage		message;
	bool			willClose;
	bool			chunked;
	HttpSocket*		remoteSocket;
	unsigned int	contentLength;
public:
	HttpResponse();
	~HttpResponse();
	bool			parseResponse(HttpSocket* __sock);
	bool			sendResponse(HttpSocket* __sock);
	const string&	getStatus();
	const string&	getReason();
	const string&	getVersion();
	bool			wouldClose() const;
	bool			isChunked() const;
	unsigned int	getContentLength()const;
	HttpMessage&	getMessage();
};

#endif // HTTPRESPONSE_H
