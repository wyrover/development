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
    static BOOL CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    BOOL InitializeDialog(HWND hwnd);
    BOOL AddProvidersToTraceSession();
    BOOL AddProvidersToList();
    bool CreateTraceSession();

    CTraceSession* GetTraceSession()
    {
        return m_NewTraceSession;
    }

};

