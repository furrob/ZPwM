#include <Windows.h>
#include <string>
#include "res.h"

//networking libs
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define SERVER_ADDR "192.168.1.2"
#define SERVER_PORT 3000

#define LINE_LENGTH 1024 //max length of transmitted message
#define LINES_COUNT 16   //lines of messages saved
#define MARGIN_X 10
#define MARGIN_Y 35
#define BACKGROUND_COLOR 0
//#define PAINT_BP //BeginPaint(), if absent use GetDC()

struct sData
{
  HWND hWnd;
  SOCKET socket;
  BOOL server;
};


WCHAR awsMessageList[LINES_COUNT][LINE_LENGTH];
INT iMessageIndex = 0;

SOCKET serverSocket, clientSocket;
BOOL bNotExit = TRUE;

sockaddr_in asClients[100];
INT iConnectedClients = 0;

INT_PTR CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

VOID RefreshWindow(HWND hWnd);
VOID TransmitMessage(HWND hWmd);
VOID DisplayMessageList(HDC hDC);
VOID CreateServer(HWND hWnd);
VOID ReceiveMessage(sData* data);
VOID CreateClient(HWND hWnd);

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
      //RefreshWindow(hWnd);



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
              CreateServer(hWnd);
              return TRUE;
            }
            case IDC_BUTTON_CLIENT:
            {
              CreateClient(hWnd);
              return TRUE;
            }
          }
      }
      return TRUE;
    }
    case WM_PAINT:
    {
#ifdef PAINT_BP
      PAINTSTRUCT ps;
      HDC hDC = BeginPaint(hWnd, &ps);
      
      INT iWidth = rClientRect.right - 2 * MARGIN_X;
      INT iHeight = rClientRect.bottom - MARGIN_Y;

      //background
      BitBlt(hDC, MARGIN_X, MARGIN_Y, iWidth - MARGIN_X, iHeight - MARGIN_Y, hDC, 0, 0, BLACKNESS);
      DisplayMessageList(hDC);


      EndPaint(hWnd, &ps);
#else
      HDC hDC = GetDC(hWnd);
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

      ReleaseDC(hWnd, hDC);
#endif
      return TRUE;
    }
    case WM_CLOSE:
    {
      closesocket(serverSocket);
      closesocket(clientSocket);
      WSACleanup();

      DestroyWindow(hWnd);
      PostQuitMessage(0);

      bNotExit = FALSE;

      return TRUE;
    }
  }
  return FALSE;
}

void RefreshWindow(HWND hWnd)
{
  RECT rRect = {};
  GetWindowRect(hWnd, &rRect);
  InvalidateRect(hWnd, &rRect, FALSE);
  RedrawWindow(hWnd, &rRect, NULL, RDW_ERASE);

  //SendMessageW(hWnd, WM_PAINT, NULL, NULL);
}

void TransmitMessage(HWND hWnd)
{
  WCHAR wsMessage[LINE_LENGTH];

  GetWindowTextW(GetDlgItem(hWnd, IDC_EDIT_MESSAGE), wsMessage, LINE_LENGTH); //TODO edit this so after adding sender data this wouldn't exceed line length

  //RefreshWindow(hWnd);
  /*
  CONST INT iPrevMessageIndex = iMessageIndex;
  iMessageIndex = ((iMessageIndex - 1) < 0) ? LINES_COUNT - 1 : iMessageIndex - 1;

  GetWindowTextW(GetDlgItem(hWnd, IDC_EDIT_MESSAGE), awsMessageList[iMessageIndex], LINE_LENGTH);
  
  RefreshWindow(hWnd);
  */

  //prepare server information
  sockaddr_in si_other{};
  memset(&si_other, 0, sizeof(sockaddr_in));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(SERVER_PORT);
  si_other.sin_addr.S_un.S_addr = inet_addr(SERVER_ADDR);

  INT iSendResult = sendto(clientSocket,
    reinterpret_cast<const char*>(wsMessage), 2 * lstrlenW(wsMessage), 0,
    reinterpret_cast<sockaddr*>(&si_other), sizeof(sockaddr));

  if(iSendResult == SOCKET_ERROR)
  {
    INT iError = WSAGetLastError();
    WCHAR wsBuffer[512];
    wsprintfW(wsBuffer, L"Cannot send data\nError code: %i", iError);
    MessageBoxW(0, wsBuffer, L"WinSock Error", MB_OK | MB_ICONERROR);
  }
}  
  

void DisplayMessageList(HDC hDC)
{
  INT iMessageIndex_ = iMessageIndex; //multiple threads and stuff

  SelectObject(hDC, GetStockObject(ANSI_VAR_FONT));
  SetTextColor(hDC, RGB(20, 255, 20));
  SetBkColor(hDC, BACKGROUND_COLOR);

  for(INT i = 0; i < LINES_COUNT; ++i)
  {
    INT iTemp = (iMessageIndex_ + i) % LINES_COUNT;
    TextOutW(hDC, MARGIN_X, 5 + MARGIN_Y + i * 15, awsMessageList[iTemp], lstrlenW(awsMessageList[iTemp]));
  }
  
}

