 /*-------------\
|   main.cpp     | Demo Challenge OpenGL 3.3 Framework
 \*-------------/ ------------------------------------------------------------------------------------------*\
  Version 3.3 - 06/05/2015

  Author: Godvalve
  Email: godvalve@hotmail.com

  Participate in the Demo Challenge. Visit: http://demochronicles.mccolm.org/compete.php

  Description: A simple OpenGL framework derived from the default project produced by the Code::Blocks project wizard.
  Read a thorough description of how this file works at http://demochronicles.mccolm.org/build.php?mode=nextPage&article=10
\*----------------------------------------------------------------------------------------------------------*/

#include <windows.h>
#include <iostream>
#include <stdio.h>      // Used of fprintf and stderr
#include <GL/glew.h>    // Used for OpenGL types like GLuint
#include <GL/wglew.h>   // Windows specific OpenGL Extension Wrangler library definitions
#include <GL/glext.h>   // OpenGL extension definitions (OpenGL Working Group)

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
using namespace glm;


#include "resource.h"   // The reouce class with program icons and launcher dialogue box
#include "Shader.h"     // Our GLSL Shader class
#include "CustomSkinnedButton.h"
#include "BitmapSkin.h"

using std::string;
using std::ifstream;
using std::vector;


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
float fTime=0.f, fDeltaTime=0.f;
char szAppName[] = "-= Demo Challenge Framework =-";
static BOOL fullscreen;
int iScreenWidth;
int iScreenHeight;
int iBitsPerPel;
float fAspectRatio;
float fFieldOfView;
Shader *demoShader;         // Declare a Shader object

HBITMAP hSkinMBmp = NULL;
HBITMAP hSkinLaunchBtnBmp = NULL;
int i = 0;
int j = 0;

LARGE_INTEGER TimerFreq;	// Timer Frequency.
LARGE_INTEGER TimeStart;	// Time of start.
LARGE_INTEGER TimeCur;		// Current time.

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 modelMatrix;
glm::mat4 MVP;

GLuint mvpMatrixLocationInShader;

GLuint vertexBuffer;        // This will identify our vertex buffer
GLuint colorBuffer;         // Our color buffer

HWND hLaunchButton;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ExitButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RunButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void EnableOpenGL(HWND hWnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);
void ConfigureViewport(GLsizei, GLsizei);
void ResizeViewport(GLsizei, GLsizei);
void InitializeDemo();
void RenderNextDemoFrame();
void ChangeDisplayMode();
void RestoreDisplayMode();
void createSquare();
void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight);

// Declare our function
typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes;


#define BUFFER_OFFSET(i) ((char*)NULL + (i))
//define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

#define bitmapHeight            463
#define bitmapWidth             738

#define buttonbitmapHeight            61
#define buttonbitmapWidth             100

#define g_ColourKey             0xFF00FF // 0,0,255(pink) in hex RGB

static COLORREF colorKey   = RGB(255,0,255);        // Color used in bitmap to designate transparent areas
static DWORD LWA_COLORKEY  = 0x00000001;            // Use colorKey as the transparency color
BOOL destroyCaption = false;                        // A boolean flag to determine whether the dialog window caption has been removed.

