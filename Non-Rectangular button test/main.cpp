#include <windows.h>
#include <iostream>     // Used for outputting info via 'cout' or 'cerr' to debug window
#include "resource.h"   // Contains declarations of resources contained in resource file


void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight);  // Used to remove window caption and size window to bitmap size
HRGN CreateRgnFromFile(HBITMAP hBmp, COLORREF color);               // Function that accepts a bitmap with color keyed to transparency and defines a region according to non transparent area

typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);   // Get a function pointer
lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes;  // Set instance of function pointer

static COLORREF colorKey   = RGB(255,0,255);        // Color used in bitmap to designate transparent areas
static DWORD LWA_COLORKEY  = 0x00000001;            // Use colorKey as the transparency color

static unsigned int uiBitmapHeight   = 463;         // Height of the bitmap for dialog background
static unsigned int uiBitmapWidth    = 738;         // Width of the bitmap for the dialog background

static unsigned int uiRunBtnBitmapHeight = 35;      // Height of the bitmaps or the 'Run' button
static unsigned int uiRunBtnBitmapWidth  = 93;      // Width of the bitmaps for the 'Run' button

static unsigned int uiExitBtnBitmapHeight = 35;     // Height of the bitmaps for the 'Exit' button
static unsigned int uiExitBtnBitmapWidth  = 93;     // Width of the bitmaps for the 'Exit' button

HBITMAP hSkinMBmp = NULL;                           // Handle to the bitmap for the dialog background
HBITMAP hBtnSkinRunOut = NULL;                      // Handle to the bitmap for the 'Run' button when the mouse is hovering over the button
HBITMAP hSkinLaunchBtnRunOver = NULL;               // Handle to the bitmap for the 'Run' button when the Left mouse button has been clicked AND the mouse is hovering over the button
HBITMAP hSkinLaunchBtnRunIn = NULL;                 // Handle to the bitmap for the 'Run' button when the mouse is not over it
HBITMAP hBtnSkinExitOut = NULL;                     // Handle to the bitmap for the 'Exit' button when the mouse is hovering over the button
HBITMAP hSkinLaunchBtnExitOver = NULL;              // Handle to the bitmap for the 'Exit' button when the Left mouse button has been clicked AND the mouse is hovering over the button
HBITMAP hSkinLaunchBtnExitIn = NULL;                // Handle to the bitmap for the 'Exit' button when the Left mouse button has been clicked AND the mouse is hovering over the button
BOOL destroyCaption = false;                        // A boolean flag to determine whether the dialog window caption has been removed.

static BOOL bRunMouseOver = false;                  // A boolean flag set to signal that the mouse is hovering over the 'Run' button
static BOOL bRunMouseIn = false;                    // A boolean flag set to signal that the left mouse button was clicked while over the 'Run' button
static BOOL bExitMouseOver = false;                 // A boolean flag set to signal that the mouse is hovering over the 'Exit' button
static BOOL bExitMouseIn = false;                   // A boolean flag set to signal that the left mouse button was clicked while over the 'Exit' button


