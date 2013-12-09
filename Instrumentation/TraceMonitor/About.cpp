#include "stdafx.h"
#include "Resource.h"
#include "devmgmt.h"
#include "About.h"


CAbout::CAbout(void)
{
}


CAbout::~CAbout(void)
{
}

BOOL
CAbout::ShowDialog(HWND hParent)
{
    return (DialogBoxParamW(g_hInstance,
                            MAKEINTRESOURCEW(IDD_ABOUTBOX),
                            hParent,
                            AboutWndProc,
                            (LPARAM)this) > 0);
}

BOOL CALLBACK
CAbout::AboutWndProc(HWND hwnd,
                     UINT msg,
                     WPARAM wParam,
                     LPARAM lParam)
{
    CAbout *This = (CAbout *)lParam;

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
