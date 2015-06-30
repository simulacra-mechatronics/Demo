#ifndef LAUNCHER_H_INCLUDED
#define LAUNCHER_H_INCLUDED

#include "CustomSkinnedButton.h"
#include "BitmapSkin.h"
#include "../resource.h"
#include "openGLWindow.h"

extern CustomSkinnedButton* pRunButton;
extern CustomSkinnedButton* pExitButton;
static BitmapSkin* pDialogSkin;
static COLORREF colorKey   = RGB(255,0,255);        // Color used in bitmap to designate transparent areas
static DWORD LWA_COLORKEY  = 0x00000001;            // Use colorKey as the transparency color
static BOOL destroyCaption = false;                 // A boolean flag to determine whether the dialog window caption has been removed.

void createButtons(HINSTANCE hInstance);
void deleteButtons();
void registerButtonWinClasses(HINSTANCE hInstance);
bool createLauncherWindow(HINSTANCE hInstance);
void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight);

LRESULT CALLBACK ExitButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RunButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

// Declare our function
typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
static lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes;

#endif // LAUNCHER_H_INCLUDED
