/*
  linux/unix version
*/

#include "httpsocket.h"
#include "youconfig.h"
#include "httpserver.h"
//#include "systemutil.h"

#include <signal.h>
#include <iostream>
#include <ctime>
#include "stdlib.h"

using namespace std;

int main(int argc, char *argv[])
{

  signal(SIGPIPE, SIG_IGN);

  ///set local network settings
  /*  int res = putenv("http_proxy=http://127.0.0.1:1998/");
  if (res==-1)
	    cout<<"ERR: http proxy setting failed!"<<endl;
  res = putenv("https_proxy=http://127.0.0.1:1998/");
  if (res==-1)
	    cout<<"ERR: https proxy setting failed!"<<endl;
	    cout<<getenv("http_proxy")<<endl;*/
  ///

  YouConfig::instance()->loadFromNetwork();
  HttpServer* server = new HttpServer();
  cout << "Start proxy server at localhost:1998\n"<<endl;
  server->startThread();

  /*
    while(1){
    sleep(1);
    }
  */
  //while(1);
  server->joinThread();
  cout << "Exit ..." <<endl;
}
