#include "res.h"

#include <Windows.h>
#include <cmath>


#pragma comment(lib, "winmm.lib")

INT_PTR CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR szCmdLine, _In_ int iCmdShow)
{
  HWND hMainWindow = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_MAINVIEW), NULL, WindowProc);

  ShowWindow(hMainWindow, iCmdShow);

  //Prepairing device

  WAVEFORMATEX pcmWaveFormat;
  pcmWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
  pcmWaveFormat.nChannels = 1;
  pcmWaveFormat.nSamplesPerSec = 44100L;
  pcmWaveFormat.wBitsPerSample = 8;
  pcmWaveFormat.nAvgBytesPerSec = 44100L;
  pcmWaveFormat.nBlockAlign = 1;
  pcmWaveFormat.cbSize = 0;

  MMRESULT mmResult;
  HWAVEOUT hWO = 0;
  UINT devId;

  for(devId = 0; devId < waveOutGetNumDevs(); ++devId)
  {
    mmResult = waveOutOpen(&hWO, devId, &pcmWaveFormat, 0, 0, CALLBACK_NULL);
    if(mmResult == MMSYSERR_NOERROR)
      break;
  }

  if(mmResult != MMSYSERR_NOERROR)
  {
    MessageBoxW(hMainWindow, L"No sound card that can support specified parameters", L"Error", MB_OK | MB_ICONERROR);
    return mmResult;
  }

  //Prep samples

  WAVEHDR whdr;
  memset(&whdr, 0, sizeof(whdr));
  whdr.lpData = new char[pcmWaveFormat.nAvgBytesPerSec * 1];
  whdr.dwBufferLength = pcmWaveFormat.nAvgBytesPerSec * 1;
  whdr.dwUser = 0;
  whdr.dwFlags = 0;
  whdr.dwBytesRecorded = 0;
  whdr.lpNext = 0;
  whdr.reserved = 0;

  for(int i=0; i<whdr.dwBufferLength; ++i)
  {
    whdr.lpData[i] = 127 * sin(i * 200.0 / static_cast<double>(pcmWaveFormat.nSamplesPerSec)) + 127;
  }

  waveOutSetVolume(hWO, 0xFFFFFFFF);
  mmResult = waveOutPrepareHeader(hWO, &whdr, sizeof(whdr));

  if(mmResult != MMSYSERR_NOERROR)
  {
    MessageBoxW(hMainWindow, L"Cannot initialize sound card", L"Error", MB_OK | MB_ICONERROR);
    return mmResult;
  }

  mmResult = waveOutWrite(hWO, &whdr, sizeof(whdr));
  if(mmResult != MMSYSERR_NOERROR)
  {
    MessageBoxW(hMainWindow, L"Cannot load samples", L"Error", MB_OK | MB_ICONERROR);
    return mmResult;
  }

  while((whdr.dwFlags & WHDR_DONE) != WHDR_DONE)
    Sleep(100);

  mmResult = waveOutUnprepareHeader(hWO, &whdr, sizeof(whdr));
  mmResult = waveOutClose(hWO);
  delete[] whdr.lpData;

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
  switch(uMsg)
  {
    case WM_INITDIALOG:
    {

      return TRUE;
    }
    case WM_CLOSE:
    {
      DestroyWindow(hWnd);
      PostQuitMessage(0);

      return TRUE;
    }
  }
  return FALSE;//DefWindowProcW(hwnd, uMsg, wParam, lParam);
}