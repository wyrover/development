#include "StdAfx.h"
#include "DisplaySleep.h"
#include "MainWindow.h"

/* DATA *****************************************************/




/* PUBLIC METHODS **********************************************/

CMainWindow::CMainWindow(void) :
    m_hMainWnd(NULL),
    m_CmdShow(0)
{
    wcscpy_s(m_szMainWndClass, 32, L"DispSleepWndClass");
}

CMainWindow::~CMainWindow(void)
{
}

BOOL
CMainWindow::Initialize(LPCTSTR lpCaption,
                        int nCmdShow)
{
    WNDCLASSEXW wc = {0};

    /* Store the show window value */
    m_CmdShow = nCmdShow;

    /* Setup the window class struct */
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = g_hInstance;
    wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCEW(IDI_DISPLAYSLEEP));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = m_szMainWndClass;
    wc.hIconSm = (HICON)LoadImage(g_hInstance,
                                  MAKEINTRESOURCE(IDI_SMALL),
                                  IMAGE_ICON,
                                  16,
                                  16,
                                  LR_SHARED);

    /* Register the window */
    if (RegisterClassExW(&wc))
    {
        /* Create the main window and store the info pointer */
        m_hMainWnd = CreateWindowExW(WS_EX_WINDOWEDGE,
                                     m_szMainWndClass,
                                     lpCaption,
                                     WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     600,
                                     450,
                                     NULL,
                                     NULL,
                                     g_hInstance,
                                     this);
        if (m_hMainWnd)
        {

        }
    }

    /* Return creation result */
    return !!(m_hMainWnd);
}

VOID
CMainWindow::Uninitialize()
{
    /* Unregister the window class */
    UnregisterClassW(m_szMainWndClass, g_hInstance);
}

INT
CMainWindow::Run()
{
    MSG Msg;

    /* Pump the message queue */
    while (GetMessageW(&Msg, NULL, 0, 0 ) != 0)
    {
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }

    return 0;
}


/* PRIVATE METHODS **********************************************/

LRESULT
CMainWindow::OnCreate(HWND hwnd)
{
    LRESULT RetCode;

    /* Assume failure */
    RetCode = -1;

    /* Store the window handle */
    m_hMainWnd = hwnd;


    return 0;
}

LRESULT
CMainWindow::OnSize()
{
    return 0;
}

LRESULT
CMainWindow::OnNotify(LPARAM lParam)
{
    LPNMHDR NmHdr = (LPNMHDR)lParam;

    switch (NmHdr->code)
    {
        case TVN_DELETEITEMW:
        {
            LPNMTREEVIEW NmTreeView = (LPNMTREEVIEW)lParam;

            NmTreeView->action = NmTreeView->action;

            break;
        }
    }

    return 0;
}


LRESULT
CMainWindow::OnCommand(WPARAM wParam,
                       LPARAM lParam)
{
    LRESULT RetCode = 0;
    WORD Msg;

    /* Get the message */
    Msg = LOWORD(wParam);

    switch (Msg)
    {


        //case IDC_ABOUT:
        //{
        //    /* Blow my own trumpet */
        //    MessageBoxW(m_hMainWnd,
        //                L"ReactOS Device Manager\r\nCopyright Ged Murphy 2011",
        //                L"About",
        //                MB_OK);

        //    /* Set focus back to the treeview */
        //    m_DeviceView->SetFocus();
        //    break;
        //}

        //case IDC_EXIT:
        //{
        //    /* Post a close message to the window */
        //    PostMessageW(m_hMainWnd,
        //                 WM_CLOSE,
        //                 0,
        //                 0);
        //    break;
        //}

        default:
            break;
    }

    return RetCode;
}

LRESULT
CMainWindow::OnDestroy()
{

    /* Clear the user data pointer */
    SetWindowLongPtr(m_hMainWnd, GWLP_USERDATA, 0);

    /* Break the message loop */
    PostQuitMessage(0);

    return 0;
}

LRESULT CALLBACK
CMainWindow::MainWndProc(HWND hwnd,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam)
{
    CMainWindow *pThis;
    LRESULT RetCode = 0;

    /* Get the object pointer from window context */
    pThis = (CMainWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    /* Check for an invalid pointer */
    if (pThis == NULL)
    {
        /* Check that this isn't a create message */
        if (msg != WM_CREATE)
        {
            /* Don't handle null info pointer */
            goto HandleDefaultMessage;
        }
    }

    switch(msg)
    {
        case WM_CREATE:
        {
            /* Get the object pointer from the create param */
            pThis = (CMainWindow *)((LPCREATESTRUCT)lParam)->lpCreateParams;

            /* Store the info pointer in the window's global user data */
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

            /* Call the create handler */
            RetCode = pThis->OnCreate(hwnd);
            break;
        }

        case WM_SIZE:
        {
            RetCode = pThis->OnSize();
            break;
        }

        case WM_NOTIFY:
        {
            /* Handle the notify message */
            RetCode = pThis->OnNotify(lParam);
            break;
        }

        case WM_COMMAND:
        {
            /* Handle the command message */
            RetCode = pThis->OnCommand(wParam, lParam);

            /* Hand it off to the default message handler */
            goto HandleDefaultMessage;
        }

        case WM_CLOSE:
        {
            /* Destroy the main window */
            DestroyWindow(hwnd);
        }
        break;

        case WM_DESTROY:
        {
            /* Call the destroy handler */
            RetCode = pThis->OnDestroy();
            break;
        }

        default:
        {
HandleDefaultMessage:
            RetCode = DefWindowProc(hwnd, msg, wParam, lParam);
            break;
        }
    }

    return RetCode;
}
