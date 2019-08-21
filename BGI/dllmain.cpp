// dllmain.cpp : 定义 DLL 应用程序的入口点。
//For 11月のアルカディア
#include "stdafx.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include "detours.h"
#pragma comment(lib, "detours.lib")
using namespace std;
ofstream TXT("Text.txt");
typedef LCID(WINAPI* fnGetUserDefaultLCID)(void);
typedef LANGID(WINAPI* fnGetSystemDefaultLangID)(void);
typedef LANGID(WINAPI* fnGetSystemDefaultUILanguage)(void);
typedef bool (WINAPI* fnSetWindowTextA)(HWND hWnd, LPCSTR lpString);
typedef int (WINAPI* fnEnumFontFamiliesExA)(
	HDC           hdc,
	LPLOGFONTA    lpLogfont,
	FONTENUMPROCA lpProc,
	LPARAM        lParam,
	DWORD         dwFlags
	);
fnGetSystemDefaultLangID pGetSystemDefaultLangID;
fnGetSystemDefaultUILanguage pGetSystemDefaultUILanguage;
fnGetUserDefaultLCID pGetUserDefaultLCID;
fnSetWindowTextA pSetWindowTextA;
fnEnumFontFamiliesExA pEnumFontFamiliesExA;

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

void memcopy(void* dest, void*src, size_t size)
{
	DWORD oldProtect;
	VirtualProtect(dest, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dest, src, size);
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
//什么鬼玩意儿这么多，还不如上asm算了。
int (*OldStr)(int a1, DWORD *a2, int a3, BYTE *a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13);
int  MyStr(int a1, DWORD *a2, int a3, BYTE *a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13)
{
	_asm
	{
		pushad
	}
	char* ret;
	int index;
	int rep;
	ret = (char*)a2;//如果直接返回OldStr会无法运行
	index = *(int*)ret;
	char* Pstr = (char*)ret + sizeof(int)-4;
	//cout <<  index << "|" << wtocGBK(ctowJIS(Pstr)) << endl;
	TXT <<  index << "|" << wtocGBK(ctowJIS(Pstr)) << endl;
	wchar_t* tmpStr = ctowJIS(Pstr);
	if (wcscmp(tmpStr,L"もはや動ける状態ではなかった。")==0)
	{
		strcpy(Pstr, "已经处于无法动弹的状态了。");
	}
	if (wcscmp(tmpStr, L"全身の至る所に切り傷が刻まれて、そこから流れる血は\n服を赤く染めている。") == 0)
	{
		strcpy(Pstr, "全身上下到处都是伤口，从那里流出的血液\n染红了衣服。");
	}
	_asm
	{
		popad
	}
	return OldStr(a1, a2, a3, a4,a5,a6,a7,a8,a9,a10,a11,a12,a13);
}

void Check()
{
	BYTE Patch1[] = { 0xFE };
	BYTE Patch2[] = { 0x85 };

	memcopy((void*)0x42aae8, Patch1, sizeof(Patch1));
	memcopy((void*)0x487c95, Patch1, sizeof(Patch1));
	memcopy((void*)0x494001, Patch1, sizeof(Patch1));
	//memcopy((void*)0x47fe08, Patch2, sizeof(Patch2));

}


LCID WINAPI newGetUserDefaultLCID()
{
	return 0x411;
}

LANGID WINAPI newGetSystemDefaultLangID()
{
	return 0x411;
}

LANGID WINAPI newGetSystemDefaultUILanguage()
{
	return 0x411;
}

bool WINAPI newSetWindowTextA(HWND hw, LPCSTR lps)
{
		wchar_t newtitle[] = L"【白井木学园汉化组】11月的理想乡 - 简体中文补丁V1.0（此补丁禁止一切录播直播和商业活动）";
		return SetWindowTextW(hw, newtitle);
}


void Hook()
{
	DWORD BaseAddr = (DWORD)GetModuleHandle(NULL);
	cout << "BaseAddress:0x" << hex<< BaseAddr << endl;
	TXT << "BaseAddress:0x" <<hex << BaseAddr << endl;
	*(DWORD*)&OldStr = BaseAddr + 0x32d20;//offset，每个游戏不同
	cout << "HookAddress:0x" << hex << (BaseAddr + 0x32d20) <<"\n"<< endl;
	TXT << "HookAddress:0x" << hex << (BaseAddr + 0x32d20) << endl;
	CreateFontAOLD = (fnCreateFontA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontA");
	pGetSystemDefaultLangID = (fnGetSystemDefaultLangID)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetSystemDefaultLangID");
	pGetSystemDefaultUILanguage = (fnGetSystemDefaultUILanguage)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetSystemDefaultUILanguage");
	pGetUserDefaultLCID = (fnGetUserDefaultLCID)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetUserDefaultLCID");
	pSetWindowTextA = (fnSetWindowTextA)GetProcAddress(GetModuleHandle(L"User32.dll"), "SetWindowTextA");
	DetourTransactionBegin();
	DetourAttach((void**)&OldStr, MyStr);
	DetourAttach((void**)&CreateFontAOLD, CreateFontAEx);
	DetourAttach((void**)&pGetSystemDefaultLangID, newGetSystemDefaultLangID);
	DetourAttach((void**)&pGetSystemDefaultUILanguage, newGetSystemDefaultUILanguage);
	DetourAttach((void**)&pGetUserDefaultLCID, newGetUserDefaultLCID);
	DetourAttach((void**)&pSetWindowTextA, newSetWindowTextA);
	if(DetourTransactionCommit()!=NOERROR){ MessageBox(NULL, L"ERROR", L"Crescendo", MB_OK); }
	//Check();
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