// Window procedure for the owner-draw custom control IDEXIT ('Exit' button)
LRESULT CALLBACK ExitButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc ;                                // Handle to the button's device context
    static HRGN hExitButnRgn;                       // Handle to the button's visible region

    switch (message)                                // Test for specific messages
    {

	case WM_LBUTTONDBLCLK:                          // Left mouse button has been clicked
        PostMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);  // Pass message back into this window procedure as a WM_LBUTTONDOWN message (Left mouse button down)
        break;

	case WM_MOUSEMOVE:                              // Mouse has been detected moving (hovering) over the button
	{
        if(!bExitMouseOver){                        // If the hover flag has not been set
            bExitMouseOver = TRUE;                  // Set the hover flag to true
            bRunMouseOver = FALSE;                  // Make sure the hover flag for the other flag is turned off
            InvalidateRgn(hwnd, NULL, FALSE);       // Cause this button to be redrawn
            InvalidateRgn(GetDlgItem(GetParent(hwnd),IDRUN), NULL, FALSE);  // Redraw the other button just in case it was also recently flagged as in hover
        }

        return (0);
	}
	break;

    case WM_CREATE:                                 // Called when the button is first created
        SetWindowPos(hwnd, NULL, 0,0,uiExitBtnBitmapWidth, uiExitBtnBitmapHeight, SWP_NOMOVE | SWP_NOZORDER);   // Set the button to the width and height of the bitmaps being used
        hExitButnRgn = CreateRgnFromFile(hBtnSkinExitOut, colorKey);       // For a good explanation of how to use regions: http://win32xplorer.blogspot.ca/2009/09/regions-and-clipping-window-to-custom.html (June 2015)
        SetWindowRgn(hwnd, hExitButnRgn, true);     // Set the region as the visible area
        DeleteObject(hExitButnRgn);                 // Delete the region
    break;

    case WM_PAINT:                                  // Draw the visible region of the button
        HDC dcSkin;                                 // Handle to a compatible memory device context used for drawing the bitmap to the visible area
        BITMAP bm;                                  // Bitmap structure
        PAINTSTRUCT ps;                             // Paint structure
        hdc = BeginPaint(hwnd, &ps);                // Set the handle to the device context to the drawable area
        dcSkin = CreateCompatibleDC(hdc);           // Create a memory device context that exits only in memory

        if(bExitMouseIn == true && bExitMouseOver == true){ // Check to see if the mouse is hovering over the button AND that the left mouse button is held down
            GetObject(hSkinLaunchBtnExitIn, sizeof(bm), &bm); // Store information about the bitmap used to represent this state in the bitmap structure
            SelectObject(dcSkin, hSkinLaunchBtnExitIn); // Select this bitmap into the memory device context
        }
        else if(bExitMouseOver == true){            // Check to see if the mouse is only hovering
            GetObject(hSkinLaunchBtnExitOver, sizeof(bm), &bm);
            SelectObject(dcSkin, hSkinLaunchBtnExitOver);
        }
        else{                                       // The mouse is neither hovering nor is the left button being held down
            GetObject(hBtnSkinExitOut, sizeof(bm), &bm);
            SelectObject(dcSkin, hBtnSkinExitOut);
        }

        BitBlt(hdc, 0,0,uiExitBtnBitmapWidth,uiExitBtnBitmapHeight, dcSkin, 0, 0, SRCCOPY);  // Performs bit-block transfer of bitmap pixels to the memory device context
        DeleteDC(dcSkin);                           // Delete the memory device context
        EndPaint(hwnd, &ps);                        // Exits the painting process for the 'Run' button
    return 0 ;

    case WM_LBUTTONUP:                              // Left mouse button has been released over the 'Run' Button
        if(bExitMouseIn)                            // Make sure that the left mouse button was clicked while over the exit button
            SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowLong(hwnd, GWL_ID), (LPARAM)hwnd);    // Send this message to the dialog's main window procedure for processing.
    return 0;

    case WM_LBUTTONDOWN:                            // The left mouse button is being held down
    {
        bExitMouseIn = true;                        // Set a flag to signify that the left mouse button is being held down
        if(bRunMouseIn)                             // If the left mouse button was previously set while over the 'Run' button but not reset before the mouse was moved over to the 'Exit' button
            bRunMouseIn = false;                    // Set the flag signifying that the left mouse button is down over the 'Run' button to false
        InvalidateRgn(hwnd, NULL, FALSE);           // Redraw this button with the image signifying that the button is being held down
        break;
    }

    }

    return DefWindowProc (hwnd, message, wParam, lParam);   // Send any unhandled messages back to the main dialog window procedure
}


// Window procedure for the owner-draw custom control IDRUN ('Run' button) - See previous function for comments
LRESULT CALLBACK RunButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc ;
    static HRGN hRunButnRgn;

    switch (message)
    {

	case WM_LBUTTONDBLCLK:
        PostMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
        break;

	case WM_MOUSEMOVE:
	{
        if(!bRunMouseOver){
            bRunMouseOver = TRUE;
            bExitMouseOver = FALSE;
            InvalidateRgn(hwnd, NULL, FALSE);
            InvalidateRgn(GetDlgItem(GetParent(hwnd),IDEXIT), NULL, FALSE);
        }

        return (0);
	}
	break;

    case WM_CREATE:
        SetWindowPos(hwnd, NULL, 0,0,uiRunBtnBitmapWidth, uiRunBtnBitmapHeight, SWP_NOMOVE | SWP_NOZORDER);
        hRunButnRgn = CreateRgnFromFile(hBtnSkinRunOut, colorKey);       // For a good explanation of how to use regions: http://win32xplorer.blogspot.ca/2009/09/regions-and-clipping-window-to-custom.html (June 2015)
        SetWindowRgn(hwnd, hRunButnRgn, true);
        DeleteObject(hRunButnRgn);
    break;

    case WM_PAINT :
        HDC dcSkin;
        BITMAP bm;
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        dcSkin = CreateCompatibleDC(hdc);

        if(bRunMouseIn == true && bRunMouseOver == true){
            GetObject(hSkinLaunchBtnRunIn, sizeof(bm), &bm);
            SelectObject(dcSkin, hSkinLaunchBtnRunIn);
        }
        else if(bRunMouseOver == true){
            GetObject(hSkinLaunchBtnRunOver, sizeof(bm), &bm);
            SelectObject(dcSkin, hSkinLaunchBtnRunOver);
        }
        else{
            GetObject(hBtnSkinRunOut, sizeof(bm), &bm);
            SelectObject(dcSkin, hBtnSkinRunOut);
        }

        BitBlt(hdc, 0,0,uiRunBtnBitmapWidth,uiRunBtnBitmapHeight, dcSkin, 0, 0, SRCCOPY);
        DeleteDC(dcSkin);
        EndPaint(hwnd, &ps);
    return 0 ;

    case WM_LBUTTONUP:
        if(bRunMouseIn)
            SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowLong(hwnd, GWL_ID), (LPARAM)hwnd);
    return 0;

    case WM_LBUTTONDOWN:
    {
        bRunMouseIn = true;
        if(bExitMouseIn)
            bExitMouseIn = false;
        InvalidateRgn(hwnd, NULL, FALSE);
        break;
    }

    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}

