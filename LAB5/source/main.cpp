#include <Windows.h>
#include "res.h"

INT_PTR CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

VOID ResizeClientRect(HWND, RECT);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR szCmdLine, _In_ int iCmdShow)
{
  HWND hMainWindow = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_MAINVIEW), NULL, WindowProc);

  ShowWindow(hMainWindow, iCmdShow);

  MSG msg = {};
  while (GetMessageW(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return 0;
}


INT_PTR CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static HBITMAP hBitmap = 0;
  static BITMAP bmInfo = {};
  static RECT rBitmap = {};

  static BOOL bFit = FALSE;

  switch (uMsg)
  {
  case WM_INITDIALOG:
    {
      //Load menu
      HMENU hMenu = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDR_MAINMENU));
      SetMenu(hWnd, hMenu);

      //Path relative to working dir when launched from VS
      hBitmap = (HBITMAP)LoadImageW(GetModuleHandleW(NULL), L"./../source/bubr.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

      if (hBitmap == NULL)
        DWORD lastError = GetLastError(); //just for debugger

      //get bitmap's info
      GetObjectW(hBitmap, sizeof(bmInfo), &bmInfo);

      // set window's initial client size to fit loaded bitmap
      rBitmap = { 0, 0, bmInfo.bmWidth, bmInfo.bmHeight };

      ResizeClientRect(hWnd, rBitmap);

      return TRUE;
    }
  case WM_PAINT:
    {
      HDC hDC; //handle to device context
      hDC = GetDC(hWnd); //getting handle to window's context

      HDC hDCbitmap = CreateCompatibleDC(hDC);
      SelectObject(hDCbitmap, hBitmap);

      if(bFit == FALSE)
        BitBlt(hDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, hDCbitmap, 0, 0, SRCCOPY);
      else
      {
        RECT rWndRect;
        GetClientRect(hWnd, &rWndRect);
        StretchBlt(hDC, 0, 0, rWndRect.right, rWndRect.bottom, hDCbitmap, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);
      }
        

      DeleteDC(hDCbitmap);

      ReleaseDC(hWnd, hDC);

      return TRUE;
    }
  case WM_COMMAND:
    {
      switch(HIWORD(wParam)) //whats happened
      {
      case BN_CLICKED:  //click
        switch(LOWORD(wParam)) //on what
        {
        case ID_WIDOK_DOPASUJ:
          {
            bFit = FALSE;
            ResizeClientRect(hWnd, rBitmap);
            return 0;
          }
        case ID_WIDOK_SKALUJ:
          {
            bFit = TRUE;
            ResizeClientRect(hWnd, rBitmap);
            return 0;
          }
        }
      }
    }
  case WM_CLOSE://zamykanie
    {
      DestroyWindow(hWnd);
      PostQuitMessage(0);

      DeleteObject(hBitmap);


      return TRUE;
    }
  }

  return 0;//DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

//resizes window to fit specified rect in its client area
VOID ResizeClientRect(HWND hWnd, RECT Rect)
{
  AdjustWindowRect(&Rect, GetWindowLongW(hWnd, GWL_STYLE), TRUE); //"enlarges" r to compensate for window style or menu

  SetWindowPos(hWnd, 0, 
    Rect.left, Rect.top, 
    Rect.right - Rect.left, Rect.bottom - Rect.top,
    SWP_NOMOVE | SWP_NOZORDER);
}