#include <Windows.h>
#include <windowsx.h> //GET_*_LPARAM
#include "res.h"

#define ZOOM_FACTOR 6
#define ZOOM_AREA_HALF_SIDE 20 //"preview" -> square with side length of ZOOM_AREA_HALF_SIDE * 2 * ZOOM_FACTOR

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

  static BOOL bEnlarged = FALSE;
  static RECT rSelection = {};

  static BOOL bZoomed = FALSE;
  static POINT pMouse = {};

  static FLOAT fZoomScale = 1.0f;
  //WCHAR szText[512];

  switch (uMsg)
  {
    case WM_INITDIALOG: //INIT
    {
      //Load menu
      HMENU hMenu = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDR_MAINMENU));
      SetMenu(hWnd, hMenu);

      //Path relative to working dir when launched from VS
      hBitmap = static_cast<HBITMAP>(LoadImageW(GetModuleHandleW(NULL), L"./../source/bitmaps/sruby.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));

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
    case WM_PAINT:  //PAINT
    {
      HDC hDC;
      hDC = GetDC(hWnd); //getting handle to window's context

      HDC hDCbitmap = CreateCompatibleDC(hDC); //creating new context
      SelectObject(hDCbitmap, hBitmap); //with previously loaded bitmap in it

      //off-screen buffer to prevent flicker when zooming
      RECT rTemp = {};
      GetWindowRect(hWnd, &rTemp); //actual size of window
      HDC hDCbuff = CreateCompatibleDC(hDC); 
      HBITMAP hBuff = CreateCompatibleBitmap(hDC, rTemp.right, rTemp.bottom); //create new, empty bitmap to draw on
      SelectObject(hDCbuff, hBuff); //select that bitmap in newly created "buffer context"

      if(bEnlarged) //enlarged (constant zoom xZOOM_FACTOR - window will be resized to fit selected area) TODO: organize things up, now its a disgusting mess
      {
        RECT rTemp = rSelection;
        
        rTemp.right = (rSelection.right - rSelection.left) * ZOOM_FACTOR;
        rTemp.bottom = (rSelection.bottom - rSelection.top) * ZOOM_FACTOR;
        rTemp.left = 0;
        rTemp.top = 0;
        
        ResizeClientRect(hWnd, rTemp);
        
        GetClientRect(hWnd, &rTemp);


        StretchBlt(hDCbuff, 0, 0, rTemp.right, rTemp.bottom, hDCbitmap,
          rSelection.left, rSelection.top,
          rSelection.right - rSelection.left, rSelection.bottom - rSelection.top, SRCCOPY);
      }
      else if(!bStretch) //fit
      {
        BitBlt(hDCbuff, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, hDCbitmap, 0, 0, SRCCOPY); //to off-screen buffer
      }
      else  //stretch
      {
        RECT rWndRect;
        GetClientRect(hWnd, &rWndRect);
        StretchBlt(hDCbuff, 0, 0, rWndRect.right, rWndRect.bottom, hDCbitmap,
          0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY); //to off-screen buffer
      }

      //zoom (magnifying glass type)
      if(bZoomed && !bEnlarged)
      {
        //upper left corner of source
        POINT pSrc = {pMouse.x - ZOOM_AREA_HALF_SIDE, pMouse.y - ZOOM_AREA_HALF_SIDE};

        //upper left corner of destination
        POINT pDst = { pMouse.x - ZOOM_AREA_HALF_SIDE * ZOOM_FACTOR * fZoomScale, pMouse.y - ZOOM_AREA_HALF_SIDE * ZOOM_FACTOR * fZoomScale};

        StretchBlt(hDCbuff, pDst.x, pDst.y,
          ZOOM_AREA_HALF_SIDE * 2 * ZOOM_FACTOR * fZoomScale, ZOOM_AREA_HALF_SIDE * 2 * ZOOM_FACTOR * fZoomScale, 
          hDCbuff, pSrc.x, pSrc.y, ZOOM_AREA_HALF_SIDE * 2, ZOOM_AREA_HALF_SIDE * 2, SRCCOPY);
      }

      //copy from off screen buffer to app context
      BitBlt(hDC, 0, 0, rTemp.right, rTemp.bottom, hDCbuff, 0, 0, SRCCOPY);

      DeleteDC(hDCbitmap);
      DeleteDC(hDCbuff);

      DeleteObject(hBuff);

      ReleaseDC(hWnd, hDC);

      return TRUE;
    }
    case WM_KEYDOWN:
    {
      if(wParam == VK_CONTROL && !bEnlarged)
        bZoomed = TRUE;
      
      return TRUE;
    }
    case WM_KEYUP:
    {
      if(wParam == VK_CONTROL)
        bZoomed = FALSE;

      return TRUE;
    }
    case WM_MOUSEWHEEL:
    {
      if(GET_WHEEL_DELTA_WPARAM(wParam) > 0) //zoom in
      {
        fZoomScale = (fZoomScale < 2.9f) ? fZoomScale + 0.1f : 3;
      }
      else //zoom out
      {
        fZoomScale = (fZoomScale > 0.6f) ? fZoomScale - 0.1f : 0.5f;
      }
      return TRUE;
    }
    case WM_MOUSEMOVE:
    {
      pMouse.x = GET_X_LPARAM(lParam);
      pMouse.y = GET_Y_LPARAM(lParam);

      return TRUE;
    }
    case WM_LBUTTONDOWN: //first point
    {
      if(!bEnlarged && !bStretch)
      {
        rSelection.left = GET_X_LPARAM(lParam);
        rSelection.top = GET_Y_LPARAM(lParam);
      }
      return TRUE;
    }
    case WM_LBUTTONUP:  //second point
    {
      if(!bEnlarged && !bStretch)
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

        bEnlarged = TRUE;
        bStretch = FALSE;

        
        GetWindowRect(hWnd, &rTemp);
        InvalidateRect(hWnd, &rTemp, FALSE); //force repaint
      }
      return TRUE;
    }
    case WM_RBUTTONDOWN: //reset to basic state (can only enlarge once)
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
              bEnlarged = FALSE;
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