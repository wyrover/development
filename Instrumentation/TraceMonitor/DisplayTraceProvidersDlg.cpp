#include "stdafx.h"
#include "Resource.h"
#include "devmgmt.h"
#include "DisplayTraceProvidersDlg.h"


CDisplayTraceProvidersDlg::CDisplayTraceProvidersDlg(void) :
    m_hListView(NULL)
{
}

CDisplayTraceProvidersDlg::~CDisplayTraceProvidersDlg(void)
{
}

CSelectedTrace*
CDisplayTraceProvidersDlg::GetSelectedItem(int item)
{
    if (item > (int)m_SelectedItems.size())
        return nullptr;

    return &m_SelectedItems.at(item);
}

BOOL
CDisplayTraceProvidersDlg::ShowDialog(HWND hParent)
{
    return (DialogBoxParamW(g_hInstance,
                            MAKEINTRESOURCEW(IDD_DISP_TRACE_PROV),
                            hParent,
                            WndProc,
                            (LPARAM)this) == IDOK);
}

UINT WINAPI
CDisplayTraceProvidersDlg::EnumProvidersThread(_In_opt_ LPVOID lpParam)
{
    CDisplayTraceProvidersDlg* This = reinterpret_cast<CDisplayTraceProvidersDlg *>(lpParam);
    PPROVIDER_ENUMERATION_INFO pBuffer;
    DWORD BufferSize, dwError;

    BufferSize = 0;
    dwError = TdhEnumerateProviders(NULL, &BufferSize);
    if (dwError == ERROR_INSUFFICIENT_BUFFER)
    {
        pBuffer = (PPROVIDER_ENUMERATION_INFO)HeapAlloc(GetProcessHeap(), 0, BufferSize);
        if (pBuffer == NULL) return ERROR_NOT_ENOUGH_MEMORY;

        dwError = TdhEnumerateProviders(pBuffer, &BufferSize);
        if (dwError == ERROR_SUCCESS)
        {
            int ii;

            for (DWORD i = 0; i < pBuffer->NumberOfProviders; i++)
            {
                LVITEM lvi = {0};
                lvi.mask = LVIF_TEXT | LVIF_PARAM;
                lvi.pszText = (LPWSTR)((PBYTE)pBuffer + pBuffer->TraceProviderInfoArray[i].ProviderNameOffset);
                lvi.cchTextMax = wcslen(lvi.pszText) + 1;
                lvi.lParam = (LPARAM)&pBuffer->TraceProviderInfoArray[i].ProviderGuid;

                ii = ListView_InsertItem(This->m_hListView, &lvi);
            }
        }

        //HeapFree(GetProcessHeap(), 0, pBuffer);
    }

    return 0;
}

BOOL CALLBACK
CDisplayTraceProvidersDlg::WndProc(HWND hwnd,
                                   UINT msg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
    CDisplayTraceProvidersDlg *This = NULL;
    HANDLE hEnumThread;

    /* Get the object pointer from window context */
    This = (CDisplayTraceProvidersDlg *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    /* Don't handle null info pointer */
    if (This == NULL && (msg != WM_INITDIALOG))
        return FALSE;


   switch (msg)
    { 
        case WM_INITDIALOG:

            This = (CDisplayTraceProvidersDlg *)lParam;

            /* Store the info pointer in the window's global user data */
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)This);

            This->m_hListView = GetDlgItem(hwnd, IDC_TRACE_PROV_LIST);
            {
            LVCOLUMNW lvc = {0};
            lvc.mask = LVCF_TEXT | LVCF_WIDTH;
            lvc.pszText = L"";
            lvc.cchTextMax = 1;
            lvc.cx = 300;

            ListView_InsertColumn(This->m_hListView, 0, &lvc);

            ListView_SetExtendedListViewStyle(This->m_hListView, LVS_EX_FULLROWSELECT);
            }
            hEnumThread = (HANDLE)_beginthreadex(NULL,
                                                 0,
                                                 EnumProvidersThread,
                                                 (LPVOID)This,
                                                 0,
                                                 NULL);
            if (hEnumThread > 0)
            {
                CloseHandle(hEnumThread);
            }

            return TRUE;

        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK:
                {
                    This->m_SelectedItems.clear();

                    for(int item = ListView_GetNextItem(This->m_hListView, -1, LVNI_SELECTED);
                        item >= 0;
                        item = ListView_GetNextItem(This->m_hListView, item, LVNI_SELECTED))
                    {
                        LVITEM lvi = {0};
                        CSelectedTrace SelectedTrace;

                        lvi.iItem = item;
                        lvi.mask = LVIF_TEXT | LVIF_PARAM;
                        lvi.pszText = SelectedTrace.SessionName;
                        lvi.cchTextMax = 1024;

                        if (ListView_GetItem(This->m_hListView, &lvi))
                        {
                            CopyMemory(&SelectedTrace.TraceGuid, (LPVOID)lvi.lParam, sizeof(GUID));
                            This->m_SelectedItems.push_back(SelectedTrace);
                        }
                    }
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
