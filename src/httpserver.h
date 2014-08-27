#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "cppthread.h" // Base class: CppThread
#include "httpsocket.h"
#include "httpconnection.h"
#include <vector>
using namespace std;

class HttpServer : public CppThread {
private:
	vector<HttpConnection*>		connections;
	HttpSocket		listenSocket;
	bool			closed;
public:
	HttpServer();
	~HttpServer();

	void	process();
	void	checkConnections();
    void    close();
};

#endif // HTTPSERVER_H
