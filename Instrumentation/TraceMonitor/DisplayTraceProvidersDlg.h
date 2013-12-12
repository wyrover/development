#pragma once
class CDisplayTraceProvidersDlg
{
private:
    HWND m_hTreeView;
    WCHAR m_SessionName[1024];
    GUID m_TraceGuid;

public:
    CDisplayTraceProvidersDlg(void);
    ~CDisplayTraceProvidersDlg(void);

    BOOL ShowDialog(HWND hParent);

    LPCWSTR GetSessionName() { return m_SessionName; }
    LPCGUID GetTraceGuid() { return &m_TraceGuid; }

private:
        static BOOL CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        static UINT WINAPI EnumProvidersThread(
        _In_opt_ LPVOID lpParam
        );
};