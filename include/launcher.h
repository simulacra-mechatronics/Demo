#ifndef LAUNCHER_H_INCLUDED
#define LAUNCHER_H_INCLUDED

#include "CustomSkinnedButton.h"
#include "BitmapSkin.h"

// Declare our function
typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes;

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ExitButtonWndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK RunButtonWndProc (HWND, UINT, WPARAM, LPARAM);

#endif // LAUNCHER_H_INCLUDED
