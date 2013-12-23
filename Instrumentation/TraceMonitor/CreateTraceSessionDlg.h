#pragma once
class CCreateTraceSessionDlg
{
    HWND m_hAddProvListView;
    HWND m_hEditProvListView;

public:
    CCreateTraceSessionDlg(void);
    ~CCreateTraceSessionDlg(void);

    BOOL ShowDialog(HWND hParent);

private:
    static BOOL CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

};

