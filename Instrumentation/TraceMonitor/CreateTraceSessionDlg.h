#pragma once
#include "TraceSession.h"

class CCreateTraceSessionDlg
{
    CTraceSession *m_NewTraceSession;
    HWND m_hAddProvListView;
    HWND m_hEditProvListView;

public:
    CCreateTraceSessionDlg(void);
    ~CCreateTraceSessionDlg(void);

    BOOL ShowDialog(HWND hParent);

    CTraceSession* GetNewTraceSession()
    {
        return m_NewTraceSession; 
    }

private:
    static BOOL CALLBACK TraceNameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK TraceSessionWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    
    HPROPSHEETPAGE InitPropSheetPage(
        PROPSHEETPAGEW *psp,
        WORD idDlg,
        DLGPROC DlgProc
        );

    BOOL InitTraceSessionPage(HWND hwnd);
    BOOL AddProvidersToTraceSession();
    BOOL AddProvidersToList();
    BOOL SetTraceDirectory(_In_ HWND hParent);
    bool CreateTraceSession(_In_ HWND hParent);
    bool GetDirFromBrowseDlg(
        HWND hParent,
        std::wstring& Directory
        );

    CTraceSession* GetTraceSession()
    {
        return m_NewTraceSession;
    }

};

