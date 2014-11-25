#include "StdAfx.h"
#include "devmgmt.h"
#include "DeviceView.h"


/* PUBLIC METHODS *************************************/

CDeviceView::CDeviceView(
    HWND hMainWnd
    ) :
    m_Devices(NULL),
    m_hMainWnd(hMainWnd),
    m_hTreeView(NULL),
    m_hPropertyDialog(NULL),
    m_hShortcutMenu(NULL),
    m_ListDevices(DevicesByType),
    m_ShowHidden(TRUE),
    m_ShowUnknown(TRUE)
{
    m_Devices = new CDevices();
}

CDeviceView::~CDeviceView(void)
{
    delete m_Devices;
    m_Devices = NULL;
}

BOOL
CDeviceView::Initialize()
{
    BOOL bSuccess;

    /* Initialize the devices class */
    bSuccess = m_Devices->Initialize();
    if (bSuccess == FALSE) return FALSE;

    /* Create the main treeview */
    m_hTreeView = CreateWindowExW(WS_EX_CLIENTEDGE,
                                  WC_TREEVIEW,
                                  NULL,
                                  WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_HASLINES |
                                   TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_LINESATROOT,
                                  0, 0, 0, 0,
                                  m_hMainWnd,
                                  (HMENU)IDC_TREEVIEW,
                                  g_hInstance,
                                  NULL);
    if (m_hTreeView)
    {
        /* Get the devices image list */
        m_ImageList = m_Devices->GetImageList();

        /* Set the image list against the treeview */
        (VOID)TreeView_SetImageList(m_hTreeView,
                                    m_ImageList,
                                    TVSIL_NORMAL);
    }

    return !!(m_hTreeView);
}

BOOL
CDeviceView::Uninitialize()
{
    (VOID)m_Devices->Uninitialize();

    return TRUE;
}

VOID
CDeviceView::Size(INT x,
                  INT y,
                  INT cx,
                  INT cy)
{
    /* Resize the treeview */
    SetWindowPos(m_hTreeView,
                 NULL,
                 x,
                 y,
                 cx,
                 cy,
                 SWP_NOZORDER);
}


VOID
CDeviceView::Refresh()
{
    HANDLE hThread;

    /* Run on a new thread to keep the gui responsive */
    hThread = (HANDLE)_beginthreadex(NULL,
                                     0,
                                     &ListDevicesThread,
                                     this,
                                     0,
                                     NULL);

    if (hThread) CloseHandle(hThread);
}

VOID
CDeviceView::DisplayPropertySheet()
{
}

VOID
CDeviceView::SetFocus()
{
}


/* PRIVATE METHODS ********************************************/

unsigned int __stdcall CDeviceView::ListDevicesThread(void *Param)
{
    CDeviceView *This = (CDeviceView *)Param;

    /* Check if there are any items in the tree */
    if (TreeView_GetRoot(This->m_hTreeView) != NULL)
    {
        /* Delete all the items */
        (VOID)TreeView_DeleteAllItems(This->m_hTreeView);
    }

    /* Reset the tree root */
    This->m_hTreeRoot = NULL;

    switch (This->m_ListDevices)
    {
    case DevicesByType:
        (VOID)This->ListDevicesByType();
        break;

    case DevicesByConnection:
        (VOID)This->ListDevicesByConnection();
        break;

    case ResourcesByType:
        break;

    case ResourcesByConnection:
        break;
    }
    return 0;
}


