#pragma once
#include "TraceView.h"
#include "TraceSession.h"

typedef struct _MENU_HINT
{
    WORD CmdId;
    UINT HintId;
} MENU_HINT, *PMENU_HINT;

class CMainWindow
{
    CAtlString m_szMainWndClass;
    CTraceView *m_TraceView;
    HWND m_hMainWnd;
    HWND m_hStatusBar;
    HWND m_hToolBar;
    HIMAGELIST m_ToolbarhImageList;
    HMENU m_hMenu;
    int m_CmdShow;

    std::vector<CTraceSession *> m_TraceSessions; //fixme: this should be linked to tabs or something

private:
    static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnCreate(HWND hwnd);
    LRESULT OnDestroy();
    LRESULT OnSize();
    LRESULT OnNotify(LPARAM lParam);
    LRESULT OnContext(LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

    BOOL CreateToolBar();
    BOOL CreateStatusBar();
    BOOL StatusBarLoadString(HWND hStatusBar, INT PartId, HINSTANCE hInstance, UINT uID);
    BOOL MainWndMenuHint(WORD CmdId,
                         const MENU_HINT *HintArray,
                         DWORD HintsCount,
                         UINT DefHintId);

public:
    CMainWindow(void);
    ~CMainWindow(void);

    BOOL Initialize(LPCTSTR lpCaption, int nCmdShow);
    INT Run();
    VOID Uninitialize();


};

