#include "stdafx.h"
#include "Resource.h"
#include "devmgmt.h"
#include "DisplayTraceProvidersDlg.h"
#include "CreateTraceSessionDlg.h"


CCreateTraceSessionDlg::CCreateTraceSessionDlg(void) :
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

            /* Store the info pointer in the window's global user data */
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)This);

            This->m_hAddProvListView = GetDlgItem(hwnd, IDC_ADDPROV_LIST);
            This->m_hEditProvListView = GetDlgItem(hwnd, IDC_EDITPROV_LIST);

            {
            LVCOLUMNW lvc = {0};
            lvc.mask = LVCF_TEXT | LVCF_WIDTH;
            lvc.pszText = L"Property";
            lvc.cchTextMax = 1;
            lvc.cx = 150;
            ListView_InsertColumn(This->m_hEditProvListView, 0, &lvc);

            lvc.pszText = L"Value";
            lvc.cx = 174;
            ListView_InsertColumn(This->m_hEditProvListView, 0, &lvc);
            }

            return TRUE;

        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
            case IDC_ADDPROV_BTN:
            {
                CDisplayTraceProvidersDlg TraceProvDlg;
                CSelectedTrace *SelectedTrace;

                TraceProvDlg.ShowDialog(NULL);
            
                int count = TraceProvDlg.GetSelectedCount();
                for (int i = 0; i < count; i++)
                {
                    SelectedTrace = TraceProvDlg.GetSelectedItem(i);
                    if (SelectedTrace)
                    {
                        MessageBoxW(NULL, SelectedTrace->m_SessionName, NULL, MB_OK);
                        delete SelectedTrace;
                    }
                
                }
            }
                case IDOK:
                {

                }
                // fall through
                case IDCANCEL:
                    EndDialog(hwnd, wParam); 
                    return TRUE; 
            } 

        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code)
            {
                case TVN_SELCHANGEDW:
                {
                    NMTVITEMCHANGE *pnm;
                    pnm = (NMTVITEMCHANGE *)lParam;
                    return TRUE;
                }
            }
    } 

    return FALSE; 
}