void CreateServer(HWND hWnd)
{
  if((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
  {
    INT iError = WSAGetLastError();
    WCHAR wsBuffer[512];
    wsprintfW(wsBuffer, L"Cannot create Socket\nError code: %i", iError);
    MessageBoxW(0, wsBuffer, L"WinSock Error", MB_OK | MB_ICONERROR);
  }

  sockaddr_in server{};
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(SERVER_PORT);

  INT iBindResult = bind(serverSocket, reinterpret_cast<sockaddr*>(&server), sizeof(sockaddr_in));

  if(iBindResult == SOCKET_ERROR)
  {
    INT iError = WSAGetLastError();
    WCHAR wsBuffer[512];
    wsprintfW(wsBuffer, L"Cannot bind Socket to chosen address\nError code: %i", iError);
    MessageBoxW(0, wsBuffer, L"WinSock Error", MB_OK | MB_ICONERROR);
    return;
  }

  //set up listening for incomming messages

  auto data = new sData;
  data->hWnd = hWnd;
  data->socket = serverSocket;
  data->server = TRUE;

  DWORD dwThreadId;
  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ReceiveMessage, data, 0, &dwThreadId);
}

void ReceiveMessage(sData* data) //message formatting by server (from who etc.)
{
  sockaddr_in si_other;
  INT slen = sizeof(sockaddr);// need to set that to proper value
  INT recv_len; 
  WCHAR wsInBuff[LINE_LENGTH];

  //CHAR test[20];
  //INT itest = 20;

  while(bNotExit)
  {
    memset(wsInBuff, '\0', LINE_LENGTH);

    //not sure if char->wchar done right
    recv_len = recvfrom(data->socket, (CHAR*)wsInBuff, 2 * LINE_LENGTH, 0, (sockaddr*)&si_other, &slen);
    //recv_len = recvfrom(data->socket, test, itest, 0, (sockaddr*)&si_other, &slen);

    if(recv_len > 0) //recieved something (?)
    {
      if(data->server == TRUE)
      {
        WCHAR wsBuff[LINE_LENGTH];

        wsprintfW(wsBuff, L"%S : %s", inet_ntoa(si_other.sin_addr), wsInBuff); //message to pass further

        BOOL bFound = FALSE;

        for(INT i = 0; i < iConnectedClients; ++i)
        {
          if(asClients[i].sin_addr.S_un.S_addr == si_other.sin_addr.S_un.S_addr)
          {
            bFound = TRUE;
            break;
          }
        }

        if(!bFound)
          memcpy(&asClients[iConnectedClients++], &si_other, sizeof(si_other));

        for(int i = 0; i < iConnectedClients; ++i)
        {
          INT iSendResult = sendto(data->socket, reinterpret_cast<char*>(wsBuff), 2 * LINE_LENGTH, 0, reinterpret_cast<sockaddr*>(&asClients[i]), slen);
          if(iSendResult == SOCKET_ERROR)
          {
            INT iError = WSAGetLastError();
            WCHAR wsBuffer[512];
            wsprintfW(wsBuffer, L"Cannot send data\nError code: %i", iError);
            MessageBoxW(0, wsBuffer, L"WinSock Error", MB_OK | MB_ICONERROR);
            //break;
          }
        }
      }
      else
      {
        iMessageIndex = ((iMessageIndex - 1) < 0) ? LINES_COUNT - 1 : iMessageIndex - 1;
        memcpy(awsMessageList[iMessageIndex], wsInBuff, LINE_LENGTH);

        RefreshWindow(data->hWnd);
      }
      
    }
    else if(recv_len == SOCKET_ERROR) 
    {
      INT iError = WSAGetLastError();
      WCHAR wsBuffer[512];
      wsprintfW(wsBuffer, L"Data reception failed\nError code: %i", iError);
      MessageBoxW(0, wsBuffer, L"WinSock Error", MB_OK | MB_ICONERROR);
      return;
    }
  }
}

void CreateClient(HWND hWnd)
{
  clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(clientSocket == INVALID_SOCKET)
  {
    INT iError = WSAGetLastError();
    WCHAR wsBuffer[512];
    wsprintfW(wsBuffer, L"Cannot create Socket\nError code: %i", iError);
    MessageBoxW(0, wsBuffer, L"WinSock Error", MB_OK | MB_ICONERROR);
  }

  //i m p o r t a n t
  TransmitMessage(hWnd);

  auto data = new sData;
  data->hWnd = hWnd;
  data->socket = clientSocket;
  data->server = FALSE;

  DWORD dwThreadId;
  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ReceiveMessage, data, 0, &dwThreadId);
}
