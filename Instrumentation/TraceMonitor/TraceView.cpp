#include "StdAfx.h"
#include "devmgmt.h"
#include "TraceView.h"


/* PUBLIC METHODS *************************************/

CTraceView::CTraceView(void) :
    m_hParent(NULL),
    m_hListView(NULL),
    m_hPropertyDialog(NULL),
    m_hShortcutMenu(NULL)
{
}

CTraceView::~CTraceView(void)
{
}

BOOL
CTraceView::Initialize(_In_ HWND hParent)
{
    m_hParent = hParent;
    return true;
}

BOOL
CTraceView::Uninitialize()
{
    return TRUE;
}

bool CTraceView::CreateNew()
{

    MDICREATESTRUCTW MdiCreate;
    MdiCreate.szClass = WC_LISTVIEW;
    MdiCreate.szTitle = L"Test";
    MdiCreate.hOwner = g_hInstance;
    MdiCreate.x = CW_USEDEFAULT;
    MdiCreate.y = CW_USEDEFAULT;
    MdiCreate.cx = CW_USEDEFAULT;
    MdiCreate.cy = CW_USEDEFAULT;
    MdiCreate.style = WS_CHILD | LVS_REPORT | LVS_EDITLABELS;
    MdiCreate.lParam = 0;

    HWND hwndChild = (HWND)SendMessage(m_hParent,
                                        WM_MDICREATE,
                                        0,
                                        (LPARAM)(LPMDICREATESTRUCT)&MdiCreate);

    return !!(hwndChild);
}

VOID
CTraceView::Size(INT x,
                  INT y,
                  INT cx,
                  INT cy)
{
    /* Resize the listview */
    SetWindowPos(m_hListView,
                 NULL,
                 x,
                 y,
                 cx,
                 cy,
                 SWP_NOZORDER);
}


VOID
CTraceView::Refresh()
{
    HANDLE hThread;

    hThread = (HANDLE)_beginthreadex(NULL,
                                     0,
                                     &ListDevicesThread,
                                     this,
                                     0,
                                     NULL);

    if (hThread) CloseHandle(hThread);
}

VOID
CTraceView::DisplayPropertySheet()
{
}

VOID
CTraceView::SetFocus()
{
}


/* PRIVATE METHODS ********************************************/

unsigned int __stdcall CTraceView::ListDevicesThread(void *Param)
{
    CTraceView *This = (CTraceView *)Param;

    return 0;
}

