#pragma once

struct CSelectedTrace
{
    WCHAR SessionName[1024];
    GUID TraceGuid;
};

class CDisplayTraceProvidersDlg
{
private:
    HWND m_hListView;
    std::vector<CSelectedTrace> m_SelectedItems;

public:
    CDisplayTraceProvidersDlg(void);
    ~CDisplayTraceProvidersDlg(void);

    BOOL ShowDialog(HWND hParent);
    INT GetSelectedCount( ) { return m_SelectedItems.size(); }
    CSelectedTrace* GetSelectedItem(INT item);
    

private:
        static BOOL CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        static UINT WINAPI EnumProvidersThread(
        _In_opt_ LPVOID lpParam
        );
};