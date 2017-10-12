#include <windows.h>
#include "resource.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <io.h>
//#include "stdafk.h"
//#include "Platform.h"
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
static unsigned xPos = 0;

HWND hwnd;

SKYETEK_STATUS ReadTagData(LPSKYETEK_READER lpReader, LPSKYETEK_TAG lpTag);


//DWORD WINAPI ListenThread(LPVOID n)
//{
//	COMMTIMEOUTS timeouts;
//	int endtime;
//	OVERLAPPED osRead = { 0 };
//	char output[2] = { NULL, NULL };
//	BOOL listenDone = false;
//	char str[100];
//	HDC hdc;
//
//	osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//	DWORD dwRead;
//
//	while (!listenDone) {
//		if (ReadFile(hComm, output, 2, &dwRead, &osRead) == FALSE)
//		{
//			dwRead = 0;
//			if ((GetLastError()) == (ERROR_IO_PENDING) && dwRead == 0)
//			{
//				if (GetOverlappedResult(hComm, &osRead, &dwRead, TRUE))
//				{
//					if (dwRead)
//					{
//						output[dwRead] = '/0';
//						hdc = GetDC(hwnd);
//						sprintf_s(str, "%c", (char)output[0]);
//						TextOut(hdc, 10 * xPos, 0, str, strlen(str));
//						xPos++;
//						ReleaseDC(hwnd, hdc);
//						OutputDebugString(str);
//
//					}
//				}
//				else
//				{
//					endtime = GetTickCount() + 250; // time in ms += 250ms
//				}
//				while (!GetOverlappedResult(hComm, &osRead, &dwRead, FALSE)) {
//					if (GetTickCount() > endtime) {
//						break;
//					}
//				}
//
//			}
//			else {
//				MessageBox(hwnd, "test", "", 1);
//			}
//		}
//		else {
//			// gets here if reading works on the first shot
//			if (dwRead) {
//				{
//					output[dwRead] = '/0';
//					hdc = GetDC(hwnd);
//
//					sprintf_s(str, "%c", (char)output[0]); //wParam
//					TextOut(hdc, 10 * xPos, 0, str, strlen(str));
//					xPos++; //how far from left
//					ReleaseDC(hwnd, hdc);
//					OutputDebugString(str);
//				}
//			}
//		}
//
//		/* manually reset event */
//		ResetEvent(osRead.hEvent);
//
//	} /* end while (reading bytes) */
//	return 1;
//}

int ReadTagData(LPSKYETEK_READER *readers) {
    LPSKYETEK_DATA lpData = NULL;
    SKYETEK_ADDRESS addr;
    SKYETEK_MEMORY mem;
    unsigned long length;
    unsigned char data[2048];
    bool didFail = false;
    unsigned char stat = 0;
    
    HDC hdc;
    hdc = GetDC(hwnd);
    
    // Allocate memory and assign Tag Info to mem
    memset(&mem, 0, sizeof(SKYETEK_MEMORY));
    SkyeTek_GetTagInfo(readers[0], *lpTags, &mem);
    
    // Allocate memory for data
    memset(data, 0, 2048);
    length = (mem.maxBlock - mem.startBlock + 1) * mem.bytesPerBlock;
    
    // Assign address
    memset(&addr, 0, sizeof(SKYETEK_ADDRESS));
    addr.start = mem.startBlock;
    addr.blocks = 1;
    
    
    unsigned char *ptr = data;
    for (; addr.start <= mem.maxBlock; addr.start++)
    {
        // Read data
        SkyeTek_ReadTagData(readers[0], *lpTags, &addr, 0, 0, &lpData);
        
        
        // Get lock status
        stat = 0;
        SkyeTek_GetLockStatus(readers[0], *lpTags, &addr, &stat);
        TextOut(hdc, 10 * xPos, 0, (*lpTags)->friendly, 100);
        
        
        // Copy over data to buffer
        memcpy(ptr, lpData->data, lpData->size);
        ptr += mem.bytesPerBlock;
        SkyeTek_FreeData(lpData);
        lpData = NULL;
    }
    
    lpData = SkyeTek_AllocateData((int)length);
    SkyeTek_CopyBuffer(lpData, data, length);
    TCHAR *str = SkyeTek_GetStringFromData(lpData);
    //output(_T("Tag data for %s: %s\n"), lpTag->friendly, str);
    //TextOut(hdc, 10 * xPos, 0, str, strlen(str));
    //TextOut(hdc, 10 * xPos, 0, "asdf", 4);
    //xPos++; //how far from left
    ReleaseDC(hwnd, hdc);
    
    SkyeTek_FreeString(str);
    SkyeTek_FreeData(lpData);
    return 0;
}

int ReadTag(LPSKYETEK_READER *readers) {
    int i = 0;
    
    while (i < 1) {
        lpTags = NULL;
        tagCount = 0;
        SkyeTek_GetTags(readers[0], AUTO_DETECT, &lpTags, &tagCount);
        ReadTagData(readers);
        SkyeTek_FreeTags(readers[0], lpTags, tagCount);
        i++;
    }
    
    return 0;
}


int Discover(HWND hwnd) {
    LPSKYETEK_DEVICE *devices = NULL;
    LPSKYETEK_READER *readers = NULL;
    
    numDevices = SkyeTek_DiscoverDevices(&devices);
    numReaders = SkyeTek_DiscoverReaders(devices, numDevices, &readers);
    
    ReadTag(readers);
    
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
                //START
                break;
            case ID_STOP:
                //STOP
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
                        300, // X coord
                        300, // Y coord DONT MAKE THIS 0, INVIS ON DUAL MONITOR
                        500, // width
                        300, // height
                        NULL,
                        NULL,
                        hInst,
                        NULL
                        );
    
    ShowWindow(hwnd, nCmdShow);
    
    UpdateWindow(hwnd);
    
    Discover(hwnd);
    
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg); // translate keybpard messages
        DispatchMessage(&Msg); // dispatch message and return control to windows
    }
    
    return Msg.wParam;
}
