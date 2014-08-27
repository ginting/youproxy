#ifndef HTTPSOCKET_H
#define HTTPSOCKET_H

#include <string>
using namespace std;
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


class HttpSocket {
private:
	int			socketHandle;
	sockaddr_in	remoteAddress;
	sockaddr_in	localAddress;
public:
	HttpSocket();
	~HttpSocket();
	bool		bind(int __port);
	bool		connect(string __host, int __port);
	bool		send(const string& __buffer);
	bool		receive(string& __buffer);
	int			send(const char* __data, int __size);
	int			receive(char* __data, int __size);
	bool		listen();
	HttpSocket*	accept();
	void		shutdown();
	int			getSocketHandle() const;
};

#endif // HTTPSOCKET_H
