#pragma once


typedef struct _MENU_HINT
{
    WORD CmdId;
    UINT HintId;
} MENU_HINT, *PMENU_HINT;

class CMainWindow
{
    WCHAR m_szMainWndClass[32];
    HWND m_hMainWnd;
    int m_CmdShow;

public:
    CMainWindow(void);
    ~CMainWindow(void);

    BOOL Initialize(LPCTSTR lpCaption, int nCmdShow);
    INT Run();
    VOID Uninitialize();

private:
    static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    BOOL AddTrayIcon(void);
    BOOL RemoveTrayIcon(void);
    BOOL TurnDisplayOff(void);

    LRESULT OnCreate(HWND hwnd);
    LRESULT OnDestroy();
    LRESULT OnSize();
    LRESULT OnNotify(LPARAM lParam);
    LRESULT OnTimer(WPARAM wParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
};

