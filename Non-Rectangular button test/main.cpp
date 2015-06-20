#include <windows.h>
#include <iostream>
#include "resource.h"


void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight);
HRGN CreateRgnFromFile(HBITMAP hBmp, COLORREF color);

HBITMAP hSkinMBmp = NULL;
HBITMAP hSkinLaunchBtnBmp = NULL;
BOOL destroyCaption = false;

// Declare our function
typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes;


#define BUFFER_OFFSET(i) ((char*)NULL + (i))
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

#define bitmapHeight            463
#define bitmapWidth             738

#define buttonBitmapHeight      61
#define buttonBitmapWidth       100

#define g_ColourKey             0xFF00FF // 0,0,255(pink) in hex RGB



LRESULT CALLBACK EllipPushWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc ;
    HRGN hRegion1;

    switch (message)
    {
    case WM_CREATE:
        SetWindowPos(hwnd, NULL, 0,0,buttonBitmapWidth, buttonBitmapHeight, SWP_NOMOVE | SWP_NOZORDER);
        hRegion1 = CreateRgnFromFile(hSkinLaunchBtnBmp, g_ColourKey);       // For a good explanation of how to use regions: http://win32xplorer.blogspot.ca/2009/09/regions-and-clipping-window-to-custom.html (June 2015)
        SetWindowRgn(hwnd, hRegion1, true);
        DeleteObject(hRegion1);
    break;

    case WM_PAINT :
        HDC dcSkin;
        BITMAP bm;
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        dcSkin = CreateCompatibleDC(hdc);
        GetObject(hSkinLaunchBtnBmp, sizeof(bm), &bm);
        SelectObject(dcSkin, hSkinLaunchBtnBmp);
        BitBlt(hdc, 0,0,buttonBitmapWidth,buttonBitmapHeight, dcSkin, 0, 0, SRCCOPY);
        DeleteDC(dcSkin);
        EndPaint(hwnd, &ps);
    return 0 ;

    case WM_KEYUP:
        if (wParam != VK_SPACE)
    break;

    // fall through
    case WM_LBUTTONUP:
        SendMessage (GetParent (hwnd), WM_COMMAND,
        GetWindowLong (hwnd, GWL_ID), (LPARAM) hwnd);
    return 0;
    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}

BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
        if(SetLayeredWindowAttributes != NULL)
        {
            if(destroyCaption == false) {
                DestroyCaption(hwndDlg,bitmapWidth,bitmapHeight);
                destroyCaption = true;
            }

          SetWindowLong(hwndDlg, GWL_EXSTYLE, GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
          SetLayeredWindowAttributes(hwndDlg, g_ColourKey, 0, LWA_COLORKEY);
        }
    }
    return TRUE;

    // draw our bitmap
    case WM_PAINT:
    {
        BITMAP bm;
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwndDlg, &ps);
        HDC dcSkin = CreateCompatibleDC(hdc);
        GetObject(hSkinMBmp, sizeof(bm), &bm);
        SelectObject(dcSkin, hSkinMBmp);
        BitBlt(hdc, 0,0,bitmapWidth,bitmapHeight, dcSkin, 0, 0, SRCCOPY);
        DeleteDC(dcSkin);
        EndPaint(hwndDlg, &ps);
    break;
    }

    case WM_CLOSE:
    {
        EndDialog(hwndDlg, 0);
    }
    return TRUE;

    case WM_COMMAND:
    {
        switch (LOWORD (wParam))
        {
            case IDOK :
                EndDialog (hwndDlg, 0) ;
                return TRUE ;
        }
    }
    return TRUE;

    }
    return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Load our bitmap
    hSkinMBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LauncherBackground));
    if(hSkinMBmp == NULL)
        std::cerr << "Could not load loader skin bitmap" << std::endl;

    hSkinLaunchBtnBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LAUNCH));
    if(hSkinLaunchBtnBmp == NULL)
        std::cerr << "Could not load launcher button skin bitmap" << std::endl;

    // import function to make windows transparent
    HMODULE hUser32 = GetModuleHandle(("USER32.DLL"));
    SetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    if(SetLayeredWindowAttributes == NULL)
        MessageBox(0, "Error, cannot load window transparency, REASON: Could not load User32.DLL", "Error!", MB_ICONSTOP | MB_OK);

    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = EllipPushWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = TEXT ("EllipPush");
    RegisterClass (&wndclass);


    return DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}


