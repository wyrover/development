#include "StdAfx.h"
#include "devmgmt.h"
#include "MainWindow.h"
#include "CreateTraceSessionDlg.h"
#include "AboutDlg.h"

/* DATA *****************************************************/

/* menu hints */
static const MENU_HINT MainMenuHintTable[] =
{
    /* Action Menu */
    {IDC_REFRESH,  IDS_HINT_REFRESH},

    {IDC_ABOUT,    IDS_HINT_ABOUT},

    //{IDC_DEVBYTYPE, IDS_HINT_DEV_BY_TYPE}

};

/* system menu hints */
static const MENU_HINT SystemMenuHintTable[] =
{
    {SC_RESTORE,    IDS_HINT_SYS_RESTORE},
    {SC_MOVE,       IDS_HINT_SYS_MOVE},
    {SC_SIZE,       IDS_HINT_SYS_SIZE},
    {SC_MINIMIZE,   IDS_HINT_SYS_MINIMIZE},
    {SC_MAXIMIZE,   IDS_HINT_SYS_MAXIMIZE},
    {SC_CLOSE,      IDS_HINT_SYS_CLOSE},
};


/* PUBLIC METHODS **********************************************/

CMainWindow::CMainWindow(void) :
    m_ToolbarhImageList(NULL),
    m_hMainWnd(NULL),
    m_hStatusBar(NULL),
    m_hToolBar(NULL),
    m_CmdShow(0)
{
    m_szMainWndClass = L"TrcMonWndClass";
}

CMainWindow::~CMainWindow(void)
{
    /* Destroy any previous list */
    if (m_ToolbarhImageList) ImageList_Destroy(m_ToolbarhImageList);
}

