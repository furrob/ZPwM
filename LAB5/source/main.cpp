#include <Windows.h>
#include <windowsx.h> //GET_*_LPARAM
#include "res.h"

/*
 *RMB - reset to default view
 *LMB click -> mouse move -> LMB release - "zooming"
 *hold CTRL to enable "magnifying glass"
 *  ^mouse scroll to zoom in or out
 */

#define ZOOM_MAX 10.f
#define ZOOM_MIN 2.f
#define ZOOM_STEP 0.2f
#define ZOOM_AREA_HALF_SIDE 20 //"preview" -> square with side length of ZOOM_AREA_HALF_SIDE * 2 * ZOOM_FACTOR

#define SELECION_SIZE_MIN 10 //minimal selection size to trigger "enlargation"

enum class ViewState
{NORMAL, STRETCH, ENLARGED};

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
  static HICON hIcon = 0;
  static BOOL bIcon = TRUE;

  static ViewState eView = ViewState::NORMAL; //to replace all this confusing bool values

  static RECT rSelection = {};

  static BOOL bZoomed = FALSE;
  static POINT pMouse = {};

  static FLOAT fZoomScale = 4.f;
  static FLOAT fEnlargeFactor = 1.f; //substitute for ZOOM_FACTOR
  //WCHAR szText[512];

  switch (uMsg)
  {
    case WM_INITDIALOG: //INIT
    {
      //Load menu
      HMENU hMenu = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDR_MAINMENU));
      SetMenu(hWnd, hMenu);

      //Load icon
      hIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONMAIN));
      SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

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
      RECT rTemp = {}; //contains window size
      GetClientRect(hWnd, &rTemp); //actual size of window
      HDC hDCbuff = CreateCompatibleDC(hDC); 
      HBITMAP hBuff = CreateCompatibleBitmap(hDC, rTemp.right, rTemp.bottom); //create new, empty bitmap to draw on
      SelectObject(hDCbuff, hBuff); //select that bitmap in newly created "buffer context"

      // "drawing" depending on current state TODO: maybe add something to avoid doing all this things every single call (maybe not)
      switch(eView)
      {
        case ViewState::NORMAL: //means window size set to fit whole unstretched bitmap
        {
          BitBlt(hDCbuff, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, hDCbitmap, 0, 0, SRCCOPY); //to off-screen buffer
          break;
        }
        case ViewState::STRETCH: //displaying whole bitmap, but stretched to cover all visible window space
        {
          //rTemp contains window client rectangle
          StretchBlt(hDCbuff, 0, 0, rTemp.right, rTemp.bottom, hDCbitmap,
            0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY); //to off-screen buffer
          break;
        }
        case ViewState::ENLARGED: //enlarged, selected portion of bitmap displayed
        {
          rTemp.right = static_cast<LONG>((rSelection.right - rSelection.left) * fEnlargeFactor); 
          rTemp.bottom = static_cast<LONG>((rSelection.bottom - rSelection.top) * fEnlargeFactor);
          rTemp.left = 0;
          rTemp.top = 0;

          ResizeClientRect(hWnd, rTemp);

          StretchBlt(hDCbuff, 0, 0, rTemp.right, rTemp.bottom, hDCbitmap,
            rSelection.left, rSelection.top,
            rSelection.right - rSelection.left, rSelection.bottom - rSelection.top, SRCCOPY);

          break;
        }
      }

      //Icon drawing
      if(bIcon)
        DrawIcon(hDCbuff, 0, 0, hIcon);

      //zoom
      if(bZoomed) //"strange" effect when there is no source bitmap to zoom - transparency
      {
        INT iOffset = static_cast<INT>(ZOOM_AREA_HALF_SIDE * fZoomScale);

        //upper left corner of source
        POINT pSrcCorner = {pMouse.x - ZOOM_AREA_HALF_SIDE, pMouse.y - ZOOM_AREA_HALF_SIDE};

        //upper left corner of destination
        POINT pDstCorner = { pMouse.x - iOffset, pMouse.y - iOffset};

        //right-left check
        pDstCorner.x = (pDstCorner.x + iOffset * 2 < rTemp.right) ? pDstCorner.x : rTemp.right - iOffset * 2;
        pDstCorner.x = (pDstCorner.x > 0) ? pDstCorner.x : 0;
        //up-down check
        pDstCorner.y = (pDstCorner.y > 0) ? pDstCorner.y : 0;
        pDstCorner.y = (pDstCorner.y + iOffset * 2 < rTemp.bottom) ? pDstCorner.y : rTemp.bottom - iOffset * 2;

        StretchBlt(hDCbuff, pDstCorner.x, pDstCorner.y, iOffset * 2, iOffset * 2,
          hDCbuff, pSrcCorner.x, pSrcCorner.y, ZOOM_AREA_HALF_SIDE * 2, ZOOM_AREA_HALF_SIDE * 2, SRCCOPY);
        //^ small zoom and SRCPAINT, SRCINVERT
      }

      //copy from off screen buffer to app context
      BitBlt(hDC, 0, 0, rTemp.right, rTemp.bottom, hDCbuff, 0, 0, SRCCOPY);

      DeleteDC(hDCbitmap);
      DeleteDC(hDCbuff);

      DeleteObject(hBuff); //important af, absence of this line caused my first time when i used all of my 16GB of RAM

      ReleaseDC(hWnd, hDC);

      return TRUE;
    }
    case WM_KEYDOWN:
    {
      if(wParam == VK_CONTROL)
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
        fZoomScale = (fZoomScale < ZOOM_MAX - ZOOM_STEP) ? fZoomScale + ZOOM_STEP : ZOOM_MAX;
      }
      else //zoom out
      {
        fZoomScale = (fZoomScale > ZOOM_MIN + ZOOM_STEP) ? fZoomScale - ZOOM_STEP : ZOOM_MIN;
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
      if(eView == ViewState::NORMAL)//should work only when bitmap isn't scaled whatsoever
      {
        rSelection.left = GET_X_LPARAM(lParam);
        rSelection.top = GET_Y_LPARAM(lParam);
      }
      return TRUE;
    }
    case WM_LBUTTONUP:  //second point
    {
      if(eView == ViewState::NORMAL)
      {
        rSelection.right = GET_X_LPARAM(lParam);
        rSelection.bottom = GET_Y_LPARAM(lParam);

        RECT rTemp;

        //hmmmmmmmmmmmmm
        rTemp.top = (rSelection.top <= rSelection.bottom) ? rSelection.top : rSelection.bottom;
        rTemp.bottom = (rSelection.bottom >= rSelection.top) ? rSelection.bottom : rSelection.top;
        rTemp.left = (rSelection.left <= rSelection.right) ? rSelection.left : rSelection.right;
        rTemp.right = (rSelection.right >= rSelection.left) ? rSelection.right : rSelection.left;

        const INT iSelSizeX = rTemp.right - rTemp.left, iSelSizeY = rTemp.bottom - rTemp.top;

        if(iSelSizeX > SELECION_SIZE_MIN && iSelSizeY > SELECION_SIZE_MIN)
        {
          rSelection = rTemp;

          eView = ViewState::ENLARGED;

          //calculating "adaptive zoom" from selection
          FLOAT fScaleX = static_cast<FLOAT>(iSelSizeX) / bmInfo.bmWidth,
            fScaleY = static_cast<FLOAT>(iSelSizeY) / bmInfo.bmHeight;

          FLOAT fTemp = max(fScaleX, fScaleY);

          fEnlargeFactor = 1 / fTemp;

          GetWindowRect(hWnd, &rTemp);
          InvalidateRect(hWnd, &rTemp, FALSE); //force repaint
        }
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
              rSelection = { 0 , 0, 0, 0};

              eView = ViewState::NORMAL;
              
              ResizeClientRect(hWnd, rBitmap);
              return 0;
            }
            case ID_WIDOK_SKALUJ:
            {
              eView = ViewState::STRETCH;
              return 0;
            }
            case ID_WIDOK_IKONA:
            {
              bIcon = !bIcon;
              return TRUE;
            }
            default:
              break;
          }
        default:
          break;
      }
    }
    case WM_CLOSE:
    {
      DestroyWindow(hWnd);
      PostQuitMessage(0);

      DeleteObject(hBitmap);

      return TRUE;
    }
  default:
    break;
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