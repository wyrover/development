#pragma once
#include <vector>

using namespace std;

class CSelectedTrace
{
public:
    WCHAR m_SessionName[1024];
    GUID m_TraceGuid;
};

class CDisplayTraceProvidersDlg
{
private:
    HWND m_hListView;
    vector<CSelectedTrace> m_SelectedItems;

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