BitmapSkin* pDialogSkin;
CustomSkinnedButton* pRunButton;
CustomSkinnedButton* pExitButton;

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;

    BOOL bQuit = FALSE;
    fullscreen = TRUE;

    // Only shows on the console window if in debug build. Use release mode or change build options to remove console window.
    std::cout << "OpenGL Demo Framework version 3.3 (June 05, 2015)\n";

    // Load our bitmaps
    pDialogSkin = new BitmapSkin(hInstance, IDB_LauncherBackground);
    pRunButton = new CustomSkinnedButton(hInstance, IDB_RunOut, IDB_RunOver, IDB_RunIn, colorKey);
    pExitButton = new CustomSkinnedButton(hInstance, IDB_ExitOut, IDB_ExitOver, IDB_ExitIn, colorKey);

    // import function to make windows transparent
    HMODULE hUser32 = GetModuleHandle(("USER32.DLL"));
    SetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    if(SetLayeredWindowAttributes == NULL)
        MessageBox(0, "Error, cannot load window transparency, REASON: Could not load User32.DLL", "Error!", MB_ICONSTOP | MB_OK);


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


    /* Display the Demo Challenge Launcher to get user's preferences for fullscreen and resolution */
    if(DialogBoxParam(hInstance, MAKEINTRESOURCE(DLG_MAIN), HWND_DESKTOP, DlgProc, 0) == FALSE)
    {
        return 0;
    }

    delete pDialogSkin;
    delete pRunButton;
    delete pExitButton;

    /* Register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_SMICON);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szAppName;
    wcex.hIconSm = LoadIcon(hInstance, (LPCTSTR)IDI_SMICON);

    if (!RegisterClassEx(&wcex))
        return 0;

    /* Create main window */
    if(fullscreen == FALSE)
    {
        hWnd = CreateWindowEx(0,
                          szAppName,
                          szAppName,
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          iScreenWidth + GetSystemMetrics(SM_CXSIZEFRAME) * 2,
                          iScreenHeight + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME) * 2,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
    }
    else
	{
	    /* Bring up the application in fullscreen mode */
		hWnd = CreateWindow(
				szAppName,
				szAppName,
				WS_POPUP,
				0, 0,
				iScreenWidth, iScreenHeight,
				NULL, NULL,
				hInstance,
				0 );
	}

    /* Reset the timer variables */
	QueryPerformanceFrequency(&TimerFreq);
	QueryPerformanceCounter(&TimeStart);

    if(fullscreen)
    {
      ShowCursor(FALSE);
      ChangeDisplayMode();
    }

    /* Enable OpenGL for the window */
    EnableOpenGL(hWnd, &hDC, &hRC);

    ShowWindow(hWnd, nCmdShow);

    InitializeDemo();                                           // new

    /* Program main loop */
    while (!bQuit)
    {
        /* Check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* Handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
			/* Get the current time, and update the time controller. */
			QueryPerformanceCounter(&TimeCur);
			float fOldTime = fTime;
			fTime = (float)((double)(TimeCur.QuadPart-TimeStart.QuadPart)/(double)TimerFreq.QuadPart);
			fDeltaTime = fTime - fOldTime;

            /* Render the next frame in our demo */
			RenderNextDemoFrame();                              // new

            /* Change logical order of back and front buffer */
            SwapBuffers(hDC);
        }
    }
    /* Shutdown OpenGL */
    DisableOpenGL(hWnd, hDC, hRC);

    /* Destroy the window explicitly */
    DestroyWindow(hWnd);

    return msg.wParam;
}

//--------------------------------------------------------------------------------------
// Set the rendering area to accomodate the selected aspect ratio
//--------------------------------------------------------------------------------------
void ConfigureViewport(GLsizei width, GLsizei height)
{
    /* Prevent a divide by Zero */
	if( height == 0 )
		height = 1;

    GLfloat fActualAspectRatio = ((GLfloat)width/(GLfloat)height);
    GLsizei x_offset = 0;
    GLsizei y_offset = 0;
    GLsizei viewPortWidth = width;
    GLsizei viewPortHeight = height;

    if(fAspectRatio < fActualAspectRatio)
    {
        GLfloat visibleWidth = (GLfloat)height * fAspectRatio;
        x_offset = (GLsizei)((width-(int)visibleWidth)/2.0f);
        viewPortWidth = (int)visibleWidth;
    }
    else if(fAspectRatio > fActualAspectRatio)
    {
        GLfloat visibleHeight = (GLfloat)width/fAspectRatio;
        y_offset =(GLsizei)((height-(int)visibleHeight)/2.0f);
        viewPortHeight = (int)visibleHeight;
    }

	/* Set viewport to our new dimensions. */
	glViewport( x_offset, y_offset, viewPortWidth, viewPortHeight);
}

