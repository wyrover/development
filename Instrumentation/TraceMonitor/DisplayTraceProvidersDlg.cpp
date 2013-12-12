#include "stdafx.h"
#include "Resource.h"
#include "devmgmt.h"
#include "DisplayTraceProvidersDlg.h"


CDisplayTraceProvidersDlg::CDisplayTraceProvidersDlg(void) :
    m_hTreeView(NULL)
{
    ZeroMemory(m_SessionName, _countof(m_SessionName));
    ZeroMemory(&m_TraceGuid, sizeof(GUID));
}


CDisplayTraceProvidersDlg::~CDisplayTraceProvidersDlg(void)
{
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
            HTREEITEM hPrev = (HTREEITEM)TVI_FIRST;

            for (DWORD i = 0; i < pBuffer->NumberOfProviders; i++)
            {
                //printf("%S  (%lu)\n", (PBYTE)pBuffer + pBuffer->TraceProviderInfoArray[i].ProviderNameOffset, pBuffer->TraceProviderInfoArray[i].SchemaSource);

                TVITEM tvi;
                tvi.mask = TVIF_TEXT | TVIF_PARAM;
                tvi.pszText = (LPWSTR)((PBYTE)pBuffer + pBuffer->TraceProviderInfoArray[i].ProviderNameOffset);
                tvi.cchTextMax = wcslen(tvi.pszText) + 1;
                tvi.lParam = (LPARAM)&pBuffer->TraceProviderInfoArray[i].ProviderGuid;

                TVINSERTSTRUCT tvins;
                tvins.item = tvi;
                tvins.hInsertAfter = hPrev;
                tvins.hParent = TVI_ROOT;

                hPrev = TreeView_InsertItem(This->m_hTreeView, &tvins);
            }

            TreeView_SortChildren(This->m_hTreeView, TVI_ROOT, FALSE);
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
    CDisplayTraceProvidersDlg *This = (CDisplayTraceProvidersDlg *)lParam;
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

            This->m_hTreeView = GetDlgItem(hwnd, IDC_TREE1);

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
                    HTREEITEM hTreeItem;
                    TVITEM TvItem;
                    WCHAR Buffer[1024];

                    hTreeItem = TreeView_GetSelection(This->m_hTreeView);
                    if (hTreeItem)
                    {
                        ZeroMemory(&TvItem, sizeof(TVITEM));
                        TvItem.mask = TVIF_TEXT | TVIF_PARAM;
                        TvItem.hItem = hTreeItem;
                        TvItem.pszText = Buffer;
                        TvItem.cchTextMax = 1024;

                        if (TreeView_GetItem(This->m_hTreeView, &TvItem))
                        {
                            wcscpy_s(This->m_SessionName, 1024, TvItem.pszText);
                            CopyMemory(&This->m_TraceGuid, (LPVOID)TvItem.lParam, sizeof(GUID));

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
