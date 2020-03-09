#include <Windows.h>
#include <cstdio>

#define IDLESS  IDNO
#define IDMORE  IDYES
#define IDHIT   IDCANCEL

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR szCmdLine, _In_ int iCmdShow)
{
  const wchar_t* csInfo = L"(Tak - wiêcej, Nie - mniej, Anuluj - trafione)\0";
  wchar_t sText[256] = {};

  int iMin = 0; //dolna granica mo¿liwego zakresu w którym le¿y liczba
  int iMax = 40; //górna granica
  int iOffset = 0; //przesuniêcie którejœ z granic po odpowiedzi u¿ytkownika
  int iGuess = (iMax - iMin) / 2; //liczba "strza³"

  wsprintf(sText, L"Pomyœl liczbê od 0 do 40.\nPo podaniu propozycji, okreœl czy twoja wybrana liczba jest\nwiêksza lub mniejsza.\n%s", csInfo);
    
  int iRet = MessageBoxW(0, sText, L"Gierka", MB_OKCANCEL);
  
  if (iRet == IDCANCEL)
    return 0;

  do
  {
    wsprintf(sText, L"Twoja liczba to %d?\n%s", iGuess, csInfo);
    iRet = MessageBoxW(0, sText, L"Strza³", MB_YESNOCANCEL);

    if (iRet == IDLESS)
      iMax = iGuess - 1;
    else if (iRet == IDMORE)
      iMin = iGuess + 1;

    if (iMin == iMax)
    {
      iGuess = iMax;
      break;
    }
    
    iGuess = ((iMax - iMin) / 2 + iMin);
  } while (iRet != IDHIT);

  wsprintf(sText, L"Twoja liczba to %d", iGuess);
  MessageBoxW(0, sText, L"Zgadniête", MB_OK);

  return 0;
}
/*
zadanie - gra w "jak¹ liczbê mam na myœli"
zakres 1-40, komputer zgaduje, u¿ytkownik wybiera
drzewko binarne zgadywanka
*/