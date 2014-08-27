#ifndef SYSTEMUTIL_H
#define SYSTEMUTIL_H

//#include <QString>

class SystemUtil
{
  //private:
  //  SetConnectionProxy(
 public:
    SystemUtil();
    //    static void setSystemProxy(QString proxyServer, int proxyPort);
    static void setSystemProxy(const char* proxyServer, int proxyPort);
    static void enableSystemProxy();
    static void disableSystemProxy();
};

#endif // SYSTEMUTIL_H
