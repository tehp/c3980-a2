#include <windows.h>
#include "resource.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <conio.h>
#include <io.h>
#include "SkyeTekAPI.h"
#include "SkyeTekProtocol.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //The window process declaration

LPSKYETEK_DEVICE *devices = NULL; //pointer to device
LPSKYETEK_READER *readers = NULL; //pointer to reader

static unsigned yPos = 0; //keeps track of the y position on the window

bool readTag; //This sets the reader to read or not to read

HWND hwnd; //The windows handle

DWORD thread; //The listening thread

/*------------------------------------------------------------------------------------------------------------------
 -- FUNCTION: TagFoundCallback
 --
 -- DATE: Oct 16, 2017
 --
 -- REVISIONS: N/A
 --
 -- DESIGNER: Mackenzie Craig, Angus Lam
 --
 -- PROGRAMMER: Mackenzie Craig, Angus Lam
 --
 -- INTERFACE: unsigned char TagFoundCallback(LPSKYETEK_TAG lpTag, void *user)
 -- LPSKYETEK_TAG lpTag: A tag that was found.
 --
 -- RETURNS: 0
 --
 -- NOTES:
 -- This function is a callback that is called by the SkyeTek_SelectTags API call in the ListenThread. This function prints the names of the tags that are recognized in the previously mentioned functions.
 ----------------------------------------------------------------------------------------------------------------------*/
unsigned char TagFoundCallback(LPSKYETEK_TAG lpTag, void *user) {
	HDC hdc = GetDC(hwnd);

	if (lpTag == NULL) {
		TextOut(hdc, 0, yPos * 20, "                                                                                                   ", 100);

		return 0;
	}

	TextOut(hdc, 0, yPos * 20, lpTag->friendly, 100);
	SkyeTek_FreeTag(lpTag);
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
 -- FUNCTION: ListenThread
 --
 -- DATE: Oct 16, 2017
 --
 -- REVISIONS: N/A
 --
 -- DESIGNER: Mackenzie Craig, Angus Lam
 --
 -- PROGRAMMER: Mackenzie Craig, Angus Lam
 --
 -- INTERFACE: DWORD WINAPI ListenThread(LPVOID n)
 --
 -- RETURNS: 0
 --
 -- NOTES:
 -- A thread that listens for tags. When found, it calls the callback function TagFoundCallback, which handles the printing of the tag name.
 ----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI ListenThread(LPVOID n)
{
	LPSKYETEK_TAG  *lpTags = NULL;
	SKYETEK_STATUS  st;
	HDC             hdc = GetDC(hwnd);

	while (1) {
		if (readTag)
		{
			st = SkyeTek_SelectTags(readers[0], AUTO_DETECT, TagFoundCallback, 0, 1, NULL);
			if (st != SKYETEK_SUCCESS)
			{
				continue;
			}
			if (st == SKYETEK_TIMEOUT)
			{
				continue;
			}
			if (lpTags == NULL)
			{
				continue;
			}
		}
	}

	return 0;

}

/*------------------------------------------------------------------------------------------------------------------
 -- FUNCTION: Discover
 --
 -- DATE: Oct 16, 2017
 --
 -- REVISIONS: N/A
 --
 -- DESIGNER: Mackenzie Craig, Angus Lam
 --
 -- PROGRAMMER: Mackenzie Craig, Angus Lam
 --
 -- INTERFACE: int Discover(HWND hwnd)
 -- HWND hwnd: The window handle.
 --
 -- RETURNS: 0
 --
 -- NOTES:
 -- This function discovers all devices using SkyeTek_DiscoverDevices, and then discovers readers using SkyeTek_DiscoverReaders. Once a reader is discovered, creates a thread.
 ----------------------------------------------------------------------------------------------------------------------*/
int Discover(HWND hwnd) {
	unsigned int numDevices = NULL;
	unsigned int numReaders = NULL;

	HDC hdc = GetDC(hwnd);

	TextOut(hdc, 0, 0, "Discovering reader...", 22);

	numDevices = SkyeTek_DiscoverDevices(&devices);
	numReaders = SkyeTek_DiscoverReaders(devices, numDevices, &readers);

	if (numReaders != 0) {
		TextOut(hdc, 0, 0, "readers found :)             ", 30);

		CreateThread(NULL, 0, &ListenThread, 0, 0, &thread);
    } else {
        TextOut(hdc, 0, 0, "err, no reader               ", 30);
    }
	
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
 -- FUNCTION: WndProc
 --
 -- DATE: Oct 16, 2017
 --
 -- REVISIONS: N/A
 --
 -- DESIGNER: Mackenzie Craig, Angus Lam
 --
 -- PROGRAMMER: Mackenzie Craig, Angus Lam
 --
 -- INTERFACE: LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
 -- HWND hwnd: The window handle.
 -- UINT Message: Default message.
 -- WPARAM wParam
 -- LPARAM lParam
 --
 -- RETURNS: 0
 --
 -- NOTES:
 -- This function contains the switch statement that handles starting, stopping, and exiting the program.
 -- Start begins discovery of devices and readers, and sets start to true. Stop sets start to false.
 ----------------------------------------------------------------------------------------------------------------------*/
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
			readTag = true;
			if (thread == NULL)
			{
				Discover(hwnd);
			}
			break;
		case ID_STOP:
			readTag = false;
			HDC hdc = GetDC(hwnd);
			Sleep(50);
			TextOut(hdc, 0, yPos * 20, "                                                                                                   ", 100);

			break;
		}
		break;
	case WM_DESTROY:
		SkyeTek_FreeReader(*readers);
		SkyeTek_FreeDevice(*devices);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
 -- FUNCTION: WinMain
 --
 -- DATE: Oct 16, 2017
 --
 -- REVISIONS: N/A
 --
 -- DESIGNER: Mackenzie Craig, Angus Lam
 --
 -- PROGRAMMER: Mackenzie Craig, Angus Lam
 --
 -- INTERFACE: WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspzCmdParam, int nCmdShow)
 --
 -- RETURNS: Msg.wParam
 --
 -- NOTES:
 -- Main method. Sets the window configuration properties and createsthe window.
 ----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspzCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;

	Wcl.cbSize  = sizeof(WNDCLASSEX);
	Wcl.style   = 0;
	Wcl.hIcon   = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc   = WndProc; // window function
	Wcl.hInstance     = hInst; // handle to this instance
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = "term"; // window class name

	Wcl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); // no class menu 
	Wcl.cbClsExtra   = 0;      // no extra memory needed
	Wcl.cbWndExtra   = 0;
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