//--------------------------------------------------------------------------------------
// Resize the window and reset the projection matrix
//--------------------------------------------------------------------------------------
void ResizeViewport(GLsizei width, GLsizei height)
{
    /* Set the viewport to conform to selected aspect ratio */
    ConfigureViewport(width, height);

	projectionMatrix = glm::perspective(fFieldOfView, fAspectRatio, 0.1F, 100.0F);     // Creates a perspective projection matrix
}

//--------------------------------------------------------------------------------------
// Set screen to fullscreen mode
//--------------------------------------------------------------------------------------
void ChangeDisplayMode()
{
  DEVMODE device_mode;

  memset(&device_mode, 0, sizeof(DEVMODE));
  device_mode.dmSize = sizeof(DEVMODE);
  device_mode.dmPelsWidth  = iScreenWidth;
  device_mode.dmPelsHeight = iScreenHeight;
  device_mode.dmBitsPerPel = iBitsPerPel;
  device_mode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

  ChangeDisplaySettings(&device_mode, CDS_FULLSCREEN);
}

//--------------------------------------------------------------------------------------
// Set screen to windowed mode using values in registry
//--------------------------------------------------------------------------------------
void RestoreDisplayMode()
{
    ChangeDisplaySettings(NULL, 0);
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        /* Catch and handle window resize event */
		case WM_SIZE:
			ResizeViewport(LOWORD(lParam),HIWORD(lParam));		// LoWord= width, HiWord= height
		break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}



