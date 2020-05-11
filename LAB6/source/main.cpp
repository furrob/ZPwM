#include <Windows.h>
#include "res.h"

//networking libs
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define SERVER_PORT 3000

#define LINE_LENGTH 1024 //max length of transmitted message
#define LINES_COUNT 16   //lines of messages saved
#define MARGIN_X 10
#define MARGIN_Y 35
#define BACKGROUND_COLOR 0

WCHAR awsMessageList[LINES_COUNT][LINE_LENGTH];
INT iMessageIndex = 0;


SOCKET serverSocket;
BOOL bNotExit = TRUE;






INT_PTR CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

VOID RefreshWindow(HWND hWnd);
VOID TransmitMessage(HWND hWmd);
VOID DisplayMessageList(HDC hDC);
VOID CreateServer();
VOID ReceiveMessage();
VOID CreateClient();

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR szCmdLine, _In_ int iCmdShow)
{
  HWND hMainWindow = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_MAINVIEW), NULL, WindowProc);

  ShowWindow(hMainWindow, iCmdShow);

  WSADATA wsaData;
  INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if(iResult != NO_ERROR)
  {
    MessageBoxW(0, L"Init error", L"WinSock", MB_OK);
    return 1;
  }

  MSG msg = {};
  while(GetMessageW(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return 0;
}

INT_PTR CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static RECT rClientRect = {};
  //static HBITMAP hBitmap = 0;

  switch(uMsg)
  {
    case WM_INITDIALOG:
    {
      GetClientRect(hWnd, &rClientRect);
      RefreshWindow(hWnd);



      return TRUE;
    }
    case WM_COMMAND:
    {
      switch(HIWORD(wParam)) //co
      {
        case BN_CLICKED:
          switch(LOWORD(LOWORD(wParam))) //komu
          {
            case IDC_BUTTON_SEND:
            {
              TransmitMessage(hWnd);
              return TRUE;
            }
            case IDC_BUTTON_SERVER:
            {
              CreateServer();
              return TRUE;
            }
          }
      }
      return TRUE;
    }
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hDC = BeginPaint(hWnd, &ps);
      //HDC hDC = GetDC(hWnd);
      
      HDC hDCBuff = CreateCompatibleDC(hDC);
      INT iWidth = rClientRect.right - 2 * MARGIN_X;
      INT iHeight = rClientRect.bottom - MARGIN_Y;
      HBITMAP hBitmap = CreateCompatibleBitmap(hDCBuff, iWidth, iHeight);
      SelectObject(hDCBuff, hBitmap);
      
      //background
      BitBlt(hDCBuff, 0, 0, iWidth, iHeight, hDCBuff, 0, 0, BLACKNESS);
      DisplayMessageList(hDCBuff);

      BitBlt(hDC, MARGIN_X, MARGIN_Y, iWidth, iHeight, hDCBuff, 0, MARGIN_Y, SRCCOPY);

      DeleteObject(hBitmap);
      DeleteDC(hDCBuff);

      EndPaint(hWnd, &ps);
      //ReleaseDC(hWnd, hDC); //nie wiem

      return TRUE;
    }
    case WM_CLOSE:
    {
      closesocket(serverSocket);
      WSACleanup();

      DestroyWindow(hWnd);
      PostQuitMessage(0);

      return TRUE;
    }
  }
  return FALSE;//DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void RefreshWindow(HWND hWnd)
{
  RECT rRect = {};
  GetWindowRect(hWnd, &rRect);
  InvalidateRect(hWnd, &rRect, FALSE);
  RedrawWindow(hWnd, &rRect, NULL, RDW_ERASE);
}

void TransmitMessage(HWND hWnd)
{
  iMessageIndex = ((iMessageIndex - 1) < 0) ? LINES_COUNT - 1 : iMessageIndex - 1;

  GetWindowTextW(GetDlgItem(hWnd, IDC_EDIT_MESSAGE), awsMessageList[iMessageIndex], LINE_LENGTH);
  
  RefreshWindow(hWnd);
}  
  

void DisplayMessageList(HDC hDC)
{
  SelectObject(hDC, GetStockObject(ANSI_VAR_FONT));
  SetTextColor(hDC, RGB(10, 115, 10));
  SetBkColor(hDC, BACKGROUND_COLOR);

  for(INT i = 0; i < LINES_COUNT; ++i)
  {
    INT iTemp = (iMessageIndex + i) % LINES_COUNT;
    TextOutW(hDC, MARGIN_X, 5 + MARGIN_Y + i * 15, awsMessageList[iTemp], lstrlenW(awsMessageList[iTemp]));
  }
  
}

void CreateServer()
{
  if((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
  {
    MessageBoxW(0, L"Cannot create Socket", L"WinSock Error", MB_OK);
    INT iError = WSAGetLastError();
  }

  sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(SERVER_PORT);

  if(bind(serverSocket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
  {
    MessageBoxW(0, L"Cannot bind Socket to chosen address", L"WinSock Error", MB_OK);
    return;
  }

  ReceiveMessage();
}

void ReceiveMessage()
{
  sockaddr_in si_other;
  INT slen, recv_len;
  WCHAR buff[LINE_LENGTH];

  while(bNotExit)
  {
    memset(buff, 0, LINE_LENGTH);

    if((recv_len = recvfrom(serverSocket, (CHAR*)buff, 2 * LINE_LENGTH, 0, (sockaddr*)&si_other, &slen)) == SOCKET_ERROR) //not sure if char->wchar done right
    {
      MessageBoxW(0, L"Data reception failed", L"WinSock Error", MB_OK);
      return;
    }
    MessageBoxW(0, L"Data reception failed", L"WinSock Error", MB_OK);
  }
}

void CreateClient()
{
  //if((clientSoc))
}
