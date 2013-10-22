#pragma once
#include "Devices.h"

enum ListDevices
{
    DevicesByType,
    DevicesByConnection,
    ResourcesByType,
    ResourcesByConnection
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


private:
    static unsigned int __stdcall ListDevicesThread(void *Param);

    BOOL ListDevicesByConnection(
        );
    BOOL ListDevicesByType(
        );


    HTREEITEM AddDeviceToTree(HWND hTreeView,
                HTREEITEM hRoot,
                DEVINST dnDevInst,
                BOOL bShowHidden);


    HTREEITEM InsertIntoTreeView(HWND hTreeView,
                   HTREEITEM hRoot,
                   LPTSTR lpLabel,
                   LPTSTR DeviceID,
                   INT DevImage,
                   LONG DevProb);
};