// Window procedure for the owner-draw custom control IDEXIT ('Exit' button)
LRESULT CALLBACK ExitButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc ;                                // Handle to the button's device context

    switch (message)                                // Test for specific messages
    {

	case WM_LBUTTONDBLCLK:                          // Left mouse button has been clicked
        PostMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);  // Pass message back into this window procedure as a WM_LBUTTONDOWN message (Left mouse button down)
        break;

	case WM_MOUSEMOVE:                              // Mouse has been detected moving (hovering) over the button
	{
        if(pExitButton->isMouseOver() == false){    // If the hover flag has not been set
            pExitButton->mouseIsOver();             // Set the hover flag to true
            pRunButton->mouseIsOut();               // Make sure the hover flag for the other flag is turned off
            InvalidateRgn(hwnd, NULL, FALSE);       // Cause this button to be redrawn
            InvalidateRgn(GetDlgItem(GetParent(hwnd),IDRUN), NULL, FALSE);  // Redraw the other button just in case it was also recently flagged as in hover
        }

        return (0);
	}
	break;

    case WM_CREATE:                                 // Called when the button is first created
        SetWindowPos(hwnd, NULL, 0,0,pExitButton->getBitmapWidth(), pExitButton->getBitmapHeight(), SWP_NOMOVE | SWP_NOZORDER);   // Set the button to the width and height of the bitmaps being used
        SetWindowRgn(hwnd, pExitButton->getHandleToVisibleRegion(), true);     // Set the region as the visible area
        DeleteObject(pExitButton->getHandleToVisibleRegion());                 // Delete the region
    break;

    case WM_PAINT:                                  // Draw the visible region of the button
        HDC dcSkin;                                 // Handle to a compatible memory device context used for drawing the bitmap to the visible area
        BITMAP bm;                                  // Bitmap structure
        PAINTSTRUCT ps;                             // Paint structure
        hdc = BeginPaint(hwnd, &ps);                // Set the handle to the device context to the drawable area
        dcSkin = CreateCompatibleDC(hdc);           // Create a memory device context that exits only in memory

        if(pExitButton->getbuttonState() == BUTTONSTATE_IN){ // Check to see if the mouse is hovering over the button AND that the left mouse button is held down
            GetObject(pExitButton->getHandleButtonInBitmap(), sizeof(bm), &bm); // Store information about the bitmap used to represent this state in the bitmap structure
            SelectObject(dcSkin, pExitButton->getHandleButtonInBitmap()); // Select this bitmap into the memory device context
        }
        else if(pExitButton->getbuttonState() == BUTTONSTATE_OVER){            // Check to see if the mouse is only hovering
            GetObject(pExitButton->getHandleButtonOverBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pExitButton->getHandleButtonOverBitmap());
        }
        else{                                       // The mouse is neither hovering nor is the left button being held down
            GetObject(pExitButton->getHandleButtonOutBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pExitButton->getHandleButtonOutBitmap());
        }

        BitBlt(hdc, 0,0,pExitButton->getBitmapWidth(),pExitButton->getBitmapHeight(), dcSkin, 0, 0, SRCCOPY);  // Performs bit-block transfer of bitmap pixels to the memory device context
        DeleteDC(dcSkin);                           // Delete the memory device context
        EndPaint(hwnd, &ps);                        // Exits the painting process for the 'Run' button
    return 0 ;

    case WM_LBUTTONUP:                              // Left mouse button has been released over the 'Run' Button
        if(pExitButton->isMouseLeftButtonDown())    // Make sure that the left mouse button was clicked while over the exit button
            SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowLong(hwnd, GWL_ID), (LPARAM)hwnd);    // Send this message to the dialog's main window procedure for processing.
    return 0;

    case WM_LBUTTONDOWN:                            // The left mouse button is being held down
    {
        pExitButton->mouseLeftDown();               // Set a flag to signify that the left mouse button is being held down
        if(pRunButton->isMouseLeftButtonDown())     // If the left mouse button was previously set while over the 'Run' button but not reset before the mouse was moved over to the 'Exit' button
            pRunButton->mouseLeftUp();              // Set the flag signifying that the left mouse button is down over the 'Run' button to false
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

    switch (message)
    {

	case WM_LBUTTONDBLCLK:
        PostMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
        break;

	case WM_MOUSEMOVE:
	{
        if(pRunButton->isMouseOver() == false){
            pRunButton->mouseIsOver();
            pExitButton->mouseIsOut();
            InvalidateRgn(hwnd, NULL, FALSE);
            InvalidateRgn(GetDlgItem(GetParent(hwnd),IDEXIT), NULL, FALSE);
        }

        return (0);
	}
	break;

    case WM_CREATE:
        SetWindowPos(hwnd, NULL, 0,0,pRunButton->getBitmapWidth(), pRunButton->getBitmapHeight(), SWP_NOMOVE | SWP_NOZORDER);
        SetWindowRgn(hwnd, pRunButton->getHandleToVisibleRegion(), true);
        DeleteObject(pRunButton->getHandleToVisibleRegion());
    break;

    case WM_PAINT :
        HDC dcSkin;
        BITMAP bm;
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        dcSkin = CreateCompatibleDC(hdc);

        if(pRunButton->getbuttonState() == BUTTONSTATE_IN){
            GetObject(pRunButton->getHandleButtonInBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pRunButton->getHandleButtonInBitmap());
        }
        else if(pRunButton->getbuttonState() == BUTTONSTATE_OVER){
            GetObject(pRunButton->getHandleButtonOverBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pRunButton->getHandleButtonOverBitmap());
        }
        else{
            GetObject(pRunButton->getHandleButtonOutBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pRunButton->getHandleButtonOutBitmap());
        }

        BitBlt(hdc, 0,0,pRunButton->getBitmapWidth(),pRunButton->getBitmapHeight(), dcSkin, 0, 0, SRCCOPY);
        DeleteDC(dcSkin);
        EndPaint(hwnd, &ps);
    return 0 ;

    case WM_LBUTTONUP:
        if(pRunButton->isMouseLeftButtonDown())
            SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowLong(hwnd, GWL_ID), (LPARAM)hwnd);
    return 0;

    case WM_LBUTTONDOWN:
    {
        pRunButton->mouseLeftDown();
        if(pExitButton->isMouseLeftButtonDown())
            pExitButton->mouseLeftUp();
        InvalidateRgn(hwnd, NULL, FALSE);
        break;
    }

    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}


