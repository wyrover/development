#include "stdafx.h"
#include "Resource.h"
#include "devmgmt.h"
#include "DisplayTraceProvidersDlg.h"
#include "CreateTraceSessionDlg.h"


CCreateTraceSessionDlg::CCreateTraceSessionDlg(void) :
    m_NewTraceSession(NULL),
    m_hAddProvListView(NULL),
    m_hEditProvListView(NULL)
{
}

CCreateTraceSessionDlg::~CCreateTraceSessionDlg(void)
{
}

BOOL
CCreateTraceSessionDlg::ShowDialog(HWND hParent)
{
    return (DialogBoxParamW(g_hInstance,
                            MAKEINTRESOURCEW(IDD_CREATE_TRACE_SESSION),
                            hParent,
                            WndProc,
                            (LPARAM)this) == IDOK);
}

BOOL CCreateTraceSessionDlg::AddProvidersToList()
{
    CDisplayTraceProvidersDlg TraceProvDlg;
    CSelectedTrace *SelectedTrace;

    BOOL bSuccess = TraceProvDlg.ShowDialog(NULL);
    if (bSuccess == FALSE) return FALSE;
            
    int count = TraceProvDlg.GetSelectedCount();
    if (count == 0) return FALSE;

    for (int i = 0; i < count; i++)
    {
        SelectedTrace = TraceProvDlg.GetSelectedItem(i);
        if (SelectedTrace)
        {
            CTraceProvider *TraceProvider = new CTraceProvider(SelectedTrace->SessionName,
                                                               SelectedTrace->TraceGuid);

            LVITEM lvi = {0};
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.pszText = SelectedTrace->SessionName;
            lvi.cchTextMax = wcslen(lvi.pszText) + 1;
            lvi.lParam = (LPARAM)TraceProvider;

            ListView_InsertItem(m_hAddProvListView, &lvi);
        }
    }

    return bSuccess;
}

BOOL CCreateTraceSessionDlg::AddProvidersToTraceSession()
{
    for (int item = ListView_GetNextItem(m_hAddProvListView, -1, LVNI_SELECTED);
         item >= 0;
         item = ListView_GetNextItem(m_hAddProvListView, item, LVNI_SELECTED))
    {
        LVITEM lvi = {0};

        lvi.iItem = item;
        lvi.mask = LVIF_PARAM;

        if (ListView_GetItem(m_hAddProvListView, &lvi))
        {
            m_NewTraceSession->AddProvider((CTraceProvider *)lvi.lParam);
        }
    }

    return TRUE;
}

bool CCreateTraceSessionDlg::CreateTraceSession()
{
    return true;
}

BOOL CCreateTraceSessionDlg::InitializeDialog(HWND hwnd)
{
    // Store the info pointer in the window's global user data
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

    // Get handles to the list views
    m_hAddProvListView = GetDlgItem(hwnd, IDC_ADDPROV_LIST);
    m_hEditProvListView = GetDlgItem(hwnd, IDC_EDITPROV_LIST);

    // Create an empty column for the add list
    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = L"";
    lvc.cchTextMax = 1;
    lvc.cx = 300;
    ListView_InsertColumn(m_hAddProvListView, 0, &lvc);
    ListView_SetExtendedListViewStyle(m_hAddProvListView, LVS_EX_FULLROWSELECT);

    // Create the columns for the trace properties view
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = L"Property";
    lvc.cchTextMax = 1;
    lvc.cx = 150;
    ListView_InsertColumn(m_hEditProvListView, 0, &lvc);

    lvc.pszText = L"Value";
    lvc.cx = 174;
    ListView_InsertColumn(m_hEditProvListView, 0, &lvc);

    ListView_SetExtendedListViewStyle(m_hEditProvListView, LVS_EX_FULLROWSELECT);

    return TRUE;
}

BOOL CALLBACK
CCreateTraceSessionDlg::WndProc(HWND hwnd,
                                UINT msg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    CCreateTraceSessionDlg *This = NULL;

    /* Get the object pointer from window context */
    This = (CCreateTraceSessionDlg *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    /* Don't handle null info pointer */
    if (This == NULL && (msg != WM_INITDIALOG))
        return FALSE;

   switch (msg)
    { 
        case WM_INITDIALOG:

            This = (CCreateTraceSessionDlg *)lParam;
            This->InitializeDialog(hwnd);
            return TRUE;

        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDC_ADDPROV_BTN:
                    This->AddProvidersToList();
                    break;

                case IDOK:
                
                    // create session
                    This->CreateTraceSession();
                
                    // fall through
                case IDCANCEL:
                    EndDialog(hwnd, wParam); 
                    return TRUE; 
            } 

        //case WM_NOTIFY:
        //    switch (((LPNMHDR)lParam)->code)
        //    {
        //        case TVN_SELCHANGEDW:
        //        {
        //            NMTVITEMCHANGE *pnm;
        //            pnm = (NMTVITEMCHANGE *)lParam;
        //            return TRUE;
        //        }
        //    }
    } 

    return FALSE; 
}
