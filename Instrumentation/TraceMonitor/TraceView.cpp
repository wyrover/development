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
    LVCOLUMN lvc = { 0 };
    TCHAR szTemp[256];

    m_hListView = CreateWindowEx(WS_EX_CLIENTEDGE,
        WC_LISTVIEW,
        NULL,
        WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER |
        LBS_NOTIFY | LVS_SORTASCENDING | LBS_NOREDRAW,
        0, 0, 0, 0,
        hParent,
        (HMENU)NULL,//IDC_SERVLIST,
        g_hInstance,
        NULL);
    if (m_hListView == NULL) return FALSE;

    (void)ListView_SetExtendedListViewStyle(m_hListView,
        LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);/*LVS_EX_GRIDLINES |*/

    lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;

    /* Add columns to the list-view */
    /* name */
    lvc.iSubItem = 0;// LVNAME;
    lvc.cx = 150;
    //LoadString(hInstance,
    //    IDS_FIRSTCOLUMN,
    //    szTemp,
    //    sizeof(szTemp) / sizeof(TCHAR));
    lvc.pszText = L"Name";
    (void)ListView_InsertColumn(m_hListView,
        0,
        &lvc);

    /* description */
    lvc.iSubItem = 1;
    lvc.cx = 240;
    //LoadString(hInstance,
    //    IDS_SECONDCOLUMN,
    //    szTemp,
    //    sizeof(szTemp) / sizeof(TCHAR));
    lvc.pszText = L"Second";
    (void)ListView_InsertColumn(m_hListView,
        1,
        &lvc);

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