//--------------------------------------------------------------------------------------
// Called every time the Demo Challenge Launcher receives a message
//--------------------------------------------------------------------------------------
BOOL CALLBACK DlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hRunButtonWnd;                      // A handle to the 'Run' button
    static HWND hExitButtonWnd;                     // A handle to the 'Exit' button

    switch (message)
    {
        static HWND hRunButtonWnd;                      // A handle to the 'Run' button
        static HWND hExitButtonWnd;                     // A handle to the 'Exit' button

        case WM_INITDIALOG:
        {
            /* Select fullscreen option by default */
            HWND hCkBxFullscreen = GetDlgItem(hDlg, IDC_FULLSCREEN);
            PostMessage(hCkBxFullscreen, BM_SETCHECK,BST_CHECKED,0);


            HWND hResolutionList = GetDlgItem(hDlg, IDC_RESOLUTION);
            DWORD iDevNum	= 0;
            DWORD iModeNum = 0;
            DISPLAY_DEVICE ddi;
            DEVMODE dmi;
            TCHAR szBuffer [100];

            ZeroMemory(&ddi, sizeof(ddi));
            ddi.cb = sizeof(ddi);
            ZeroMemory(&dmi, sizeof(dmi));
            dmi.dmSize = sizeof(dmi);

            if(SetLayeredWindowAttributes != NULL)      // Make sure that this function exits
            {
                if(destroyCaption == false) {           // Make sure that the caption has not already been destroyed
                    DestroyCaption(hDlg, pDialogSkin->getBitmapWidth(), pDialogSkin->getBitmapHeight());   // Destroy any window caption that may be set
                    destroyCaption = true;              // Set a flag to ensure that this has been accomplished
                }

              SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE) | WS_EX_LAYERED); // Set the window style
              SetLayeredWindowAttributes(hDlg, colorKey, 0, LWA_COLORKEY);                           // Set the transparency color key
            }
            hRunButtonWnd = ::GetDlgItem(hDlg,IDRUN);    // Get the window handle for the 'Run' button
            hExitButtonWnd = ::GetDlgItem(hDlg,IDEXIT);  // Get the window handle for the 'Exit' button

            while(EnumDisplayDevices (NULL, iDevNum++, &ddi, 0))
            {
                while(EnumDisplaySettings (ddi.DeviceName, iModeNum++, &dmi))
                {
                    if(((ddi.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) > 0) && (dmi.dmBitsPerPel >= 32))
                    {
                        wsprintf(szBuffer, "%ix%i, %i bpp @ %iHz", dmi.dmPelsWidth,
                                  dmi.dmPelsHeight, dmi.dmBitsPerPel, dmi.dmDisplayFrequency);
                        SendMessage(hResolutionList, LB_INSERTSTRING,0,(LPARAM)szBuffer);       // p. 403 Petzold
                        ZeroMemory(&dmi, sizeof(dmi));
                        dmi.dmSize = sizeof(dmi);
                    }
                }
                ZeroMemory(&ddi, sizeof(ddi));
                ddi.cb = sizeof(ddi);
                iModeNum = 0;
            }

            SendMessage(hResolutionList, LB_SETCURSEL, 0, 0);   // p. 403 Petzold

            HWND hAspectRatioList = GetDlgItem(hDlg, IDC_ASPECT);
            SendMessage(hAspectRatioList, LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"Auto");
            SendMessage(hAspectRatioList, LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"4:3");
            SendMessage(hAspectRatioList, LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"16:9");
            SendMessage(hAspectRatioList, LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"16:10");
            SendMessage(hAspectRatioList, LB_SETCURSEL, 0, 0);

        break;
        }


        case WM_COMMAND:
        {
            switch(LOWORD (wParam))
            {
                /* User pressed the 'Launch!' button */
                case IDRUN:
                {
                    /* Check to see if fullscreen is selected and set our global fullscreen variable */
                    fullscreen = (IsDlgButtonChecked(hDlg, IDC_FULLSCREEN) == BST_CHECKED);

                    /* Grab the selected resolution */
                    HWND hResolutionList = GetDlgItem(hDlg, IDC_RESOLUTION);                // p. 404 Petzold
                    int iIndex = SendMessage(hResolutionList, LB_GETCURSEL, 0, 0);
                    int iLength = SendMessage(hResolutionList, LB_GETTEXTLEN, iIndex, 0);
                    char* szResolutionBuffer = new char[iLength+1];;
                    SendMessage(hResolutionList, LB_GETTEXT, iIndex, (LPARAM)szResolutionBuffer);

                    int iCount = 1;
                    char * pch;
                    pch = strtok(szResolutionBuffer,"x");
                    while(pch != NULL)
                    {
                        switch(iCount)
                        {
                            case 1:
                                iScreenWidth = (int)atof(pch);
                                break;
                            case 2:
                                iScreenHeight = (int)atof(pch);
                                break;
                            case 3:
                                iBitsPerPel = (int)atof(pch);
                                break;
                        }

                        pch = strtok (NULL, " ,H");
                        iCount++;
                    }

                    /* Get the selected aspect ratio */
			        HWND hAspectRatioList = GetDlgItem(hDlg, IDC_ASPECT);
                    iIndex = SendMessage(hAspectRatioList, LB_GETCURSEL, 0, 0);

                    GLfloat iWidthRatio;
                    GLfloat iHeightRatio;

                    if(iIndex == 0)
                    {
                        /* Use the native aspect ratio */
                        fAspectRatio = (GLfloat)iScreenWidth / (GLfloat)iScreenHeight;
                    }
                    else
                    {
                        /* Use the selected aspect ratio */
                        iLength = SendMessage(hAspectRatioList, LB_GETTEXTLEN, iIndex, 0);
                        char* szAspectBuffer = new char[iLength+1];
                        SendMessage(hAspectRatioList, LB_GETTEXT, iIndex, (LPARAM)szAspectBuffer);

                        iCount = 1;
                        pch = strtok(szAspectBuffer,":");

                        while(pch != NULL)
                        {
                            switch(iCount)
                            {
                                case 1:
                                    iWidthRatio = (GLfloat)atof(pch);
                                    break;
                                case 2:
                                    iHeightRatio = (GLfloat)atof(pch);
                                    break;
                            }

                            pch = strtok (NULL, ":");
                            iCount++;
                        }
                        /* Use selected aspect ratio */
                        fAspectRatio = iWidthRatio / iHeightRatio;
                    }

                    EndDialog (hDlg, 1);    // User is ready to launch the demo
                    return TRUE;
                }

                /* Handle default Win32 keyboard logic for dialog windows (ESC key pressed) */
                case IDCANCEL:
                /* User pressed the 'Exit' button */
                case IDEXIT:
                    EndDialog (hDlg, 0);    // User cancelled the dialog window
                    return TRUE;
            }
        }
        break;

        // draw our bitmap
        case WM_PAINT:
        {
            BITMAP bm;                                  // Create a bitmap structure
            PAINTSTRUCT ps;                             // Create a paint structure
            HDC hdc = BeginPaint(hDlg, &ps);         // Create a device context used for the dialog window
            HDC dcSkin = CreateCompatibleDC(hdc);       // Create a compatible memory device context to copy the color information from the bitmap to
            GetObject(pDialogSkin->getBitmapHandle(), sizeof(bm), &bm);      // Fill bitmap structure with information about the background image bitmap
            SelectObject(dcSkin, pDialogSkin->getBitmapHandle());            // Select this bitmap into the memory device context
            BitBlt(hdc, 0,0,pDialogSkin->getBitmapWidth(),pDialogSkin->getBitmapHeight(), dcSkin, 0, 0, SRCCOPY);   // Performs bit-block transfer of bitmap pixels to the memory device context
            DeleteDC(dcSkin);                           // Release the memory device context
            EndPaint(hDlg, &ps);                     // End painting of dialog window
        }

        case WM_MOUSEMOVE:                              // Mouse has been moved while over the dialog window area
        {
            if(pRunButton->isMouseOver()){              // Check to see if the mouse was previously over the 'Run' button
                pRunButton->mouseIsOut();               // Set a flag to signify that the mouse is not hovering over the 'Run' button any more

                if(pRunButton->isMouseLeftButtonDown()){// Check to see if the mouse button was previously flagged as down over the 'Run' button
                    pRunButton->mouseLeftUp();          // Set a flag to signify that the 'Run' button does not have the left mouse button clicked over it any more

                }
                InvalidateRgn(hRunButtonWnd, NULL, FALSE);  // Redraw the 'Run' button with the default state
            }
            if(pExitButton->isMouseOver()){             // Check to see if the mouse was previously over the 'Exit' button
                pExitButton->mouseIsOut();                 // Set a flag to signify that the mouse is not hovering over the 'Exit' button any more
                if(pExitButton->isMouseLeftButtonDown()){// Check to see if the mouse button was previously flagged as down over the 'Exit' button
                    pExitButton->mouseLeftUp();               // Set a flag to signify that the 'Exit' button does not have the left mouse button clicked over it any more

                }
                InvalidateRgn(hExitButtonWnd, NULL, FALSE); // Redraw the 'Exit' button with the default state
            }
        }
        break;

        // Moves the window when the user clicks anywhere not covered by a control. HTCAPTION specifies
        // that all button clicks originate in the title bar area - even when the window has no title bar.
        case WM_LBUTTONDOWN:
        {
            PostMessage(hDlg, WM_NCLBUTTONDOWN, HTCAPTION,0);
            break;
        }

        case WM_LBUTTONUP:                              // The left mouse button was released
        {
            if(pRunButton->isMouseLeftButtonDown()){    // Check to see if the mouse button was previously flagged as down over the 'Run' button
                pRunButton->mouseLeftUp();              // Set a flag to signify that the 'Run' button does not have the left mouse button clicked over it any more
                InvalidateRgn(hRunButtonWnd, NULL, FALSE);  // Redraw the 'Run' button in its default state
            }
            if(pExitButton->isMouseLeftButtonDown()){   // Check to see if the mouse button was previously flagged as down over the 'Exit' button
                pExitButton->mouseLeftUp();                  // Set a flag to signify that the 'Exit' button does not have the left mouse button clicked over it any more
                InvalidateRgn(hExitButtonWnd, NULL, FALSE); // Redraw the 'Exit' button in its default state
            }
        }

    }
    return FALSE;
}


