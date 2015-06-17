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
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002

#define bitmapHeight            463
#define bitmapWidth             738

#define buttonbitmapHeight            61
#define buttonbitmapWidth             100

#define g_ColourKey             0xFF00FF // 0,0,255(pink) in hex RGB

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

    /* Display the Demo Challenge Launcher to get user's preferences for fullscreen and resolution */
    if(DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DemoLauncher), HWND_DESKTOP, DlgProc, 0) == FALSE)
    {
        return 0;
    }

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

//--------------------------------------------------------------------------------------
// Called every time the Demo Challenge Launcher receives a message
//--------------------------------------------------------------------------------------
BOOL CALLBACK DlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    // The struct that WM_DRAWITEM needs:
	static DRAWITEMSTRUCT* pdis;

    switch (message)
    {
        case WM_DRAWITEM:
        {
			// The DRAWITEMSTRUCT struct contains all there is to know
			// about the owner draw control and what there is to do:
			pdis = (DRAWITEMSTRUCT*) lParam;
			// (winuser.h) Maybe you also want to account for pdis->CtlType (ODT_MENU, ODT_LISTBOX, ODT_COMBOBOX, ODT_BUTTON, ODT_STATIC)
			switch(pdis->CtlID)
			{
				case IDC_LaunchButton:
                {


                    BITMAP bitmap;

                    HDC hdcMem = CreateCompatibleDC (pdis->hDC) ;
                    SelectObject (hdcMem, hSkinLaunchBtnBmp) ;

                    BITMAP bm ;
                    GetObject (hSkinLaunchBtnBmp, sizeof(BITMAP), &bitmap) ;

                    BitBlt (pdis->hDC, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

                    break;
                }
				// Other case labels if any...
				default:
					break;
			}
			return(TRUE);
        }
        case WM_CREATE:
        {
            HINSTANCE hInst = ((LPCREATESTRUCT) lParam)->hInstance;
            hLaunchButton = CreateWindowEx(0, "BUTTON", "RED", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 10, 10, 100, 50, hDlg, (HMENU)IDC_LaunchButton, hInst, NULL);
            //hLaunchButton = CreateWindow("button", "Exit", BS_PUSHBUTTON|WS_CHILD|WS_VISIBLE, 250, 30, 100, 20, hDlg, (HMENU)IDC_LaunchButton, (HINSTANCE)GetWindowLong(hDlg, GWL_HINSTANCE), NULL);
            if(hLaunchButton == NULL)
                std::cerr << "error creating button window";

            if(SetLayeredWindowAttributes != NULL)
            {
                if(j < 1) {
                    DestroyCaption(hLaunchButton,buttonbitmapWidth,buttonbitmapHeight);
                    j++;
                }

                SetWindowLong(hLaunchButton, GWL_EXSTYLE, GetWindowLong(hLaunchButton, GWL_EXSTYLE) | WS_EX_LAYERED);
                SetLayeredWindowAttributes(hLaunchButton, g_ColourKey, 0, LWA_COLORKEY);
            }
            break;
        }


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

            if(SetLayeredWindowAttributes != NULL)
            {
                if(i < 1) {
                    DestroyCaption(hDlg,bitmapWidth,bitmapHeight);
                    i++;
                }

              SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
              SetLayeredWindowAttributes(hDlg, g_ColourKey, 0, LWA_COLORKEY);
            }

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
                case IDLAUNCH:
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
            BITMAP bm;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hDlg, &ps);
            HDC dcSkin = CreateCompatibleDC(hdc);
            GetObject(hSkinMBmp, sizeof(bm), &bm);
            SelectObject(dcSkin, hSkinMBmp);
            BitBlt(hdc, 0,0,bitmapWidth,bitmapHeight, dcSkin, 0, 0, SRCCOPY);
            DeleteDC(dcSkin);
            EndPaint(hDlg, &ps);
            break;
        }

        /* Moves the window when the user clicks anywhere not covered by a control. HTCAPTION specifies */
        /* that all button clicks originate in the title bar area - even when the window has no title bar. */
        case WM_LBUTTONDOWN:
        {
            PostMessage(hDlg, WM_NCLBUTTONDOWN, HTCAPTION,0);
            break;
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
    demoShader = new Shader("basic-fixed.vert", "basic-fixed.frag");    // Initialize our Shader object with the file names of our shader files. Shader files are located inthe same directory as the compiled application.

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
