#include "StdAfx.h"
#include "devmgmt.h"
#include "DeviceView.h"


/* PUBLIC METHODS *************************************/

CDeviceView::CDeviceView(HWND hMainWnd) :
    m_Devices(NULL),
    m_hMainWnd(hMainWnd),
    m_hTreeView(NULL),
    m_hPropertyDialog(NULL),
    m_hShortcutMenu(NULL),
    m_ListDevices(DevicesByType)
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
    TCHAR DevName[256];
    TCHAR DevDesc[256];
    LPTSTR DeviceID = NULL;
    BOOL DevExist = FALSE;
    INT ClassRet;
    INT index = 0;
    INT DevImage;
    BOOL IsUnknown = FALSE;
    BOOL IsHidden = FALSE;

    BOOL bSuccess;

    m_Devices->GetDeviceTreeRoot(DevName, 256, &DevImage);

    m_hTreeRoot = InsertIntoTreeView(m_hTreeView,
                                     NULL,
                                     DevName,
                                     NULL,
                                     DevImage,
                                     0);


    do
    {
        bSuccess = m_Devices->EnumDeviceClasses(index,
                                                DevName,
                                                256,
                                                DevDesc,
                                                256,
                                                &DevImage,
                                                &IsUnknown,
                                                &IsHidden);

        //ClassRet = EnumDeviceClasses(index,
        //                             TRUE,
        //                             DevName,
        //                             DevDesc,
        //                             &DevExist,
        //                             &DevImage,
        //                             &IsUnknown,
        //                             &IsHidden);

        if (bSuccess)
        {
            TCHAR DeviceName[256];
            INT DevIndex = 0;
            BOOL HasChild = FALSE;

            if (DevDesc[0] != _T('\0'))
            {
                hDevItem = InsertIntoTreeView(m_hTreeView,
                                              m_hTreeRoot,
                                              DevDesc,
                                              NULL,
                                              DevImage,
                                              0);
            }
            else
            {
                hDevItem = InsertIntoTreeView(m_hTreeView,
                                              m_hTreeRoot,
                                              DevName,
                                              NULL,
                                              DevImage,
                                              0);
            }

            do
            {
                //Ret = EnumDevices(DevIndex,
                //                  IsUnknown ? NULL : DevName,
                //                  DeviceName,
                //                  &DeviceID);
                //bSuccess = m_Devices->EnumChildDevices(DevIndex,
                //                                       &HasChild,
                //                                       DeviceName,
                //                                       256,
                //                                       &DeviceID);

                if (m_Devices->EnumChildDevices(index,
                                                DevIndex,
                                                       &HasChild,
                                                       DeviceName,
                                                       256,
                                                       &DeviceID))
                {
                    InsertIntoTreeView(m_hTreeView,
                                       hDevItem,
                                       DeviceName,
                                       DeviceID,
                                       DevImage,
                                       0);
                    //if (Ret != 0)
                    {
                        /* Expand the class if the device has a problem */
                        (void)TreeView_Expand(m_hTreeView,
                                              hDevItem,
                                              TVE_EXPAND);
                    }
                }

                DevIndex++;

            } while (HasChild);

            ///* don't insert classes with no devices */
            //if (!TreeView_GetChild(m_hTreeView,
            //                       hDevItem))
            //{
            //    (void)TreeView_DeleteItem(m_hTreeView,
            //                              hDevItem);
            //}
            //else
            //{
            //    (void)TreeView_SortChildren(m_hTreeView,
            //                                hDevItem,
            //                                0);
            //}
        }

        index++;

    } while (bSuccess);

    (void)TreeView_Expand(m_hTreeView,
                          m_hTreeRoot,
                          TVE_EXPAND);

    (void)TreeView_SortChildren(m_hTreeView,
                                m_hTreeRoot,
                                0);

    (void)TreeView_SelectItem(m_hTreeView,
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
CDeviceView::InsertIntoTreeView(HWND hTreeView,
                   HTREEITEM hRoot,
                   LPTSTR lpLabel,
                   LPTSTR DeviceID,
                   INT DevImage,
                   LONG DevProb)
{
    TV_ITEM tvi;
    TV_INSERTSTRUCT tvins;

    ZeroMemory(&tvi, sizeof(tvi));
    ZeroMemory(&tvins, sizeof(tvins));

    tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvi.pszText = lpLabel;
    tvi.cchTextMax = lstrlen(lpLabel);
    tvi.lParam = (LPARAM)DeviceID;
    tvi.iImage = DevImage;
    tvi.iSelectedImage = DevImage;

    if (DevProb != 0)
    {
        tvi.stateMask = TVIS_OVERLAYMASK;

        if (DevProb == CM_PROB_DISABLED)
        {
            /* FIXME: set the overlay icon */
        }

    }

    tvins.item = tvi;
    tvins.hParent = hRoot;

    return TreeView_InsertItem(hTreeView, &tvins);
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
