#include <Windows.h>
#include <cstdlib> //rand
#include <time.h> //time
#include <string> //stoi

#include "res.h"

//#define DEBUG //do podpatrywania

//ZMIENNE GLOBALNE - nie mog³em znaleŸæ wiadomoœci wysy³anej przy pokazywaniu-"inicjalizacji" okienka
INT iNumber;
INT iGuessCount;

INT_PTR CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR szCmdLine, _In_ int iCmdShow)
{
  srand((UINT)time(NULL)); //zmienne ziarno
  iNumber = (rand() % 40) + 1;

  iGuessCount = 1; //nr próby

  HWND hMainWindow = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_MAINVIEW), NULL, WindowProc);

#ifdef DEBUG
  HWND hTip = GetDlgItem(hMainWindow, IDC_STATICINFO);
  WCHAR szText[10];
  wsprintfW(szText, L"%d", iNumber);
  SetWindowTextW(hTip, szText);
#endif

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
  switch (uMsg)
  {
  case WM_CLOSE:  //zamykanie
    DestroyWindow(hWnd);
    PostQuitMessage(0);
    return TRUE;
  case WM_COMMAND:
    //co sie stao
    switch (HIWORD(wParam))
    {
    case BN_CLICKED:
      //komu sie stao
      switch (LOWORD(wParam))
      {
      case IDC_BUTTONGUESS:
        HWND hEditGuess = GetDlgItem(hWnd, IDC_EDITGUESS);
        if (hEditGuess == NULL)
        {
          PostQuitMessage(0);
          return 0;
        }
        INT iTextLength = GetWindowTextLengthW(hEditGuess);
        WCHAR szText[100];
        GetWindowTextW(hEditGuess, szText, iTextLength + 1);

        INT iGuess = (INT)wcstol(szText, NULL, 10);
        if (iGuess == iNumber)
        {
          wsprintfW(szText, L"%d - trafione za %d razem.", iNumber, iGuessCount);
          MessageBoxW(hWnd, szText, L"Koniec", MB_OK);
          PostQuitMessage(0); //wyjœcie
          return 0;
        }
        else
        {
          iGuessCount++;
          SetWindowTextW(hEditGuess, L""); //czyszczenie po strzale
          //info zwrotne
          HWND hHintText = GetDlgItem(hWnd, IDC_STATICHINT);
          if (iGuess < iNumber) //za ma³o
            wsprintfW(szText, L"%d - Za ma³o", iGuess);
          else
            wsprintfW(szText, L"%d - Za du¿o", iGuess);
          SetWindowTextW(hHintText, szText);
        }

        return 0;
      }
    }

  }
  return FALSE;
}