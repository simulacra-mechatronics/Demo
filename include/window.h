#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED




LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

void ConfigureViewport(GLsizei, GLsizei);
void ResizeViewport(GLsizei, GLsizei);
void ChangeDisplayMode();
void RestoreDisplayMode();
void EnableOpenGL(HWND hWnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);


#endif // WINDOW_H_INCLUDED
