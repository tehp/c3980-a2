#include <windows.h>
#include "resource.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <conio.h>
#include <io.h>
#include "SkyeTekAPI.h"
#include "SkyeTekProtocol.h"
#include <chrono>


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

DWORD thread;
HANDLE handle;

LPSKYETEK_DEVICE *devices = NULL;
LPSKYETEK_READER *readers = NULL;

LPCSTR tags[10];

SKYETEK_STATUS ReadTagData(LPSKYETEK_READER lpReader, LPSKYETEK_TAG lpTag);

unsigned char TagFoundCallback(LPSKYETEK_TAG lpTag, void *user) {
	HDC hdc = GetDC(hwnd);

	if (lpTag == NULL) {
		TextOut(hdc, 0, yPos * 20, "                                                                                                   ", 100);

		return 0;
	}
	tags[0] = lpTag->friendly;

	TextOut(hdc, 0, yPos * 20, lpTag->friendly, 100);
	//yPos++;
	SkyeTek_FreeTag(lpTag);
	return 0;
}


DWORD WINAPI ListenThread(LPVOID n)
{
	HDC hdc = GetDC(hwnd);
	SKYETEK_STATUS st;

	/*int numTags = 0;

	while (1) {
		if (start) {
			yPos = 0;
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

			if (tagCount == 0) {
				TextOut(hdc, 0, yPos * 20, "nothing", 100);
			}
			else {
				TextOut(hdc, 0, yPos * 20, (*lpTags)->friendly, 100);
			}
			yPos++;
			SkyeTek_FreeTags(readers[0], lpTags, tagCount);
		}
	}

	return 0;*/

	// NEW ATTEMPT :)

	while (1) {
		st = SkyeTek_SelectTags(readers[0], AUTO_DETECT, TagFoundCallback, 0, 1, NULL);
		if (st != SKYETEK_SUCCESS) {
			continue;
		}
		if (st == SKYETEK_TIMEOUT) {
			continue;
		}
		if (lpTags == NULL) {
			continue;
		}
	}

	return 0;

}



// Discovers devices as well as the reader
int Discover(HWND hwnd) {

	DWORD threadID;
	HDC hdc = GetDC(hwnd);

	TextOut(hdc, 0, 0, "starting", 8);


	numDevices = SkyeTek_DiscoverDevices(&devices);
	numReaders = SkyeTek_DiscoverReaders(devices, numDevices, &readers);



	if (numReaders != 0) {
		TextOut(hdc, 0, 0, "readers found :)", 16);

		//ReadTag(readers);
		CreateThread(NULL, 0, &ListenThread, 0, 0, &thread);
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
			//handle = CreateThread(NULL, 0, &ListenThread, 0, 0, &thread);
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
		110, // X coord
		110, // Y coord DONT MAKE THIS 0, INVIS ON DUAL MONITOR
		500, // width
		300, // height
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