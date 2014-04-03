#pragma once

class CTraceView
{
    HWND m_hParent;
    HWND m_hListView;
    HWND m_hPropertyDialog;
    HWND m_hShortcutMenu;

    HIMAGELIST m_ImageList;

public:
    CTraceView(void);
    ~CTraceView(void);

    BOOL Initialize(_In_ HWND hParent);
    BOOL Uninitialize();

    VOID Size(
        _In_ INT x,
        _In_ INT y,
        _In_ INT cx,
        _In_ INT cy
        );

    VOID Refresh();
    VOID DisplayPropertySheet();
    VOID SetFocus();


private:
    static unsigned int __stdcall ListDevicesThread(
        void *Param
        );

};