BOOL
CDeviceView::ListDevicesByType()
{
    HTREEITEM hDevItem = NULL;
    GUID ClassGuid;
    WCHAR ClassName[256];
    WCHAR ClassDescription[256];
    INT ClassIndex;
    INT ClassImage;
    LPTSTR DeviceId = NULL;
    
    BOOL IsUnknown = FALSE;
    BOOL IsHidden = FALSE;
    BOOL bSuccess;


    /* Get the details of the root of the device tree */
    bSuccess = m_Devices->GetDeviceTreeRoot(ClassName, 256, &ClassImage);
    if (bSuccess)
    {
        /* Add the root of the device tree to the treeview */
        m_hTreeRoot = InsertIntoTreeView(NULL,
                                         ClassName,
                                         NULL,
                                         ClassImage,
                                         0);
    }

    /* If something went wrong, bail */
    if (m_hTreeRoot == NULL) return FALSE;

    ClassIndex = 0;
    do
    {
        /* Get the next device class */
        bSuccess = m_Devices->EnumClasses(ClassIndex,
                                          &ClassGuid,
                                          ClassName,
                                          256,
                                          ClassDescription,
                                          256,
                                          &ClassImage,
                                          &IsUnknown,
                                          &IsHidden);
        if (bSuccess &&
            (IsUnknown == FALSE || (IsUnknown && m_ShowUnknown)) &&
            (IsHidden == FALSE || (IsHidden && m_ShowHidden)))
        {
            BOOL bDevSuccess;
            HANDLE Handle = NULL;
            WCHAR DeviceName[256];
            INT DeviceIndex = 0;
            BOOL MoreItems = FALSE;
            BOOL DeviceHasProblem = FALSE;
            ULONG DeviceStatus, ProblemNumber, OverlayImage;


            /* Insert the new class under the root item */
            hDevItem = InsertIntoTreeView(m_hTreeRoot,
                                          ClassDescription,
                                          NULL,
                                          ClassImage,
                                          0);

            do
            {
                bDevSuccess = m_Devices->EnumDevicesForClass(&Handle,
                                                             &ClassGuid,
                                                             DeviceIndex,
                                                             &MoreItems,
                                                             DeviceName,
                                                             256,
                                                             &DeviceId);
                if (bDevSuccess)
                {
                    /* Get the status of the device */
                    if (m_Devices->GetDeviceStatus(DeviceId,
                                                   &DeviceStatus,
                                                   &ProblemNumber))
                    {
                        /* Check if the device has a problem */
                        if (DeviceStatus & DN_HAS_PROBLEM)
                        {
                            DeviceHasProblem = TRUE;
                            OverlayImage = 1;
                        }

                        if (ProblemNumber == CM_PROB_DISABLED ||
                            ProblemNumber == CM_PROB_HARDWARE_DISABLED)
                        {
                            OverlayImage = 2;
                        }
                    }

                    /* Add the device under the class item */
                    (VOID)InsertIntoTreeView(hDevItem,
                                             DeviceName,
                                             (LPARAM)DeviceId,
                                             ClassImage,
                                             OverlayImage);

                    /* Check if there's a problem with the device */
                    if (DeviceHasProblem)
                    {
                        /* Expand the class */
                        (VOID)TreeView_Expand(m_hTreeView,
                                              hDevItem,
                                              TVE_EXPAND);
                    }

                    HeapFree(GetProcessHeap(), 0, DeviceId);
                }

                DeviceIndex++;

            } while (MoreItems);

            /* Check if this class has any devices */
            if (TreeView_GetChild(m_hTreeView, hDevItem))
            {
                /* Sort the devices alphabetically */
                (VOID)TreeView_SortChildren(m_hTreeView,
                                            hDevItem,
                                            0);
            }
            else
            {
                /* The class has no devices, delete it from the treeview */
                (VOID)TreeView_DeleteItem(m_hTreeView, hDevItem);
            }
        }

        ClassIndex++;

    } while (bSuccess);

    /* Sort the classes alphabetically */
    (VOID)TreeView_SortChildren(m_hTreeView,
                                m_hTreeRoot,
                                0);

    /* Expand the root item */
    (VOID)TreeView_Expand(m_hTreeView,
                          m_hTreeRoot,
                          TVE_EXPAND);

    /* Pre-select the root item */
    (VOID)TreeView_SelectItem(m_hTreeView,
                              m_hTreeRoot);

    return 0;
}

BOOL
CDeviceView::ListDevicesByConnection()
{
    return FALSE;
    //DEVINST RootDevInst;

    //if (!m_Devices->GetDeviceTreeRoot(&RootDevInst))
    //    return FALSE;
    
    ////AddDeviceToTree(NULL, NULL, RootDevInst, TRUE);

    ///* Get the first child */
    //if (!m_Devices->GetChildDevice(RootDevInst, &DevInst))
    //    return FALSE;

    //AddDeviceToTree(NULL, NULL, DevInst, TRUE);

    //do
    //{
    //    /* Check if this device has any childern */
    //    if (m_Devices->GetChildDevice(DevInst, &DevInst))
    //    {
    //        /* List its siblings */
    //        while (m_Devices->GetSiblingDevice(DevInst, &DevInst))
    //        {
    //            /* add the sibling */
    //            AddDeviceToTree(NULL, NULL, DevInst, TRUE);
    //        }
    //    }

    //} while (m_Devices->GetSiblingDevice(DevInst, &DevInst));

    //return TRUE;
}

 HTREEITEM
