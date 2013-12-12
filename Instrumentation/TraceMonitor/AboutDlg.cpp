#include "stdafx.h"
#include "Resource.h"
#include "devmgmt.h"
#include "AboutDlg.h"


CAboutDlg::CAboutDlg(void)
{
}


CAboutDlg::~CAboutDlg(void)
{
}

BOOL
CAboutDlg::ShowDialog(HWND hParent)
{
    return (DialogBoxParamW(g_hInstance,
                            MAKEINTRESOURCEW(IDD_ABOUTBOX),
                            hParent,
                            AboutWndProc,
                            (LPARAM)this) > 0);
}

BOOL CALLBACK
CAboutDlg::AboutWndProc(HWND hwnd,
                        UINT msg,
                        WPARAM wParam,
                        LPARAM lParam)
{
    CAboutDlg *This = (CAboutDlg *)lParam;

   switch (msg) 
    { 
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
                    EndDialog(hwnd, wParam); 
                    return TRUE; 
            } 
    } 

    return FALSE; 
}
