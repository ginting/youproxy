#include "systemutil.h"

#include "stdio.h"

#ifdef __WIN32__

#include <wininet.h>
#include <windows.h>

typedef struct {
  DWORD dwOption;
  union {
    DWORD    dwValue;
    LPTSTR   pszValue;
    FILETIME ftValue;
  } Value;
} INTERNET_PER_CONN_OPTION, *LPINTERNET_PER_CONN_OPTION;


typedef struct {
  DWORD                      dwSize;
  LPTSTR                     pszConnection;
  DWORD                      dwOptionCount;
  DWORD                      dwOptionError;
  LPINTERNET_PER_CONN_OPTION pOptions;
} INTERNET_PER_CONN_OPTION_LIST, *LPINTERNET_PER_CONN_OPTION_LIST;

//// Options used in INTERNET_PER_CONN_OPTON struct
#define INTERNET_PER_CONN_FLAGS                         1
#define INTERNET_PER_CONN_PROXY_SERVER                  2
#define INTERNET_PER_CONN_PROXY_BYPASS                  3
#define INTERNET_PER_CONN_AUTOCONFIG_URL                4
#define INTERNET_PER_CONN_AUTODISCOVERY_FLAGS           5
//etc.
//// PER_CONN_FLAGS
#define PROXY_TYPE_DIRECT                               0x00000001   // direct to net
#define PROXY_TYPE_PROXY                                0x00000002   // via named proxy
#define PROXY_TYPE_AUTO_PROXY_URL                       0x00000004   // autoproxy URL
#define PROXY_TYPE_AUTO_DETECT                          0x00000008   // use autoproxy detection
#define INTERNET_OPTION_PER_CONNECTION_OPTION   75

BOOL SetConnectionProxy( char * proxyAdressStr , char * connNameStr = NULL)
{
    INTERNET_PER_CONN_OPTION_LIST conn_options;
    BOOL    bReturn;
    DWORD   dwBufferSize = sizeof(conn_options);
    conn_options.dwSize = dwBufferSize;
    conn_options.pszConnection = (TCHAR*)connNameStr;//NULL == LAN

    conn_options.dwOptionCount = 3;
    conn_options.pOptions = new INTERNET_PER_CONN_OPTION[3];

    if(!conn_options.pOptions)
        return FALSE;

    conn_options.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    conn_options.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT|PROXY_TYPE_PROXY;

    conn_options.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    conn_options.pOptions[1].Value.pszValue = (TCHAR*)proxyAdressStr;
    conn_options.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    conn_options.pOptions[2].Value.pszValue = (TCHAR*)"<local>";

    bReturn = InternetSetOptionA(NULL,INTERNET_OPTION_PER_CONNECTION_OPTION, &conn_options, dwBufferSize);

    delete [] conn_options.pOptions;

    InternetSetOptionA(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH , NULL, 0);
    return bReturn;
}

