 /*-------------\
|   main.cpp     | OpenGL 3.3 Demo Framework
 \*-------------/ ------------------------------------------------------------------------------------------*\
  Version 3.3 - 06/29/2015

  Author: Godvalve
  GitHub URL: https://github.com/godvalve/Demo

  Description: A basic OpenGL 3.3 framework with non-rectangular dialog launcher
\*----------------------------------------------------------------------------------------------------------*/

#include "launcher.h"
#include "openGLWindow.h"

using std::string;

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){

    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;

    BOOL bQuit = FALSE;
    fullscreen = TRUE;

    // Only shows on the console window if in debug build. Use release mode or change build options to remove console window.
    std::cout << "OpenGL Demo Framework version 3.3 (June 05, 2015)\n";


    if(createLauncherWindow(hInstance) == false){
        return 0;
    }

    hWnd = createOpenGLWindow(hInstance);

    if(fullscreen){
      ShowCursor(FALSE);
      ChangeDisplayMode();
    }

    EnableOpenGL(hWnd, &hDC, &hRC);                     // Enable OpenGL 3.3 for this window
    ShowWindow(hWnd, nCmdShow);
    InitializeDemo();

    while (!bQuit){                                     // Main program loop
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){  // Check for messages
            if (msg.message == WM_QUIT){                // Handle QM_Quit message
                bQuit = TRUE;
            }
            else{
                TranslateMessage(&msg);                 // Translate messages
                DispatchMessage(&msg);                  // Dispatch messages
            }
        }
        else{
			RenderNextDemoFrame();                      //Render the next frame in our demo
            SwapBuffers(hDC);                           //Change logical order of back and front buffer
        }
    }

    DisableOpenGL(hWnd, hDC, hRC);                      // Shut down OpenGL
    DestroyWindow(hWnd);                                // Destroy the window

    return msg.wParam;
}