BOOL
CMainWindow::Initialize(LPCTSTR lpCaption,
                        int nCmdShow)
{
    CAtlString szCaption;
    WNDCLASSEXW wc = {0};

    /* Store the show window value */
    m_CmdShow = nCmdShow;

    /* Setup the window class struct */
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = g_hInstance;
    wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MAINMENU);
    wc.lpszClassName = m_szMainWndClass;
    wc.hIconSm = (HICON)LoadImage(g_hInstance,
                                  MAKEINTRESOURCE(IDI_MAIN_ICON),
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
            m_hMenu = GetMenu(m_hMainWnd);
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


BOOL
CMainWindow::MainWndMenuHint(WORD CmdId,
                             const MENU_HINT *HintArray,
                             DWORD HintsCount,
                             UINT DefHintId)
{
    BOOL Found = FALSE;
    const MENU_HINT *LastHint;
    UINT HintId = DefHintId;

    LastHint = HintArray + HintsCount;
    while (HintArray != LastHint)
    {
        if (HintArray->CmdId == CmdId)
        {
            HintId = HintArray->HintId;
            Found = TRUE;
            break;
        }
        HintArray++;
    }

    StatusBarLoadString(m_hStatusBar,
                        SB_SIMPLEID,
                        g_hInstance,
                        HintId);

    return Found;
}

BOOL
CMainWindow::CreateToolBar()
{
    HIMAGELIST hImageList;
    HBITMAP hBitmap;
    UINT StartResource, EndResource;
    INT NumButtons;
    BOOL bRet = FALSE;

    static TBBUTTON ToolbarButtons [] =
    {
        {TBICON_REFRESH, IDC_REFRESH, TBSTATE_ENABLED,       BTNS_BUTTON, {0}, 0, 0},

        /* Add a seperator: First item for a seperator is its width in pixels */
        {15, 0, TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
    };

    /* Calculate the number of buttons */
    NumButtons = sizeof(ToolbarButtons) / sizeof(ToolbarButtons[0]);

    /* Create the toolbar window */
    m_hToolBar = CreateWindowExW(0,
                                 TOOLBARCLASSNAME,
                                 NULL,
                                 WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
                                 0, 0, 0, 0,
                                 m_hMainWnd,
                                 (HMENU)IDC_TOOLBAR,
                                 g_hInstance,
                                 NULL);
    if (m_hToolBar == NULL)
        return FALSE;

    /* Don't show clipped buttons */
    SendMessageW(m_hToolBar,
                    TB_SETEXTENDEDSTYLE,
                    0,
                    TBSTYLE_EX_HIDECLIPPEDBUTTONS);

    /* Set the struct size, the toobar needs this... */
    SendMessageW(m_hToolBar,
                    TB_BUTTONSTRUCTSIZE,
                    sizeof(ToolbarButtons[0]),
                    0);

    /* Create the toolbar icon image list */
    m_ToolbarhImageList = ImageList_Create(16,
                                           16,
                                           ILC_MASK | ILC_COLOR24,
                                           NumButtons,
                                           0);
    if (m_ToolbarhImageList == NULL)
        return FALSE;

    /* Set the index endpoints */
    StartResource = IDB_REFRESH;
    EndResource = IDB_REFRESH;

    /* Add all icons to the image list */
    for (UINT i = StartResource; i <= EndResource; i++)
    {
        /* Load the image resource */
        hBitmap = (HBITMAP)LoadImage(g_hInstance,
                                        MAKEINTRESOURCE(i),
                                        IMAGE_BITMAP,
                                        16,
                                        16,
                                        LR_LOADTRANSPARENT);
        if (hBitmap)
        {
            /* Add it to the image list */
            ImageList_AddMasked(m_ToolbarhImageList,
                                hBitmap,
                                RGB(255, 0, 128));

            /* Delete the bitmap */
            DeleteObject(hBitmap);
        }
    }

    /* Set the new image list */
    hImageList = (HIMAGELIST)SendMessageW(m_hToolBar,
                                          TB_SETIMAGELIST,
                                          0,
                                          (LPARAM)m_ToolbarhImageList);

    /* Destroy any previous list */
    if (hImageList) ImageList_Destroy(hImageList);

    /* Add the buttons */
    bRet = (BOOL)SendMessageW(m_hToolBar,
                                TB_ADDBUTTONS,
                                NumButtons,
                                (LPARAM)ToolbarButtons);

    return bRet;
}

BOOL
CMainWindow::CreateStatusBar()
{
    INT StatWidths[] = {110, -1}; /* widths of status bar */
    BOOL bRet = FALSE;

    /* Create the status bar */
    m_hStatusBar = CreateWindowExW(0,
                                   STATUSCLASSNAME,
                                   NULL,
                                   WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                                   0, 0, 0, 0,
                                   m_hMainWnd,
                                   (HMENU)IDC_STATUSBAR,
                                   g_hInstance,
                                   NULL);
    if (m_hStatusBar)
    {
        /* Set the width */
        bRet = (BOOL)SendMessageW(m_hStatusBar,
                                  SB_SETPARTS,
                                  sizeof(StatWidths) / sizeof(INT),
                                  (LPARAM)StatWidths);
    }

    return bRet;
}

BOOL
CMainWindow::StatusBarLoadString(IN HWND hStatusBar,
                                 IN INT PartId,
                                 IN HINSTANCE hInstance,
                                 IN UINT uID)
{
    CAtlString szMessage;
    BOOL bRet = FALSE;

    /* Load the string */
    if (szMessage.LoadStringW(hInstance, uID))
    {
        /* Send the message to the status bar */
        bRet = (BOOL)SendMessageW(hStatusBar,
                                  SB_SETTEXT,
                                  (WPARAM)PartId,
                                  (LPARAM)szMessage.GetBuffer());
    }

    return bRet;
}

LRESULT
CMainWindow::OnCreate(HWND hwnd)
{
    LRESULT RetCode;

    /* Assume failure */
    RetCode = -1;

    /* Store the window handle */
    m_hMainWnd = hwnd;

    /* Create the toolbar */
    if (CreateToolBar() && CreateStatusBar())
    {
        /* Create the device view object */
        m_TraceView = new CTraceView();

        /* Initialize it */
        if (m_TraceView->Initialize(hwnd))
        {
            /* Display the window according to the user request */
            ShowWindow(hwnd, m_CmdShow);

            /* Set as handled */
            RetCode = 0;
        }
    }

    return RetCode;
}

LRESULT
CMainWindow::OnSize()
{
    RECT rcClient, rcTool, rcStatus;
    INT lvHeight, iToolHeight, iStatusHeight;

    /* Autosize the toolbar */
    SendMessage(m_hToolBar, TB_AUTOSIZE, 0, 0);

    /* Get the toolbar rect and save the height */
    GetWindowRect(m_hToolBar, &rcTool);
    iToolHeight = rcTool.bottom - rcTool.top;

    /* Resize the status bar */
    SendMessage(m_hStatusBar, WM_SIZE, 0, 0);

    /* Get the statusbar rect and save the height */
    GetWindowRect(m_hStatusBar, &rcStatus);
    iStatusHeight = rcStatus.bottom - rcStatus.top;

    /* Get the full client rect */
    GetClientRect(m_hMainWnd, &rcClient);

    /* Calculate the remaining height for the treeview */
    lvHeight = rcClient.bottom - iToolHeight - iStatusHeight;

    /* Resize the device view */
    m_TraceView->Size(0,
        iToolHeight,
        rcClient.right,
        lvHeight);



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
CMainWindow::OnContext(LPARAM lParam)
{
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
        case IDC_NEW_TRACESESSION:
        {
            CCreateTraceSessionDlg TraceSessionDlg;
            if (TraceSessionDlg.ShowDialog(m_hMainWnd))
            {
                m_TraceSession = TraceSessionDlg.GetNewTraceSession();
            }
            break;
        }

        case IDC_PROP:
        {
            /* Display the device property sheet */
           // m_TraceView->DisplayPropertySheet();
            break;
        }

        case IDC_REFRESH:
        {
            /* Refresh the device list */
            //m_TraceView->Refresh();
            break;
        }

        case IDC_ABOUT:
        {
            /* Blow my own trumpet */
            CAboutDlg AboutDialog;

            AboutDialog.ShowDialog(m_hMainWnd);

            /* Set focus back to the treeview */
            //m_DeviceView->SetFocus();
            break;
        }

        default:
            break;
    }

    return RetCode;
}

LRESULT
CMainWindow::OnDestroy()
{
    /* Uninitialize the device view */
    //m_TraceView->Uninitialize();

    /* Kill the object */
    //delete m_TraceView;
    //m_TraceView = NULL;

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

        case WM_CONTEXTMENU:
        {
            /* Handle creating the context menu */
            RetCode = pThis->OnContext(lParam);
            break;
        }

        case WM_MENUSELECT:
        {
            if (pThis->m_hStatusBar != NULL)
            {
                if (!pThis->MainWndMenuHint(LOWORD(wParam),
                                     MainMenuHintTable,
                                     sizeof(MainMenuHintTable) / sizeof(MainMenuHintTable[0]),
                                     IDS_HINT_BLANK))
                {
                    pThis->MainWndMenuHint(LOWORD(wParam),
                                    SystemMenuHintTable,
                                    sizeof(SystemMenuHintTable) / sizeof(SystemMenuHintTable[0]),
                                    IDS_HINT_BLANK);
                }
            }

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
            RetCode = DefWindowProcW(hwnd, msg, wParam, lParam);
            break;
        }
    }

    return RetCode;
}
