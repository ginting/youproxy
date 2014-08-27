#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "cppthread.h" // Base class: CppThread
#include "httpsocket.h"
#include "httprequest.h"
#include "httpresponse.h"
#include <ctime>

class HttpConnection : public CppThread {
private:
    HttpSocket*             clientSocket;
    HttpSocket*             remoteSocket;
    HttpRequest*            request;
    HttpResponse*           response;

    bool                closed;
    int                 addNewLine;


    bool                transferTrunkedData();
    bool                transferData();
    void                doAddNewLine();
    string              parseConnectHost(const string& host);
public:
    /* Shrimp is our friend! */
    friend class        HttpShrimp;

    HttpConnection(HttpSocket* __sock);
    ~HttpConnection();
    void                process();
    void                doCONNECT();
    bool                isClosed()const;
    void                close();

    HttpRequest*        getRequest();
    HttpResponse*        getResponse();

    void                updateBlockSize(int __size);
};

#endif // HTTPCONNECTION_H
