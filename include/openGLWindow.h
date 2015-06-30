#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <windows.h>
#include <iostream>
#include <stdio.h>      // Used for fprintf and stderr
#include <GL/glew.h>    // Used for OpenGL types like GLuint
#include <GL/wglew.h>   // Windows specific OpenGL Extension Wrangler library definitions
#include <GL/glext.h>   // OpenGL extension definitions (OpenGL Working Group)

#include "demoMain.h"
#include "../resource.h"


extern BOOL fullscreen;
extern int iScreenWidth, iScreenHeight;
extern int iBitsPerPel;
extern float fAspectRatio;
extern float fFieldOfView;
extern float fTime, fDeltaTime;
static char szAppName[] = "-= Demo Challenge Framework =-";

HWND createOpenGLWindow(HINSTANCE hInstance);
void ConfigureViewport(GLsizei, GLsizei);
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void ResizeViewport(GLsizei, GLsizei);
void ChangeDisplayMode();
void RestoreDisplayMode();
void EnableOpenGL(HWND hWnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);




#endif // WINDOW_H_INCLUDED
