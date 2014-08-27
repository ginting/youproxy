#include "httpserver.h"
#include <iostream>

HttpServer::HttpServer()
{
	this->closed = false;
}

HttpServer::~HttpServer()
{
}

void HttpServer::close()
{
    this->listenSocket.shutdown();
    this->closed = true;
}

void HttpServer::process()
{
        if(!this->listenSocket.bind(1998)){
            cerr << "Failed to bind server port 1998." << endl;
        }

	this->listenSocket.listen();
	
	while(!this->closed){
		HttpSocket* sock = this->listenSocket.accept();
		if(sock == NULL)
			continue;
		
		HttpConnection* conn = new HttpConnection(sock);
		conn->startThread();
		
		this->connections.push_back(conn);
		checkConnections();
	}
	
	
	checkConnections();
    cout << "httpserver ends." << endl;
}

void HttpServer::checkConnections()
{
	if(this->closed){
		for(vector<HttpConnection*>::iterator it = this->connections.begin();
			it!=this->connections.end(); it++)
			(*it)->close();
	}
	for(vector<HttpConnection*>::iterator it = this->connections.begin();
		it!=this->connections.end(); ){
		HttpConnection* conn = *it;
		if(conn->isClosed()){
			conn->joinThread();
			delete conn;
			this->connections.erase(it);
		}else{
			it ++;
		}
	}
}

