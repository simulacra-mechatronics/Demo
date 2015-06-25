#include "window.h"

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
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, // Set our OpenGL context to be forward compatible
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
