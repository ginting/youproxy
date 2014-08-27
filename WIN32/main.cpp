
#define _WIN32_IE 0x0500 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "io.h"
#include "windows.h"
#include <windowsx.h>
#include "CommCtrl.h"
#include <windef.h>
#include <gdiplus.h>
//
#include "httpsocket.h"
#include "youconfig.h"
#include "httpserver.h"
#include "systemutil.h"

#include <signal.h>
//std
#include <memory>
#include <iostream>
#include <ctime>

#include "resource.h"
#include "main.h"


using namespace std;

HWND wnd;
HINSTANCE hinstance;
HMENU menu;
SystemUtil ieSetter;
bool bAgentOn = false;
const UINT WM_TASKBARCREATED = ::RegisterWindowMessage( "TaskbarCreated");
const static wchar_t statement[]=
L"YouProxy For Windows\n\
========================\n\
\n\
* YouProxy 是一款免费软件，欢迎所有人提供BUG信息及建议。\n\
* 请访问 http://code.google.com/p/icefox/ 获取最新信息 。\n\
* 版本信息：20130103\n \
      \n\
      \n\
* 使用方法：右键托盘菜单中，选择启用 YouProxy 。\n \
";

int PASCAL WinMain( HINSTANCE hInstance, 
		HINSTANCE hPrevInstance, 
		LPSTR lpCmdLine,
		int nCmdShow)
{
	MSG msg;

	if ( !InitWindow( hInstance, nCmdShow ) )
		return FALSE;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

        ieSetter.disableSystemProxy();
        setIcon(true,true);
	return msg.wParam;
}

static BOOL InitWindow( HINSTANCE hInstance, int nCmdShow )
{
	WNDCLASS wc; 
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(MAINAPP));
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground =(HBRUSH) CreateSolidBrush(RGB(240,240,240));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "youagent";

	RegisterClass( &wc );

        long screenw,screenh;
        screenw=GetSystemMetrics(SM_CXSCREEN);
        screenh=GetSystemMetrics(SM_CYSCREEN);
        long ww= 599 ,hh= 280 ;
	wnd = CreateWindowW(
			L"youagent", 
			L" YouProxy ", //title
			WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX, 
			(screenw-ww)/2,
			(screenh-hh)/2,
			ww,
			hh,
			NULL,
			NULL,
			hInstance,
			NULL );
	if( !wnd ) return FALSE;
	//
	SendMessage(wnd, WM_SETICON, TRUE, (LPARAM) 
                    LoadIcon((HINSTANCE) GetWindowLong(wnd, GWL_HINSTANCE),MAKEINTRESOURCE(MAINAPP)));
  createCtrls(wnd);
  hinstance=hInstance;
  menu=LoadMenu(hinstance, MAKEINTRESOURCE (IDR_MENU1)) ;

  static WSADATA wsa_data;
  if(WSAStartup((WORD)(1<<8|1), &wsa_data) != 0) exit(1);

  YouConfig::instance()->loadFromNetwork();
  HttpServer* server = new HttpServer();
  server->startThread();
  
	ShowWindow( wnd, SW_SHOW);
	setIcon(true);
	setTips("YouProxy is DISabled!");
	return TRUE;
}
///
BOOL CALLBACK WinProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch( message )
    {
    case WM_CREATE:
      {
        ieSetter.disableSystemProxy();
	break;
      }
    case  WM_COMMAND:
      {
	switch( LOWORD( wParam))
	  {
	  case IDM_1://enable
	    {
	      if (!bAgentOn)
		{
		  CheckMenuItem( menu, IDM_1, MF_BYCOMMAND | MF_CHECKED); 
		  ///set local ie settings
		  ieSetter.setSystemProxy("127.0.0.1",1998);
		  //flag
		  bAgentOn = true;
		  //tray icon
		  setIcon(false);
                  setTips("YouProxy is ENabled!");
		}
	      else {
		CheckMenuItem( menu, IDM_1, MF_BYCOMMAND | MF_UNCHECKED); 
		ieSetter.disableSystemProxy();
		bAgentOn = false;
		setIcon(true);
                setTips("YouProxy is DISabled!");
	      }
	      break;
	    }
	  case IDM_2://close
	    {
	      DestroyWindow(wnd);
	      break;
	    }
          case IDM_3://website
            {
              SHELLEXECUTEINFO sei;
              ::ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
              sei.cbSize = sizeof( SHELLEXECUTEINFO );	
              sei.lpVerb = TEXT( "open" );
              sei.lpFile = "https://code.google.com/p/icefox";
              sei.nShow = SW_SHOWNORMAL;		
              ShellExecuteEx(&sei);
              break;
            }
          case IDM_4://website
            {
              SHELLEXECUTEINFO sei;
              ::ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
              sei.cbSize = sizeof( SHELLEXECUTEINFO );	
              sei.lpVerb = TEXT( "open" );
              sei.lpFile = "https://code.google.com/p/icefox/downloads/list";
              sei.nShow = SW_SHOWNORMAL;
              ShellExecuteEx(&sei);
              break;
            }
          case IDM_5://website
            {
              SHELLEXECUTEINFO sei;
              ::ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
              sei.cbSize = sizeof( SHELLEXECUTEINFO );	
              sei.lpVerb = TEXT( "open" );
              sei.lpFile = "https://code.google.com/p/icefox/issues/list";
              sei.nShow = SW_SHOWNORMAL;
              ShellExecuteEx(&sei);
              break;
            }
	  case IDBT:
	    {
	      ShowWindow(wnd,SW_HIDE);
	      break;
	    }
	    
	  }
	break;
      }
    case TRAY_NOTIFY:
      {
	onTray(wParam,lParam);
	break;
      }
		
    case WM_DESTROY:
      {
				
	PostQuitMessage( 0 );
	break;
      }
    case WM_QUERYENDSESSION:
      {
        ieSetter.disableSystemProxy();
        //MessageBox(0,"hello",0,0);
        return 1;
      }
    case WM_ENDSESSION:
      {
        break;
      }

    default:
      if (message == WM_TASKBARCREATED)
	{
	  firstTimeToAdd = true;
	  setIcon(!bAgentOn);
        }
    }
  return DefWindowProc(hWnd, message, wParam, lParam);
} 