//--------------------------------------------------------------------------------------
// Set the application's pixel format and create a rendering context
//--------------------------------------------------------------------------------------
void EnableOpenGL(HWND hWnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hWnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    HGLRC tempContext;

    /* create and enable a temporary OpenGL 2.1 rendering context (RC) */
    tempContext = wglCreateContext(*hDC);

    /*WGL functions are extensions to Win32 specifically for OpenGL (used for remembering OGL settings and commands)*/
    wglMakeCurrent(*hDC, tempContext);  // Make the OpenGL 2.1 context current and active


    int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3, // Set the MAJOR version of OpenGL to 3
        WGL_CONTEXT_MINOR_VERSION_ARB, 3, // Set the MINOR version of OpenGL to 3
        //WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, // Set our OpenGL context to be forward compatible
        0
    };

    GLenum error = glewInit();  // Enable GLEW so we can use extensions required to use shaders
    if (error != GLEW_OK){      // If GLEW fails
        MessageBox(NULL, "GLEW failed to intialize", "Error", MB_ICONERROR | MB_OK);
    }


    if (wglewIsSupported("WGL_ARB_create_context") == 1) { // If the OpenGL 3.x context creation extension is available
        *hRC = wglCreateContextAttribsARB(*hDC, NULL, attributes); // Create and OpenGL 3.x context based on the given attributes
        wglMakeCurrent(NULL, NULL); // Remove the temporary context from being active
        wglDeleteContext(tempContext); // Delete the temporary OpenGL 2.1 context
        wglMakeCurrent(*hDC, *hRC); // Make our OpenGL 3.x context current
    }
    else {
        *hRC = tempContext; // If we didn't have support for OpenGL 3.x and up, use the OpenGL 2.1 context
    }

    int glVersion[2] = {-1, -1}; // Set some default values for the version
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); // Get back the OpenGL MAJOR version we are using
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]); // Get back the OpenGL MAJOR version we are using

    std::cout << "Using OpenGL: " << glVersion[0] << "." << glVersion[1] << std::endl; // Output which version of OpenGL we are using On Windows, you won’t get a console for a Win32 Application, but a nifty trick to get console output, is to open up Command Prompt, navigate to your directory with your executable file, and use something like: “program.exe > temp.txt”

    *hRC = wglCreateContextAttribsARB(*hDC, 0, attributes);
    wglDeleteContext(tempContext);

    wglMakeCurrent(*hDC, *hRC);
}

