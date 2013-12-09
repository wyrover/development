#include "StdAfx.h"
#include "devmgmt.h"
#include "TraceView.h"


/* PUBLIC METHODS *************************************/

CTraceView::CTraceView(HWND hMainWnd) :
    m_hMainWnd(hMainWnd),
    m_hListView(NULL),
    m_hPropertyDialog(NULL),
    m_hShortcutMenu(NULL)
{
}

CTraceView::~CTraceView(void)
{
}

BOOL
CTraceView::Initialize()
{
    BOOL bSuccess;

    /* Create the main treeview */
    m_hListView = CreateWindowExW(WS_EX_CLIENTEDGE,
                                  WC_LISTVIEW,
                                  NULL,
                                  WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
                                  0, 0, 0, 0,
                                  m_hMainWnd,
                                  (HMENU)IDC_TRACEVIEW,
                                  g_hInstance,
                                  NULL);
    if (m_hListView)
    {
        /* Set the image list against the treeview */
        (VOID)ListView_SetImageList(m_hListView,
                                    m_ImageList,
                                    TVSIL_NORMAL);
    }

    return !!(m_hListView);
}

BOOL
CTraceView::Uninitialize()
{
    return TRUE;
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