// Destroy our windows caption
void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight)
{
 DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
 dwStyle &= ~(WS_CAPTION|WS_SIZEBOX);

 SetWindowLong(hwnd, GWL_STYLE, dwStyle);
 InvalidateRect(hwnd, NULL, true);
 SetWindowPos(hwnd, NULL, 0,0,windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
}

// Function created by By Yuriy Zaporozhets (Retrieved from http://www.codeproject.com/Articles/573/CreateRegionFromFile, June 2015)
HRGN CreateRgnFromFile( HBITMAP hBmp, COLORREF color )
{
	// get image properties
	BITMAP bmp = { 0 };
	GetObject( hBmp, sizeof(BITMAP), &bmp );
	// allocate memory for extended image information
	LPBITMAPINFO bi = (LPBITMAPINFO) new BYTE[ sizeof(BITMAPINFO) + 8 ];
	memset( bi, 0, sizeof(BITMAPINFO) + 8 );
	bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	// set window size
	//DWORD m_dwWidth	= buttonBitmapWidth;		// bitmap width
	//DWORD m_dwHeight = buttonBitmapHeight;		// bitmap height
	// create temporary dc
	HDC dc = CreateIC( "DISPLAY",NULL,NULL,NULL );
	// get extended information about image (length, compression, length of color table if exist, ...)
	GetDIBits( dc, hBmp, 0, bmp.bmHeight, 0, bi, DIB_RGB_COLORS );
	// allocate memory for image data (colors)
	LPBYTE pBits = new BYTE[ bi->bmiHeader.biSizeImage + 4 ];
	// allocate memory for color table
	if ( bi->bmiHeader.biBitCount == 8 )
	{
		// actually color table should be appended to this header(BITMAPINFO),
		// so we have to reallocate and copy it
		LPBITMAPINFO old_bi = bi;
		// 255 - because there is one in BITMAPINFOHEADER
		bi = (LPBITMAPINFO)new char[ sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD) ];
		memcpy( bi, old_bi, sizeof(BITMAPINFO) );
		// release old header
		delete old_bi;
	}
	// get bitmap info header
	BITMAPINFOHEADER& bih = bi->bmiHeader;
	// get color table (for 256 color mode contains 256 entries of RGBQUAD(=DWORD))
	LPDWORD clr_tbl = (LPDWORD)&bi->bmiColors;
	// fill bits buffer
	GetDIBits( dc, hBmp, 0, bih.biHeight, pBits, bi, DIB_RGB_COLORS );
	DeleteDC( dc );

	BITMAP bm;
	GetObject( hBmp, sizeof(BITMAP), &bm );
	// shift bits and byte per pixel (for comparing colors)
	LPBYTE pClr = (LPBYTE)&color;
	// swap red and blue components
	BYTE tmp = pClr[0]; pClr[0] = pClr[2]; pClr[2] = tmp;
	// convert color if curent DC is 16-bit (5:6:5) or 15-bit (5:5:5)
	if ( bih.biBitCount == 16 )
	{
		// for 16 bit
		color = ((DWORD)(pClr[0] & 0xf8) >> 3) |
				((DWORD)(pClr[1] & 0xfc) << 3) |
				((DWORD)(pClr[2] & 0xf8) << 8);
		// for 15 bit
//		color = ((DWORD)(pClr[0] & 0xf8) >> 3) |
//				((DWORD)(pClr[1] & 0xf8) << 2) |
//				((DWORD)(pClr[2] & 0xf8) << 7);
	}

	const DWORD RGNDATAHEADER_SIZE	= sizeof(RGNDATAHEADER);
	const DWORD ADD_RECTS_COUNT		= 40;			// number of rects to be appended
													// to region data buffer

	// BitPerPixel
	BYTE	Bpp = bih.biBitCount >> 3;				// bytes per pixel
	// bytes per line in pBits is DWORD aligned and bmp.bmWidthBytes is WORD aligned
	// so, both of them not
	DWORD m_dwAlignedWidthBytes = (bmp.bmWidthBytes & ~0x3) + (!!(bmp.bmWidthBytes & 0x3) << 2);
	// DIB image is flipped that's why we scan it from the last line
	LPBYTE	pColor = pBits + (bih.biHeight - 1) * m_dwAlignedWidthBytes;
	DWORD	dwLineBackLen = m_dwAlignedWidthBytes + bih.biWidth * Bpp;	// offset of previous scan line
													// (after processing of current)
	DWORD	dwRectsCount = bih.biHeight;			// number of rects in allocated buffer
	INT		i, j;									// current position in mask image
	INT		first = 0;								// left position of current scan line
													// where mask was found
	bool	wasfirst = false;						// set when mask has been found in current scan line
	bool	ismask;									// set when current color is mask color


	// allocate memory for region data
	// region data here is set of regions that are rectangles with height 1 pixel (scan line)
	// that's why first allocation is <bm.biHeight> RECTs - number of scan lines in image
	RGNDATAHEADER* pRgnData =
		(RGNDATAHEADER*)new BYTE[ RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT) ];
	// get pointer to RECT table
	LPRECT pRects = (LPRECT)((LPBYTE)pRgnData + RGNDATAHEADER_SIZE);
	// zero region data header memory (header  part only)
	memset( pRgnData, 0, RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT) );
	// fill it by default
	pRgnData->dwSize	= RGNDATAHEADER_SIZE;
	pRgnData->iType		= RDH_RECTANGLES;

	for ( i = 0; i < bih.biHeight; i++ )
	{
		for ( j = 0; j < bih.biWidth; j++ )
		{
			// get color
			switch ( bih.biBitCount )
			{
			case 8:
				ismask = (clr_tbl[ *pColor ] != color);
				break;
			case 16:
				ismask = (*(LPWORD)pColor != (WORD)color);
				break;
			case 24:
				ismask = ((*(LPDWORD)pColor & 0x00ffffff) != color);
				break;
			case 32:
				ismask = (*(LPDWORD)pColor != color);
			}
			// shift pointer to next color
			pColor += Bpp;
			// place part of scan line as RECT region if transparent color found after mask color or
			// mask color found at the end of mask image
			if ( wasfirst )
			{
				if ( !ismask )
				{
					// save current RECT
					pRects[ pRgnData->nCount++ ] = {first, i, j, i + 1};    // Modified line to use RECT instead of CRect
					// if buffer full reallocate it with more room
					if ( pRgnData->nCount >= dwRectsCount )
					{
						dwRectsCount += ADD_RECTS_COUNT;
						// allocate new buffer
						LPBYTE pRgnDataNew = new BYTE[ RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT) ];
						// copy current region data to it
						memcpy( pRgnDataNew, pRgnData, RGNDATAHEADER_SIZE + pRgnData->nCount * sizeof(RECT) );
						// delte old region data buffer
						delete pRgnData;
						// set pointer to new regiondata buffer to current
						pRgnData = (RGNDATAHEADER*)pRgnDataNew;
						// correct pointer to RECT table
						pRects = (LPRECT)((LPBYTE)pRgnData + RGNDATAHEADER_SIZE);
					}
					wasfirst = false;
				}
			}
			else if ( ismask )		// set wasfirst when mask is found
			{
				first = j;
				wasfirst = true;
			}
		}

		if ( wasfirst && ismask )
		{
			// save current RECT
			pRects[ pRgnData->nCount++ ] = {first, i, j, i + 1};    // Modified line to use RECT instead of CRect
			// if buffer full reallocate it with more room
			if ( pRgnData->nCount >= dwRectsCount )
			{
				dwRectsCount += ADD_RECTS_COUNT;
				// allocate new buffer
				LPBYTE pRgnDataNew = new BYTE[ RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT) ];
				// copy current region data to it
				memcpy( pRgnDataNew, pRgnData, RGNDATAHEADER_SIZE + pRgnData->nCount * sizeof(RECT) );
				// delte old region data buffer
				delete pRgnData;
				// set pointer to new regiondata buffer to current
				pRgnData = (RGNDATAHEADER*)pRgnDataNew;
				// correct pointer to RECT table
				pRects = (LPRECT)((LPBYTE)pRgnData + RGNDATAHEADER_SIZE);
			}
			wasfirst = false;
		}

		pColor -= dwLineBackLen;
	}
	// release image data
	delete pBits;
	delete bi;

	// create region
	HRGN hRgn = ExtCreateRegion( NULL, RGNDATAHEADER_SIZE + pRgnData->nCount * sizeof(RECT), (LPRGNDATA)pRgnData );
	// release region data
	delete pRgnData;

	return hRgn;
}