// The main window procedure used by the dialog window
BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hRunButtonWnd;                      // A handle to the 'Run' button
    static HWND hExitButtonWnd;                     // A handle to the 'Exit' button

    switch(uMsg)                                    // Search through messages sent to this window procedure
    {
    case WM_INITDIALOG:                             // Creating of the dialog window
    {
        if(SetLayeredWindowAttributes != NULL)      // Make sure that this function exits
        {
            if(destroyCaption == false) {           // Make sure that the caption has not already been destroyed
                DestroyCaption(hwndDlg,uiBitmapWidth,uiBitmapHeight);   // Destroy any window caption that may be set
                destroyCaption = true;              // Set a flag to ensure that this has been accomplished
            }

          SetWindowLong(hwndDlg, GWL_EXSTYLE, GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED); // Set the window style
          SetLayeredWindowAttributes(hwndDlg, colorKey, 0, LWA_COLORKEY);                           // Set the transparency color key
        }
        hRunButtonWnd = ::GetDlgItem(hwndDlg,IDRUN);    // Get the window handle for the 'Run' button
        hExitButtonWnd = ::GetDlgItem(hwndDlg,IDEXIT);  // Get the window handle for the 'Exit' button
    }
    return TRUE;

    // draw our bitmap
    case WM_PAINT:                                  // Draw the dialog window
    {
        BITMAP bm;                                  // Create a bitmap structure
        PAINTSTRUCT ps;                             // Create a paint structure
        HDC hdc = BeginPaint(hwndDlg, &ps);         // Create a device context used for the dialog window
        HDC dcSkin = CreateCompatibleDC(hdc);       // Create a compatible memory device context to copy the color information from the bitmap to
        GetObject(hSkinMBmp, sizeof(bm), &bm);      // Fill bitmap structure with information about the background image bitmap
        SelectObject(dcSkin, hSkinMBmp);            // Select this bitmap into the memory device context
        BitBlt(hdc, 0,0,uiBitmapWidth,uiBitmapHeight, dcSkin, 0, 0, SRCCOPY);   // Performs bit-block transfer of bitmap pixels to the memory device context
        DeleteDC(dcSkin);                           // Release the memory device context
        EndPaint(hwndDlg, &ps);                     // End painting of dialog window
    break;
    }

    case WM_CLOSE:                                  // Exit application
    {
        EndDialog(hwndDlg, 0);                      // Close down the dialog window
    }
    return TRUE;

    case WM_COMMAND:                                // Button has been clicked
    {
        switch (LOWORD (wParam))
        {
            case IDRUN:                             // 'Run' button was clicked
                EndDialog (hwndDlg, 0) ;            // Close the dialog window (or anything else you want to happen)
                return TRUE ;
            case IDEXIT :                           // 'Exit' button was clicked
                EndDialog (hwndDlg, 0) ;            // Close the dialog window because the user is exiting the application
                return TRUE ;
        }
    }
    return TRUE;

    case WM_MOUSEMOVE:                              // Mouse has been moved while over the dialog window area
    {
        if(bRunMouseOver){                          // Check to see if the mouse was previously over the 'Run' button
            bRunMouseOver = FALSE;                  // Set a flag to signify that the mouse is not hovering over the 'Run' button any more
            InvalidateRgn(hRunButtonWnd, NULL, FALSE);  // Redraw the 'Run' button with the default state
        }
        if(bExitMouseOver){                         // Check to see if the mouse was previously over the 'Exit' button
            bExitMouseOver = FALSE;                 // Set a flag to signify that the mouse is not hovering over the 'Exit' button any more
            InvalidateRgn(hExitButtonWnd, NULL, FALSE); // Redraw the 'Exit' button with the default state
        }
    }
    break;

    // Moves the window when the user clicks anywhere not covered by a control. HTCAPTION specifies
    // that all button clicks originate in the title bar area - even when the window has no title bar.
    case WM_LBUTTONDOWN:
    {
        PostMessage(hwndDlg, WM_NCLBUTTONDOWN, HTCAPTION,0);
    }
    return TRUE;

    case WM_LBUTTONUP:                              // The left mouse button was released
    {
        if(bRunMouseIn){                            // Check to see if the mouse button was previously flagged as down over the 'Run' button
            bRunMouseIn = FALSE;                    // Set a flag to signify that the 'Run' button does not have the left mouse button clicked over it any more
            InvalidateRgn(hRunButtonWnd, NULL, FALSE);  // Redraw the 'Run' button in its default state
        }
        if(bExitMouseIn){                           // Check to see if the mouse button was previously flagged as down over the 'Exit' button
            bExitMouseIn = FALSE;                   // Set a flag to signify that the 'Exit' button does not have the left mouse button clicked over it any more
            InvalidateRgn(hExitButtonWnd, NULL, FALSE); // Redraw the 'Exit' button in its default state
        }
    }

    }
    return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Load our bitmaps
    hSkinMBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LauncherBackground));
    if(hSkinMBmp == NULL)
        std::cerr << "Could not load loader skin bitmap" << std::endl;

    hBtnSkinRunOut = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_RunOut));
    if(hBtnSkinRunOut == NULL)
        std::cerr << "Could not load RunOut button skin bitmap" << std::endl;

    hSkinLaunchBtnRunOver = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_RunOver));
    if(hSkinLaunchBtnRunOver == NULL)
        std::cerr << "Could not load RunOver button skin bitmap" << std::endl;

    hSkinLaunchBtnRunIn = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_RunIn));
    if(hSkinLaunchBtnRunIn == NULL)
        std::cerr << "Could not load RunIn button skin bitmap" << std::endl;

    hBtnSkinExitOut = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ExitOut));
    if(hBtnSkinExitOut == NULL)
        std::cerr << "Could not load ExitOut button skin bitmap" << std::endl;

    hSkinLaunchBtnExitOver = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ExitOver));
    if(hSkinLaunchBtnExitOver == NULL)
        std::cerr << "Could not load ExitOver button skin bitmap" << std::endl;

    hSkinLaunchBtnExitIn = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_ExitIn));
    if(hSkinLaunchBtnExitIn == NULL)
        std::cerr << "Could not load ExitIn button skin bitmap" << std::endl;

    // Import function to make windows transparent
    HMODULE hUser32 = GetModuleHandle(("USER32.DLL"));
    SetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    if(SetLayeredWindowAttributes == NULL)
        std::cerr << "Error: could not load window transparency. Could not load User32.DLL" << std::endl;

    // Create the window class for our 'Run' button
    WNDCLASS runWndclass;
    runWndclass.style = CS_HREDRAW | CS_VREDRAW;
    runWndclass.lpfnWndProc = RunButtonWndProc;         // Where we specify the name of the window procedure
    runWndclass.cbClsExtra = 0;
    runWndclass.cbWndExtra = 0;
    runWndclass.hInstance = hInstance;
    runWndclass.hIcon = NULL;
    runWndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
    runWndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    runWndclass.lpszMenuName = NULL;
    runWndclass.lpszClassName = TEXT ("runBtnProc");    // This value is reference by the IDRUN control in the RC file
    RegisterClass (&runWndclass);

    // Create the window class for our 'Exit' button
    WNDCLASS exitWndclass;
    exitWndclass.style = CS_HREDRAW | CS_VREDRAW;
    exitWndclass.lpfnWndProc = ExitButtonWndProc;       // Where we specify the name of the window procedure
    exitWndclass.cbClsExtra = 0;
    exitWndclass.cbWndExtra = 0;
    exitWndclass.hInstance = hInstance;
    exitWndclass.hIcon = NULL;
    exitWndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
    exitWndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    exitWndclass.lpszMenuName = NULL;
    exitWndclass.lpszClassName = TEXT ("exitBtnProc");  // This value is referenced by the IDEXIT control in the RC file
    RegisterClass (&exitWndclass);

    // Launch the dialog window
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
