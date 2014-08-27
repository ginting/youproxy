#pragma once
#define TRAY_NOTIFY 3001

static BOOL InitWindow( HINSTANCE hInstance, int nCmdShow );
void createCtrls(HWND&hwnd);
BOOL CALLBACK WinProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
void onTray(WPARAM wParam,LPARAM lParam);
void setIcon(bool disabled ,bool deL = false);
void setTips(const char *tips);

static bool firstTimeToAdd = true;

class icon_maker
{
public:
	icon_maker(HINSTANCE hInstance, LPCTSTR lpIconName){
		picon = LoadIcon(hInstance, lpIconName);
	}
	~icon_maker(){
		DestroyIcon(picon); 
	}
	HICON get_icon(){
		return picon;
	}
private:
	HICON picon;
};