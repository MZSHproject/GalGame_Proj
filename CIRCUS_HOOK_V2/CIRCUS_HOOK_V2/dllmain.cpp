// dllmain.cpp : 定义 DLL 应用程序的入口点。
//For D.S.I.F -Dal Segno In Future-
#include "stdafx.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include "detours.h"
#pragma comment(lib, "detours.lib")
using namespace std;
ofstream TXT("Text.txt");

typedef HFONT(WINAPI *fnCreateFontA)(
	int nHeight, // logical height of font height
	int nWidth, // logical average character width
	int nEscapement, // angle of escapement
	int nOrientation, // base-line orientation angle
	int fnWeight, // font weight
	DWORD fdwItalic, // italic attribute flag
	DWORD fdwUnderline, // underline attribute flag
	DWORD fdwStrikeOut, // strikeout attribute flag
	DWORD fdwCharSet, // character set identifier
	DWORD fdwOutputPrecision, // output precision
	DWORD fdwClipPrecision, // clipping precision
	DWORD fdwQuality, // output quality
	DWORD fdwPitchAndFamily, // pitch and family
	LPCSTR lpszFace // pointer to typeface name string
	);
fnCreateFontA CreateFontAOLD;
HFONT WINAPI CreateFontAEx(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
	fdwCharSet = 0x86;
	fnWeight = FW_SEMIBOLD;
	return CreateFontAOLD(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, "黑体");
}

PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND(WINAPI *pfuncCreateWindowExA)(
	DWORD dwExStyle,
	LPCTSTR lpClassName,
	LPCTSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam);
HWND WINAPI NewCreateWindowExA(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	const char* szWndName = "Test";

	return ((pfuncCreateWindowExA)g_pOldCreateWindowExA)(dwExStyle, lpClassName, (LPCTSTR)szWndName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

PVOID g_pOldCreateFontIndirectA = CreateFontIndirectA;
typedef HFONT(WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
HFONT WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	lplf->lfCharSet = GB2312_CHARSET;
	strcpy(lplf->lfFaceName, "黑体");

	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

LPWSTR ctowJIS(char* str)
{
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(932, 0, str, -1, NULL, 0); //计算长度
	LPWSTR out = new wchar_t[dwMinSize];
	MultiByteToWideChar(932, 0, str, -1, out, dwMinSize);//转换
	return out;
}

char* wtocGBK(LPCTSTR str)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(936, NULL, str, -1, NULL, 0, NULL, FALSE); //计算长度
	char *out = new char[dwMinSize];
	WideCharToMultiByte(936, NULL, str, -1, out, dwMinSize, NULL, FALSE);//转换
	return out;
}

static void make_console() {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	cout << "Console\n" << endl;
}
/*可以在这里hook，导出的结果应该是脚本执行过程。
char * (*OldStr)(const char *a1, const char *a2);
char *__cdecl MyStr(const char *a1, const char *a2)
{
	_asm
	{
		pushad
	}
	char* ret;
	int index;
	ret = (char*)a1;
	index = *(int*)ret;
	char* Pstr = ret + sizeof(int);
	cout <<  index << "|" << wtocGBK(ctowJIS(Pstr)) << endl;
	TXT <<  index << "|" << wtocGBK(ctowJIS(Pstr)) << endl;
	_asm
	{
		popad
	}
	return OldStr(a1, a2);
}*/
//只获取文本
const char * (*OldStr)(char *a1, char *a2);
const char * MyStr(char *a1, char *a2)
{
	_asm
	{
		pushad
	}
	int index;
	index = (int)a1;
	cout <<index<< "|"<< wtocGBK(ctowJIS(a1)) << endl;
	TXT << index << "|" << wtocGBK(ctowJIS(a1)) << endl;
	wchar_t* tmpStr = ctowJIS(a1);
	if (wcscmp(tmpStr,L"「おー……今日は凄いな」")==0)
	{
		strcpy(a1, "「中文测试ABCabc123ＡＢＣａｂｃ１２３」");
	}//@sあれから１年
	if (wcscmp(tmpStr, L"@sあれから１年") == 0)
	{
		strcpy(a1, "@s在那之后过了一年");
	}//現在の通常文字表示速度で表示しています。
	if (wcscmp(tmpStr, L"現在の通常文字表示速度で表示しています。") == 0)
	{
		strcpy(a1, "这是一般文字的显示速度。");
	}//切り替えはオートに依存しています。
	if (wcscmp(tmpStr, L"切り替えはオートに依存しています。") == 0)
	{
		strcpy(a1, "然后是自动模式的速度。");
	}//切り替えはオートに依存しています。
	_asm
	{
		popad
	}
	return OldStr(a1,a2);
}

PVOID g_pOldSetWindowTextA = NULL;
typedef int (WINAPI *PfuncSetWindowTextA)(HWND hwnd, LPCTSTR lpString);
int WINAPI NewSetWindowTextA(HWND hwnd, LPCTSTR lpString)
{
	if (!memcmp(lpString, "\x93\xD6\x96\xE7", 4))
	{
		strcpy((char*)(LPCTSTR)lpString, "敦也");
		return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
	}
	if (!memcmp(lpString, "\x8D\x82\x91\xBA", 4))
	{
		strcpy((char*)(LPCTSTR)lpString, "高村");
		return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
	}
	//strcpy((char*)(LPCTSTR)lpString, "敦也");
	else 
	{
		return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
	}
}
typedef int (WINAPI *fnMessageboxA)(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType
	);
fnMessageboxA MessageBoxAOLD;
int WINAPI MessageBoxAEx(_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType)
{
	return MessageBoxAOLD(hWnd, "确定要结束吗?", "提示", uType);
}

void Hook()
{
	DWORD BaseAddr = (DWORD)GetModuleHandle(NULL);
	cout << "BaseAddress:0x" << hex << BaseAddr << endl;
	TXT << "BaseAddress:0x" << hex << BaseAddr << endl;
	*(DWORD*)&OldStr = BaseAddr + 0x3c030;
	cout << "HookAddress:0x" << hex << (BaseAddr + 0x3c030) <<"\n"<< endl;
	TXT << "HookAddress:0x" << hex << (BaseAddr + 0x3c030) << endl;
	CreateFontAOLD = (fnCreateFontA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontA");
	g_pOldSetWindowTextA = DetourFindFunction("USER32.dll", "SetWindowTextA");
	MessageBoxAOLD = (fnMessageboxA)GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxA");
	DetourTransactionBegin();
	DetourAttach((void**)&OldStr, MyStr);
	DetourAttach((void**)&CreateFontAOLD, CreateFontAEx);
	DetourAttach(&g_pOldCreateFontIndirectA, NewCreateFontIndirectA);
	DetourAttach(&g_pOldCreateWindowExA, NewCreateWindowExA);
	DetourAttach(&g_pOldSetWindowTextA, NewSetWindowTextA);
	DetourAttach((void**)&MessageBoxAOLD, MessageBoxAEx);
	if(DetourTransactionCommit()!=NOERROR){ MessageBox(NULL, L"ERROR", L"Crescendo", MB_OK); }
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		make_console();
		Hook();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void dummy(void) {
	return;
}
/*
char * OLDSTR(int String, int a2, LPCSTR lpString, int a4, int a5)
{
	cout << "lpString:" << lpString << endl;
	cout << " lpString:" << String << endl;
	flog << " lpString:" << lpString << endl;
	flog <<  " lpString:" << String << endl;
	return OLDSTR(String, a2, lpString,  a4,  a5);
}*/