BOOL RemoveConnectionProxy(char* connectionNameStr = NULL)
{
    INTERNET_PER_CONN_OPTION_LIST conn_options;
    BOOL    bReturn;
    DWORD   dwBufferSize = sizeof(conn_options);

    conn_options.dwSize = dwBufferSize;

    conn_options.pszConnection = (TCHAR*)connectionNameStr; //NULL - LAN
    conn_options.dwOptionCount = 1;
    conn_options.pOptions = new INTERNET_PER_CONN_OPTION[conn_options.dwOptionCount];

    if(!conn_options.pOptions)
        return FALSE;

    conn_options.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    conn_options.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT  ;

    bReturn = InternetSetOptionA(NULL,INTERNET_OPTION_PER_CONNECTION_OPTION, &conn_options, dwBufferSize);

    delete [] conn_options.pOptions;
    InternetSetOptionA(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOptionA(NULL, INTERNET_OPTION_REFRESH , NULL, 0);

    return bReturn;
}
#endif //__WIN32__

#ifdef __MAC__

#include <SystemConfiguration/SystemConfiguration.h>
#include <CoreFoundation/CoreFoundation.h>

CFDictionaryRef oldDNS = NULL;
CFStringRef oldDNSInterface = NULL;

int set_global_proxy(int enable, const char* host, int port)
{
    printf("set global proxy %d %s %d\n", enable, host, port);
    SCPreferencesRef pref = SCPreferencesCreate(kCFAllocatorSystemDefault, CFSTR("setproxy"), 0); 
    if(!pref){
        printf("Failed to open preference.\n");
        return 1;
    }   
    CFDictionaryRef set = SCPreferencesPathGetValue(pref, CFSTR("/NetworkServices/"));
    if(!set){
        printf("Failed to get network services.\n");
    }   
    CFMutableDictionaryRef mset = CFDictionaryCreateMutableCopy(0, 0, set);

    SCDynamicStoreRef theDynamicStore = SCDynamicStoreCreate(nil, CFSTR("setproxy"), nil, nil);
    CFDictionaryRef returnedPList;
    returnedPList = (CFDictionaryRef)SCDynamicStoreCopyValue(theDynamicStore, CFSTR("State:/Network/Global/IPv4"));
    CFStringRef primaryService = NULL;
    if(returnedPList){
        primaryService = (CFStringRef)CFDictionaryGetValue(returnedPList, CFSTR("PrimaryService"));
    }   

    size_t size = CFDictionaryGetCount(set);
    CFTypeRef *keysTypeRef = (CFTypeRef *) malloc( size * sizeof(CFTypeRef) );
    CFTypeRef *valuesTypeRef = (CFTypeRef *) malloc( size * sizeof(CFTypeRef) );
    CFDictionaryGetKeysAndValues(set, (const void **) keysTypeRef, (const void**)valuesTypeRef);
    const void **keys = (const void **) keysTypeRef;
    printf("Number of interfaces = %ld\n", size);
    int i;
    for(i=0; i<size && keysTypeRef[i]; i++){
        Boolean success;
        CFStringRef service = (CFStringRef)keysTypeRef[i];

        printf("Setting interface %d\n", i);

        if(enable == 1 && primaryService && CFStringCompare(service, primaryService, kCFCompareCaseInsensitive) != 0){
            continue;
        }

        CFTypeRef value = valuesTypeRef[i];
        if(!value){
            continue;
        }
        CFDictionaryRef face = (CFDictionaryRef)value;

        CFMutableDictionaryRef mface = CFDictionaryCreateMutableCopy(0, 0, face);
        CFMutableDictionaryRef mproxy = NULL;
        CFDictionaryRef proxy = (CFDictionaryRef)CFDictionaryGetValue(mface, CFSTR("Proxies"));
        if(NULL == proxy){
            if(enable == 0){
                CFRelease(mface);
                continue;
            }
            printf("proxy = %p, try to create it\n", proxy);
            mproxy = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        }else{
            mproxy = CFDictionaryCreateMutableCopy(0, 0, proxy);
            if(mproxy == NULL)
                return 4;
        }

        if(enable){
            CFStringRef cfHost = CFStringCreateWithCString(0, host, kCFStringEncodingASCII);
            CFDictionarySetValue(mproxy, CFSTR("HTTPEnable"), CFNumberCreate(0, kCFNumberIntType, &enable));
            CFDictionarySetValue(mproxy, CFSTR("HTTPProxy"), cfHost);
            CFDictionarySetValue(mproxy, CFSTR("HTTPPort"), CFNumberCreate(0, kCFNumberIntType, &port));
            CFDictionarySetValue(mproxy, CFSTR("HTTPSEnable"), CFNumberCreate(0, kCFNumberIntType, &enable));
            CFDictionarySetValue(mproxy, CFSTR("HTTPSProxy"), cfHost);
            CFDictionarySetValue(mproxy, CFSTR("HTTPSPort"), CFNumberCreate(0, kCFNumberIntType, &port));
            CFRelease(cfHost);
        }else{
            CFDictionaryRemoveValue(mproxy, CFSTR("HTTPEnable"));
            CFDictionaryRemoveValue(mproxy, CFSTR("HTTPProxy"));
            CFDictionaryRemoveValue(mproxy, CFSTR("HTTPPort"));
            CFDictionaryRemoveValue(mproxy, CFSTR("HTTPSEnable"));
            CFDictionaryRemoveValue(mproxy, CFSTR("HTTPSProxy"));
            CFDictionaryRemoveValue(mproxy, CFSTR("HTTPSPort"));
        }

        CFDictionarySetValue(mface, CFSTR("Proxies"), mproxy);
        CFDictionarySetValue(mset, service, mface);

        SCPreferencesPathSetValue(pref, CFSTR("/NetworkServices/"), mset);
        success = SCPreferencesCommitChanges(pref);
        printf("success: %d\n", success);
        success = SCPreferencesApplyChanges(pref);
        printf("success: %d\n", success);

        CFRelease(mface);
        CFRelease(mproxy);
    }

    CFRelease(mset);
    CFRelease(pref);
    free(keysTypeRef);
    free(valuesTypeRef);
}

#endif //__MAC__


SystemUtil::SystemUtil()
{
}

void SystemUtil::disableSystemProxy()
{
#ifdef __WIN32__
    int ret = RemoveConnectionProxy();
#endif
#ifdef __MAC__
    set_global_proxy(0, "", 1998);
#endif
}


void SystemUtil::enableSystemProxy()
{
}

void SystemUtil::setSystemProxy(const char* proxyServer, int proxyPort)
{
#ifdef __WIN32__
    char addr[100];
    sprintf(addr,"%s:%d",proxyServer,proxyPort);

    int ret = SetConnectionProxy(addr);
#endif

#ifdef __MAC__
    set_global_proxy(1, "127.0.0.1", proxyPort);
#endif

}
