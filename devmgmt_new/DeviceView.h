#pragma once
#include "Devices.h"

enum ListDevices
{
    ByType,
    ByConnection

};

class CDeviceView : public CDevices
{
    CDevices *m_Devices;
    HWND m_hMainWnd;
    HWND m_hTreeView;
    HWND m_hPropertyDialog;
    HWND m_hShortcutMenu;
    ListDevices m_ListDevices;

    HIMAGELIST m_ImageList;
    //HDEVINFO m_hDevInfo;

    HTREEITEM m_hTreeRoot;

public:
    CDeviceView(HWND hMainWnd);
    ~CDeviceView(void);

    BOOL Initialize();
    BOOL Uninitialize();

    VOID Size(INT x, INT y, INT cx, INT cy);
    VOID Refresh();
    VOID DisplayPropertySheet();
    VOID SetFocus();

    VOID SetDeviceListType(ListDevices List)
    {
        m_ListDevices = List;
    }

    static unsigned int __stdcall ListDevicesThread(void *Param);

private:
    BOOL ListDevicesByConnection();
    BOOL ListDevicesByType();


    HTREEITEM AddDeviceToTree(HWND hTreeView,
                HTREEITEM hRoot,
                DEVINST dnDevInst,
                BOOL bShowHidden);

    INT EnumDeviceClasses(INT ClassIndex,
                  BOOL ShowHidden,
                  LPTSTR DevClassName,
                  LPTSTR DevClassDesc,
                  BOOL *DevPresent,
                  INT *ClassImage,
                  BOOL *IsUnknown,
                  BOOL *IsHidden);

    LONG EnumDevices(INT index,
            LPTSTR DeviceClassName,
            LPTSTR DeviceName,
            LPTSTR *DeviceID);

     HTREEITEM InsertIntoTreeView(HWND hTreeView,
                   HTREEITEM hRoot,
                   LPTSTR lpLabel,
                   LPTSTR DeviceID,
                   INT DevImage,
                   LONG DevProb);
};

