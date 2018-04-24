#pragma once
#include<windows.h>
#include<windowsx.h>
#define InitPositionX 0
#define InitPositionY 0
#define WIDTH 800
#define HEIGHT 600
class WindowsMonitor {
public:
	static LPVOID m_pFrame;
	static HDC m_compatibleDC;
	static HDC m_screenDc;
	static HBITMAP m_screenOb;
	static HBITMAP m_screenHb;
	static HBITMAP m_compatibleBitmap;
	static WNDCLASS m_wc;
	static BITMAPINFO m_bi;
	static HWND m_hwnd;
	static BOOL InitWindow(HINSTANCE hInstance, int nCmdShow);
	static LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};