//--------------------------------------------------------------------------------------
// Release rendering context, reset display mode, and release the device context
//--------------------------------------------------------------------------------------
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    /* Restore graphics mode to values stored in registry */
    if(fullscreen)
    {
      RestoreDisplayMode();
      ShowCursor(TRUE);
    }

    ReleaseDC(hWnd, hDC);
}

//--------------------------------------------------------------------------------------
// Initialize the OpenGL state machine. Set up projection matrix and other initial states
//--------------------------------------------------------------------------------------
void InitializeDemo()   // Setup our scene
{
    glClearColor(0.4f, 0.6f, 0.9f, 0.0f);                               // Set the clear colour based on Microsoft's CornflowerBlue (default in XNA)
    demoShader = new Shader("resources\\shaders\\basic-fixed.vert", "resources\\shaders\\basic-fixed.frag");    // Initialize our Shader object with the file names of our shader files. Shader files are located inthe same directory as the compiled application.

    fFieldOfView = 45.0F; // Set to 60.0F
    ConfigureViewport(iScreenWidth, iScreenHeight);                     // Set the viewport to the selected aspect ratio and compute the projection matrix

    viewMatrix = glm::lookAt(glm::vec3(4,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::perspective(fFieldOfView, fAspectRatio, 0.1F, 100.0F);     // Creates a perspective projection matrix
    MVP = projectionMatrix * viewMatrix * modelMatrix;

    mvpMatrixLocationInShader = glGetUniformLocation(demoShader->id(), "MVP");   // Get the location of the projection matrix in the shader

    createSquare();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
}

//--------------------------------------------------------------------------------------
// The location of your OpenGL code
//--------------------------------------------------------------------------------------
void RenderNextDemoFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    demoShader->Bind();

	//glRotatef(fTime*120, 0.2f, 1.0f, 0.2f);
	modelMatrix = glm::rotate(modelMatrix,fDeltaTime,glm::vec3(0.2f, 1.0f, 0.2f)); //rotate
	MVP = projectionMatrix * viewMatrix * modelMatrix;

    glUniformMatrix4fv(mvpMatrixLocationInShader, 1, GL_FALSE, &MVP[0][0]);

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(
       0,                   // attribute 0. No particular reason for 0, but must match the layout in the vertex shader.
       3,                   // size
       GL_FLOAT,            // type
       GL_FALSE,            // normalized?
       0,                   // stride
       (void*)0             // array buffer offset
    );


    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glVertexAttribPointer(
        1,                  // attribute 1. No particular reason for 1, but must match the layout in the vertex shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);

    demoShader->unbind();
}

void createSquare() {
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);   // Create our Vertex Array Object
    glBindVertexArray(VertexArrayID);       // Bind our Vertex Array Object so we can use it

    // Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
    // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
        1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };


    // One color for each vertex. They were generated randomly.
    static const GLfloat g_color_buffer_data[] = {
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f
    };

    glGenBuffers(1, &vertexBuffer);                 // Generate 1 buffer, put the resulting identifier in vertexBuffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);    // The following commands will talk about our 'vertexBuffer' buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);  // Give our vertices to OpenGL.

    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
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
