#include <Windows.h>
#include <windowsx.h> //GET_*_LPARAM
#include "res.h"

#define ZOOM_FACTOR 4

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

  static BOOL bStretch = FALSE;

  static BOOL bZoomed = FALSE;
  static RECT rSelection = {};

  WCHAR szText[512];

  switch (uMsg)
  {
    case WM_INITDIALOG:
    {
      //Load menu
      HMENU hMenu = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDR_MAINMENU));
      SetMenu(hWnd, hMenu);

      //Path relative to working dir when launched from VS
      hBitmap = (HBITMAP)LoadImageW(GetModuleHandleW(NULL), L"./../source/bitmaps/test.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

      if(hBitmap == NULL)
      {
        DWORD lastError = GetLastError(); //just for debugger
        PostQuitMessage(0);
        return FALSE;
      }
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

      if(bZoomed) //zoomed (constant zoom xZOOM_FACTOR - window will be resized to fit selected area)
      {
        RECT rTemp = rSelection;
        
        rTemp.right = (rSelection.right - rSelection.left) * ZOOM_FACTOR;
        rTemp.bottom = (rSelection.bottom - rSelection.top) * ZOOM_FACTOR;
        rTemp.left = 0;
        rTemp.top = 0;

        ResizeClientRect(hWnd, rTemp);
        
        GetClientRect(hWnd, &rTemp);


        StretchBlt(hDC, 0, 0, rTemp.right, rTemp.bottom, hDCbitmap,
          rSelection.left, rSelection.top,
          rSelection.right - rSelection.left, rSelection.bottom - rSelection.top, SRCCOPY);
      }
      else if(!bStretch) //fit
      {
        BitBlt(hDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, hDCbitmap, 0, 0, SRCCOPY);
      }
      else  //stretch
      {
        RECT rWndRect;
        GetClientRect(hWnd, &rWndRect);
        StretchBlt(hDC, 0, 0, rWndRect.right, rWndRect.bottom, hDCbitmap, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);
      }




      DeleteDC(hDCbitmap);

      ReleaseDC(hWnd, hDC);

      return TRUE;
    }
    case WM_LBUTTONDOWN: //first point
    {
      if(!bZoomed && !bStretch)
      {
        rSelection.left = GET_X_LPARAM(lParam);
        rSelection.top = GET_Y_LPARAM(lParam);
      }
      return TRUE;
    }
    case WM_LBUTTONUP:  //second point
    {
      if(!bZoomed && !bStretch)
      {
        rSelection.right = GET_X_LPARAM(lParam);
        rSelection.bottom = GET_Y_LPARAM(lParam);

        RECT rTemp;

        //hmmmmmmmmmmmmm
        rTemp.top = (rSelection.top <= rSelection.bottom) ? rSelection.top : rSelection.bottom;
        rTemp.bottom = (rSelection.bottom >= rSelection.top) ? rSelection.bottom : rSelection.top;
        rTemp.left = (rSelection.left <= rSelection.right) ? rSelection.left : rSelection.right;
        rTemp.right = (rSelection.right >= rSelection.left) ? rSelection.right : rSelection.left;

        rSelection = rTemp;

        bZoomed = TRUE;
        bStretch = FALSE;

        
        GetWindowRect(hWnd, &rTemp);
        InvalidateRect(hWnd, &rTemp, FALSE); //force repaint
      }
      return TRUE;
    }
    case WM_RBUTTONDOWN: //reset zoom (can only zoom once)
    {
      SendMessageW(hWnd, WM_COMMAND, BN_CLICKED << 16 | ID_WIDOK_DOPASUJ, NULL);

      return TRUE;
    }
    case WM_COMMAND:
    {
      switch(HIWORD(wParam)) //whats happened
      {
        case BN_CLICKED: //click
          switch(LOWORD(wParam)) //on what
          {
            case ID_WIDOK_DOPASUJ:
            {
              bZoomed = FALSE;
              rSelection = { 0 , 0, 0, 0};

              bStretch = FALSE;
              ResizeClientRect(hWnd, rBitmap);
              return 0;
            }
            case ID_WIDOK_SKALUJ:
            {
              bStretch = TRUE;
              //ResizeClientRect(hWnd, rBitmap);
              return 0;
            }
            default:
              break;
          }
        default:
          break;
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