void createCtrls(HWND&hwnd)
{

	HFONT hfont0 = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, 1, 400, 0, 0, 0, ("Ms Shell Dlg"));
	HWND hCtrl0_0 =CreateWindowW( L"BUTTON",L"隐藏",
				      WS_VISIBLE|WS_CHILD, 486, 205, 99, 36,
					hwnd,(HMENU)IDBT,hinstance,NULL);
	
	SendMessage(hCtrl0_0, WM_SETFONT, (WPARAM)hfont0, TRUE);
	HWND hCtrl0_1 =CreateWindowW( L"STATIC",statement,
			       WS_VISIBLE | WS_CHILD | WS_GROUP | SS_LEFT,11, 20, 399, 184,
			       hwnd, NULL,hinstance,NULL);
	SendMessage(hCtrl0_1, WM_SETFONT, (WPARAM)hfont0, TRUE);

}


void onTray(WPARAM wParam,LPARAM lParam)
{
  UINT uMouseMsg = (UINT) lParam;

  switch(uMouseMsg){

  case WM_LBUTTONDBLCLK:
    {
      //show or hide the dialog
      if(IsWindowVisible(wnd))
	ShowWindow(wnd,SW_HIDE);
      else
	ShowWindow(wnd,SW_NORMAL);
      SetForegroundWindow(wnd);
      break;
    }
  case WM_RBUTTONDOWN:
    {
      HMENU popmenu= GetSubMenu(menu,0);
      POINT point;
      GetCursorPos(&point);
      SetForegroundWindow(wnd);
      TrackPopupMenu (popmenu, TPM_RIGHTBUTTON, point.x, point.y, 0, wnd, NULL) ;
      break;

    }
  default: 
    break;
  }
}


NOTIFYICONDATA tnid;

void setIcon(bool disabled , bool deL )
{
  tnid.cbSize = sizeof(NOTIFYICONDATA); 
  tnid.hWnd = wnd; 
  tnid.uID = 0x1; //Resourcename
  tnid.uCallbackMessage = TRAY_NOTIFY;

  tnid.uFlags = NIF_MESSAGE |NIF_ICON | NIF_TIP; //
  icon_maker im(hinstance, MAKEINTRESOURCE(disabled ? AGENTOFF : MAINAPP));
  tnid.hIcon = im.get_icon();
  lstrcpyn(tnid.szTip, "YouProxy for Windows", sizeof(tnid.szTip));
  if (deL)
    {
      Shell_NotifyIcon(NIM_DELETE,&tnid);
      return;
    }
  if (firstTimeToAdd){
    Shell_NotifyIcon(NIM_ADD, &tnid); 
    firstTimeToAdd = false;
  }
  else{
    Shell_NotifyIcon(NIM_MODIFY, &tnid);
  }
}

void setTips(const char* tips)
{
  tnid.uFlags = NIF_INFO;
  tnid.uVersion = NOTIFYICON_VERSION;
  tnid.uTimeout = 1000;
  tnid.dwInfoFlags = NIIF_INFO;
  wsprintf( tnid.szInfoTitle, "%s"," YouProxy " );
  wsprintf( tnid.szInfo,"%s",      tips     );
  //wcscpy_s( tnid.szTip,       _T("tip")       );
  Shell_NotifyIcon( NIM_MODIFY, &tnid );
}

