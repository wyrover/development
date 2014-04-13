#include "stdafx.h"
#include "Resource.h"
#include "devmgmt.h"
#include "DisplayTraceProvidersDlg.h"
#include "CreateTraceSessionDlg.h"


CCreateTraceSessionDlg::CCreateTraceSessionDlg(void) :
    m_NewTraceSession(nullptr),
    m_hAddProvListView(NULL),
    m_hEditProvListView(NULL)
{
}

CCreateTraceSessionDlg::~CCreateTraceSessionDlg(void)
{
}

HPROPSHEETPAGE
CCreateTraceSessionDlg::InitPropSheetPage(PROPSHEETPAGEW *psp,
                                          WORD idDlg,
                                          DLGPROC DlgProc)
{
    ZeroMemory(psp, sizeof(PROPSHEETPAGEW));
    psp->dwSize = sizeof(PROPSHEETPAGEW);
    psp->dwFlags = PSP_DEFAULT;
    psp->hInstance = g_hInstance;
    psp->pszTemplate = MAKEINTRESOURCEW(idDlg);
    psp->pfnDlgProc = DlgProc;

    return CreatePropertySheetPageW(psp);
}

BOOL
CCreateTraceSessionDlg::ShowDialog(HWND hParent)
{
    PROPSHEETHEADERW psh;
    PROPSHEETPAGEW psp[2];
    WCHAR Caption[] = L"my caption";
    LONG Ret = 0;

    ZeroMemory(&psh, sizeof(PROPSHEETHEADERW));
    psh.dwSize = sizeof(PROPSHEETHEADERW);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_PROPTITLE | PSH_WIZARD | PSH_AEROWIZARD | PSH_WIZARDHASFINISH;
    psh.hwndParent = hParent;
    psh.hInstance = g_hInstance;
    psh.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN_ICON));
    psh.pszCaption = Caption;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGEW);
    psh.nStartPage = 0;
    psh.ppsp = psp;

    InitPropSheetPage(&psp[0], IDD_CREATE_TRACE_NAME, (DLGPROC)TraceNameWndProc);
    InitPropSheetPage(&psp[1], IDD_CREATE_TRACE_SESSION, (DLGPROC)TraceSessionWndProc);

    return (PropertySheetW(&psh) != -1);
}

BOOL CCreateTraceSessionDlg::SetTraceDirectory(_In_ HWND hParent)
{
    std::wstring dir;
    if (GetDirFromBrowseDlg(hParent, dir))
    {
        HWND hTxt = GetDlgItem(hParent, IDC_TRACEBROWSE_EDIT);
        SetWindowTextW(hTxt, dir.c_str());
    }

    return TRUE;
}

bool CCreateTraceSessionDlg::GetDirFromBrowseDlg(
    HWND hParent,
    std::wstring& Directory
    )
{
    CComPtr<IFileOpenDialog> pDlg;
    HRESULT hr;
    bool Success = false;

    hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
    if (FAILED(hr)) return false;

    pDlg->SetTitle(L"Select a folder");
    pDlg->SetOptions(FOS_PICKFOLDERS);

    // Show the dialog
    hr = pDlg->Show(hParent);
    if (SUCCEEDED(hr))
    {
        CComPtr<IShellItem> Item;
        hr = pDlg->GetResult(&Item);
        if (SUCCEEDED(hr))
        {
            LPOLESTR pwsz = NULL;
            hr = Item->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
            if (SUCCEEDED(hr))
            {
                Directory = pwsz;
                CoTaskMemFree(pwsz);
                Success = true;
            }
        }
    }

    return Success;
}

BOOL CCreateTraceSessionDlg::AddProvidersToList()
{
    CDisplayTraceProvidersDlg TraceProvDlg;
    CSelectedTrace *SelectedTrace;

    BOOL bSuccess = TraceProvDlg.ShowDialog(NULL);
    if (bSuccess == FALSE) return FALSE;
            
    int count = TraceProvDlg.GetSelectedCount();
    if (count == 0) return FALSE;
    int ret;
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

            ret = ListView_InsertItem(m_hAddProvListView, &lvi);
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
            m_NewTraceSession->AddTraceProvider((CTraceProvider *)lvi.lParam);
        }
    }

    return TRUE;
}

bool CCreateTraceSessionDlg::CreateTraceSession(_In_ HWND hParent)
{
    WCHAR Name[MAX_PATH];
    WCHAR Dir[MAX_PATH];

    GetDlgItemTextW(hParent, IDC_TRACENAME_EDIT, Name, MAX_PATH);
    GetDlgItemTextW(hParent, IDC_TRACEBROWSE_EDIT, Dir, MAX_PATH);

    m_NewTraceSession = new CTraceSession();

    HRESULT hr;
    hr = m_NewTraceSession->Create(Name, Dir);
    return SUCCEEDED(hr);
}

BOOL CCreateTraceSessionDlg::InitTraceSessionPage(HWND hwnd)
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
CCreateTraceSessionDlg::TraceNameWndProc(HWND hwnd,
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
            
            SetDlgItemTextW(hwnd, IDC_TRACEBROWSE_EDIT, L"%LOCALAPPDATA%\\");
            PropSheet_SetWizButtons(hwnd, PSWIZB_NEXT);

            return TRUE;

        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDC_TRACE_BROWSE_BTN:
                    This->SetTraceDirectory(hwnd);
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, wParam); 
                    return TRUE; 
            }
            break;

        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code)
            {
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(hwnd, PSWIZB_NEXT);
                    break;

                case PSN_WIZFINISH:
                    This->CreateTraceSession(hwnd);
                    break;
            }
            break;

        case WM_DESTROY:
            break;
    } 

    return FALSE; 
}

BOOL CALLBACK
CCreateTraceSessionDlg::TraceSessionWndProc(HWND hwnd,
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
            This->InitTraceSessionPage(hwnd);
            return TRUE;

        case WM_COMMAND: 
            switch (LOWORD(wParam))
            { 
                case IDC_ADDPROV_BTN:
                    This->AddProvidersToList();
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, wParam); 
                    return TRUE; 
            }
            break;

        case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code)
            {
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(hwnd, PSWIZB_NEXT | PSWIZB_BACK);
                    break;

                case PSN_WIZFINISH:
                    This->CreateTraceSession(hwnd);
                    break;


                //case TVN_SELCHANGEDW:
                //{
                //    NMTVITEMCHANGE *pnm;
                //    pnm = (NMTVITEMCHANGE *)lParam;
                //    return TRUE;
                //}
            }
            break;

        case WM_DESTROY:
            break;
    } 

    return FALSE; 
}
