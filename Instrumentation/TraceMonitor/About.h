#pragma once
class CAbout
{
public:
    CAbout(void);
    ~CAbout(void);

    BOOL ShowDialog(HWND hParent);

private:
        static BOOL CALLBACK AboutWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

};