CDeviceView::InsertIntoTreeView(_In_ HTREEITEM hParent,
                                _In_z_ LPWSTR lpLabel,
                                _In_ LPARAM lParam,
                                _In_ INT DevImage,
                                _In_ UINT OverlayImage)
{
    TV_ITEMW tvi;
    TV_INSERTSTRUCT tvins;

    ZeroMemory(&tvi, sizeof(tvi));
    ZeroMemory(&tvins, sizeof(tvins));

    tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvi.pszText = lpLabel;
    tvi.cchTextMax = wcslen(lpLabel);
    tvi.lParam = lParam;
    tvi.iImage = DevImage;
    tvi.iSelectedImage = DevImage;

    if (OverlayImage != 0)
    {
        tvi.mask |= TVIF_STATE;
        tvi.stateMask = TVIS_OVERLAYMASK;
        tvi.state = INDEXTOOVERLAYMASK(OverlayImage);
    }

    tvins.item = tvi;
    tvins.hParent = hParent;

    return TreeView_InsertItem(m_hTreeView, &tvins);
}
#if 0
HDEVINFO hDevInfo;
 INT
CDeviceView::EnumDeviceClasses(INT ClassIndex,
                               BOOL ShowHidden,
                               LPTSTR DevClassName,
                               LPTSTR DevClassDesc,
                               BOOL *DevPresent,
                               INT *ClassImage,
                               BOOL *IsUnknown,
                               BOOL *IsHidden)
{
    GUID ClassGuid;
    HKEY KeyClass;
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    DWORD RequiredSize = MAX_CLASS_NAME_LEN;
    UINT Ret;

    *DevPresent = FALSE;
    *DevClassName = _T('\0');
    *IsHidden = FALSE;

    Ret = CM_Enumerate_Classes(ClassIndex,
                               &ClassGuid,
                               0);
    if (Ret != CR_SUCCESS)
    {
        /* all classes enumerated */
        if(Ret == CR_NO_SUCH_VALUE)
        {
            return -1;
        }

        if (Ret == CR_INVALID_DATA)
        {
            ; /*FIXME: what should we do here? */
        }

        /* handle other errors... */
    }

    /* This case is special because these devices don't show up with normal class enumeration */
    *IsUnknown = IsEqualGUID(ClassGuid, GUID_DEVCLASS_UNKNOWN);

    if (ShowHidden == FALSE &&
        (IsEqualGUID(ClassGuid, GUID_DEVCLASS_LEGACYDRIVER) ||
         IsEqualGUID(ClassGuid, GUID_DEVCLASS_VOLUME)))
        *IsHidden = TRUE;

    if (SetupDiClassNameFromGuid(&ClassGuid,
                                 ClassName,
                                 RequiredSize,
                                 &RequiredSize))
    {
        lstrcpy(DevClassName, ClassName);
    }

    //if (!SetupDiGetClassImageIndex(&m_ImageListData,
    //                               &ClassGuid,
    //                               ClassImage))
    //{
    //    /* FIXME: can we do this?
    //     * Set the blank icon: IDI_SETUPAPI_BLANK = 41
    //     * it'll be image 24 in the imagelist */
    //    *ClassImage = 24;
    //}

    /* Get device info for all devices of a particular class */
    hDevInfo = SetupDiGetClassDevs(*IsUnknown ? NULL : &ClassGuid,
                                   NULL,
                                   NULL,
                                   DIGCF_PRESENT | (*IsUnknown ? DIGCF_ALLCLASSES : 0));
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    KeyClass = SetupDiOpenClassRegKeyEx(&ClassGuid,
                                        MAXIMUM_ALLOWED,
                                        DIOCR_INSTALLER,
                                        NULL,
                                        0);
    if (KeyClass != INVALID_HANDLE_VALUE)
    {

        LONG dwSize = 256;

        if (RegQueryValue(KeyClass,
                          NULL,
                          DevClassDesc,
                          &dwSize) != ERROR_SUCCESS)
        {
            *DevClassDesc = _T('\0');
        }
    }
    else
    {
        return 0;
    }

    *DevPresent = TRUE;

    RegCloseKey(KeyClass);

    return 0;
}
#endif
