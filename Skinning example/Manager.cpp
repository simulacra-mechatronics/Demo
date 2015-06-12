
// includes
#include <windows.h>
#include <stdio.h>
#include "res.h"

// defines
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

#define bitmapHeight            385
#define bitmapWidth             425

#define g_ColourKey             0xFF00FF // 0,0,255(pink) in hex RGB

// delcare our function
typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes;

HBITMAP hSkinMBmp = NULL;
int i = 0;

// Destroy our windows caption
void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight)
{
 DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
 dwStyle &= ~(WS_CAPTION|WS_SIZEBOX);

 SetWindowLong(hwnd, GWL_STYLE, dwStyle);
 InvalidateRect(hwnd, NULL, true);
 SetWindowPos(hwnd, NULL, 0,0,windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
}

// Our dialogs callback routine
BOOL CALLBACK dialog(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  	switch(Message)
	{
     // make window layered and set all pixels with an RGB value of 0,0,255 invisible
	  case WM_INITDIALOG:
          {
            if(SetLayeredWindowAttributes != NULL)
            {
              if(i < 1) {
               DestroyCaption(hwnd,bitmapWidth,bitmapHeight);
               i++;
              }

              SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
              SetLayeredWindowAttributes(hwnd, g_ColourKey, 0, LWA_COLORKEY);
            }

            break;
          }

       // move our window
        case WM_LBUTTONDOWN:
           {
            PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION,0);
            break;
           }

       // cleanup
        case WM_CLOSE:
           {
		    DeleteObject(hSkinMBmp);
            EndDialog(hwnd, 0);
		    break;
           }

       // draw our bitmap
        case WM_PAINT:
           {
            BITMAP bm;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC dcSkin = CreateCompatibleDC(hdc);
            GetObject(hSkinMBmp, sizeof(bm), &bm);
            SelectObject(dcSkin, hSkinMBmp);
            BitBlt(hdc, 0,0,bitmapWidth,bitmapHeight, dcSkin, 0, 0, SRCCOPY);
            DeleteDC(dcSkin);
            EndPaint(hwnd, &ps);
            break;
           }

        case WM_COMMAND:
           switch(LOWORD(wParam))
			{
			 // btn commands here
            }
       default:
			return FALSE;
	}
	return TRUE;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
  // load our bitmap
  hSkinMBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MSKIN));
    if(hSkinMBmp == NULL)
     MessageBox(0, "Could not load Skin", "Warning", MB_OK | MB_ICONEXCLAMATION);

 // import function to make windows transparent
  HMODULE hUser32 = GetModuleHandle(("USER32.DLL"));
  SetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
     if(SetLayeredWindowAttributes == NULL)
      MessageBox(0, "Error, cannot load window transparency, REASON: Could not load User32.DLL", "Error!", MB_ICONSTOP | MB_OK);

   // create ourlog
   return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, dialog);
}


