#include <Windows.h>
#include "res.h"

#include <string>

HINSTANCE ghInstance;

INT_PTR CALLBACK WindowProcKA(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK WindowProcKO(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR szCmdLine, _In_ int iCmdShow)
{
  ghInstance = hInstance;

  wchar_t szText[64];

  //Kajko
  HWND hWindowKA = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_MAINVIEW), NULL, WindowProcKA);
  wsprintfW(szText, L"Kajko");
  SetWindowTextW(hWindowKA, szText);

  

  //Kokosz
  HWND hWindowKO = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_MAINVIEW), NULL, WindowProcKO);
  wsprintfW(szText, L"Kokosz");
  SetWindowTextW(hWindowKO, szText);


  ShowWindow(hWindowKA, iCmdShow);
  ShowWindow(hWindowKO, iCmdShow);

  MSG msg = {};

  while (GetMessageW(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return 0;
}

INT_PTR CALLBACK WindowProcKA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_CLOSE:  //zamykanie
    DestroyWindow(hWnd);
    PostQuitMessage(0);
    return TRUE;
  case WM_INITDIALOG:
    HICON hIcon = LoadIconW(ghInstance, MAKEINTRESOURCE(IDI_ICON_KA));
    SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    return TRUE;
  }

  return 0;
  //return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK WindowProcKO(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_CLOSE:  //zamykanie
    MessageBoxW(NULL, L"kokosz", L"kokosz", MB_OK);
    DestroyWindow(hWnd);
    PostQuitMessage(0);
    return TRUE;
  case WM_INITDIALOG:
    HICON hIcon = LoadIconW(ghInstance, MAKEINTRESOURCE(IDI_ICON_KO));
    SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    return TRUE;
  }

  return 0;
  //return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}