#include <windows.h>
#include "resource.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <io.h>
#include "SkyeTekAPI.h"
#include "SkyeTekProtocol.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

OVERLAPPED osWrite;
BOOL initialized;

HANDLE hComm;
HANDLE hThread;
BOOL Status;
DCB dcb = { 0 };
COMMCONFIG	cc;
HANDLE monitor = GetStdHandle(STD_OUTPUT_HANDLE);
LPSKYETEK_TAG *lpTags;

unsigned short tagCount;
unsigned int numDevices = NULL;
unsigned int numReaders = NULL;
static unsigned yPos = 0;

bool start;
HWND hwnd;

SKYETEK_STATUS ReadTagData(LPSKYETEK_READER lpReader, LPSKYETEK_TAG lpTag);

DWORD WINAPI ListenThread(HWND hwnd)
{
	return 0;
}


int ReadTag(LPSKYETEK_READER *readers) {
	int i = 0;
	HDC hdc = GetDC(hwnd);

	while (1) {
		if (start) {
			lpTags = NULL;
			tagCount = 0;
			SKYETEK_STATUS st = SkyeTek_GetTags(readers[0], AUTO_DETECT, &lpTags, &tagCount);
			if (st != SKYETEK_SUCCESS) {
				continue;
			}
			if (st == SKYETEK_TIMEOUT) {
				continue;
			}
			if (lpTags == NULL) {
				continue;
			}

			TextOut(hdc, 0, yPos * 20, (*lpTags)->friendly, 100);
			yPos++;
			SkyeTek_FreeTags(readers[0], lpTags, tagCount);
			i++;
		}
	}

	return 0;
}

// Discovers devices as well as the reader
int Discover(HWND hwnd) {
	LPSKYETEK_DEVICE *devices = NULL;
	LPSKYETEK_READER *readers = NULL;
	DWORD threadID;

	numDevices = SkyeTek_DiscoverDevices(&devices);
	numReaders = SkyeTek_DiscoverReaders(devices, numDevices, &readers);

	if (numReaders != 0) {
		ReadTag(readers);
		//CreateThread(NULL, 0, ListenThread, 0, 0, &threadID);
	}
	
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_EXIT:
			PostQuitMessage(0);
			break;
		case ID_START:
			start = true;
			Discover(hwnd);
			break;
		case ID_STOP:
			start = false;
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

int WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspzCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = 0;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc; // window function
	Wcl.hInstance = hInst; // handle to this instance
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = "term"; // window class name

	Wcl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); // no class menu 
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;
	RegisterClassEx(&Wcl);

	ShowWindow(hwnd, nCmdShow);

	hwnd = CreateWindow(
		"term",
		"term",
		WS_SYSMENU,
		0, // X coord
		0, // Y coord DONT MAKE THIS 0, INVIS ON DUAL MONITOR
		1900, // width
		1000, // height
		NULL,
		NULL,
		hInst,
		NULL
		);

	ShowWindow(hwnd, nCmdShow);

	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg); // translate keybpard messages
		DispatchMessage(&Msg); // dispatch message and return control to windows
	}

	return Msg.wParam;
}