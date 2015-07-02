#include "launcher.h"

CustomSkinnedButton* pRunButton;
CustomSkinnedButton* pExitButton;


bool createLauncherWindow(HINSTANCE hInstance){
    bool returnValue = true;
    // import function to make windows transparent
    HMODULE hUser32 = GetModuleHandle(("USER32.DLL"));
    SetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    if(SetLayeredWindowAttributes == NULL)
        MessageBox(0, "Error, cannot load window transparency, REASON: Could not load User32.DLL", "Error!", MB_ICONSTOP | MB_OK);

    // Load our bitmap
    pDialogSkin = new BitmapSkin(hInstance, IDB_LauncherBackground);

    createButtons(hInstance);
    registerButtonWinClasses(hInstance);

    /* Display the Demo Challenge Launcher to get user's preferences for fullscreen and resolution */
    if(DialogBoxParam(hInstance, MAKEINTRESOURCE(DLG_MAIN), HWND_DESKTOP, DlgProc, 0) == FALSE)
    {
        returnValue = false;
    }

    deleteButtons();

    return returnValue;
}

void createButtons(HINSTANCE hInstance){
    pRunButton = new CustomSkinnedButton(hInstance, IDB_RunOut, IDB_RunOver, IDB_RunIn, colorKey);
    pExitButton = new CustomSkinnedButton(hInstance, IDB_ExitOut, IDB_ExitOver, IDB_ExitIn, colorKey);
}

void deleteButtons(){
    delete pRunButton;
    delete pExitButton;
    delete pDialogSkin;
}

void registerButtonWinClasses(HINSTANCE hInstance){
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
    static HBRUSH hbrBackground = NULL;

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

                case IDCANCEL:              // Handle default Win32 keyboard logic for dialog windows (ESC key pressed)

                case IDEXIT:                // 'Exit' key pressed
                    EndDialog (hDlg, 0);    // User cancelled the dialog window
                    return TRUE;
            }
        }
        break;


        case WM_CTLCOLORSTATIC:
            if ((HWND)lParam == GetDlgItem(hDlg, IDT_VERSION) || (HWND)lParam == GetDlgItem(hDlg, IDT_BUILD) || (HWND)lParam == GetDlgItem(hDlg, IDT_VERNUM) || (HWND)lParam == GetDlgItem(hDlg, IDT_BUILDNUM))
            {
                    SetBkMode((HDC)wParam,TRANSPARENT);
                    SetTextColor((HDC)wParam, RGB(85,103,126));            // Set the colour of the text
                    SetBkMode((HDC)wParam, TRANSPARENT);
                    hbrBackground = CreateSolidBrush(RGB(7, 17, 32));
                    return (LONG)hbrBackground;
            }
            break;


        case WM_PAINT:
        {
            BITMAP bm;                                  // Create a bitmap structure
            PAINTSTRUCT ps;                             // Create a paint structure
            HDC hdc = BeginPaint(hDlg, &ps);            // Create a device context used for the dialog window
            HDC dcSkin = CreateCompatibleDC(hdc);       // Create a compatible memory device context to copy the color information from the bitmap to
            GetObject(pDialogSkin->getBitmapHandle(), sizeof(bm), &bm);      // Fill bitmap structure with information about the background image bitmap
            SelectObject(dcSkin, pDialogSkin->getBitmapHandle());            // Select this bitmap into the memory device context
            BitBlt(hdc, 0,0,pDialogSkin->getBitmapWidth(),pDialogSkin->getBitmapHeight(), dcSkin, 0, 0, SRCCOPY);   // Performs bit-block transfer of bitmap pixels to the memory device context
            DeleteDC(dcSkin);                           // Release the memory device context
            EndPaint(hDlg, &ps);                        // End painting of dialog window
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



// Destroy our windows caption
void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight)
{
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    dwStyle &= ~(WS_CAPTION|WS_SIZEBOX);

    SetWindowLong(hwnd, GWL_STYLE, dwStyle);
    InvalidateRect(hwnd, NULL, true);
    SetWindowPos(hwnd, NULL, 0,0,windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
}
