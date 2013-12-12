#pragma once
class CAboutDlg
{
public:
    CAboutDlg(void);
    ~CAboutDlg(void);

    BOOL ShowDialog(HWND hParent);

private:
        static BOOL CALLBACK AboutWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

};

