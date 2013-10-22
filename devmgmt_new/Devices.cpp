#include "StdAfx.h"
#include "devmgmt.h"
#include "Devices.h"

CDevices::CDevices(void) :
    m_bInitialized(FALSE)
{
    /* get the device image List */
    ZeroMemory(&m_ImageListData, sizeof(SP_CLASSIMAGELIST_DATA));
}

CDevices::~CDevices(void)
{
    ATLASSERT(m_bInitialized == FALSE);
}

BOOL
CDevices::Initialize()
{
    BOOL bSuccess;

    ATLASSERT(m_bInitialized == FALSE);

    bSuccess = CreateImageList();
    if (bSuccess)
    {
        m_bInitialized = TRUE;
    }

    return m_bInitialized;
}

BOOL
CDevices::Uninitialize()
{
    if (m_ImageListData.ImageList != NULL)
        SetupDiDestroyClassImageList(&m_ImageListData);

    m_bInitialized = FALSE;

    return TRUE;
}

BOOL
CDevices::GetDeviceTreeRoot(_Out_ PDEVINST DevInst)
{
    CONFIGRET cr;

    cr = CM_Locate_DevNodeW(DevInst,
                            NULL,
                            CM_LOCATE_DEVNODE_NORMAL);
    return (cr == CR_SUCCESS);
}

BOOL
CDevices::GetChildDevice(_In_ DEVINST ParentDevInst,
                         _Out_ PDEVINST DevInst)
{
    CONFIGRET cr;

    cr = CM_Get_Child(DevInst,
                      ParentDevInst,
                      0);
    return (cr == CR_SUCCESS);
}

BOOL
CDevices::GetSiblingDevice(_In_ DEVINST PrevDevice,
                           _Out_ PDEVINST DevInst)
{
    CONFIGRET cr;

    cr = CM_Get_Sibling(DevInst,
                        PrevDevice,
                        0);
    return (cr == CR_SUCCESS);
}

BOOL
CDevices::EnumDeviceClasses(_In_ ULONG ClassIndex,
                            _Out_ LPWSTR ClassName,
                            _In_ DWORD ClassNameSize,
                            _Out_ LPWSTR ClassDesc,
                            _In_ DWORD ClassDescSize,
                            _Out_ PINT ClassImage,
                            _Out_ LPBOOL IsUnknown,
                            _Out_ LPBOOL IsHidden)
{
    DWORD RequiredSize;
    GUID ClassGuid;
    HKEY hKey;
    CONFIGRET cr;

    ClassName[0] = UNICODE_NULL;
    ClassDesc[0] = UNICODE_NULL;
    *ClassImage = -1;
    *IsUnknown = FALSE;
    *IsHidden = FALSE;

    cr = CM_Enumerate_Classes(ClassIndex,
                              &ClassGuid,
                              0);
    if (cr != CR_SUCCESS)
        return FALSE;

    RequiredSize = MAX_CLASS_NAME_LEN;
    if (SetupDiClassNameFromGuidW(&ClassGuid,
                                  ClassName,
                                  RequiredSize,
                                  &RequiredSize))
    {
        wcscpy_s(ClassName, ClassNameSize, ClassName);
    }

    hKey = SetupDiOpenClassRegKeyExW(&ClassGuid,
                                     MAXIMUM_ALLOWED,
                                     DIOCR_INSTALLER,
                                     NULL,
                                     0);
    if (hKey != INVALID_HANDLE_VALUE)
    {
        DWORD Type = REG_SZ;
        DWORD Size = ClassDescSize;

        (VOID)RegQueryValueExW(hKey,
                               NULL,
                               NULL,
                               &Type,
                               (LPBYTE)ClassDesc,
                               &Size);
    }



    (VOID)SetupDiGetClassImageIndex(&m_ImageListData,
                                    &ClassGuid,
                                    ClassImage);

    *IsUnknown = IsEqualGUID(ClassGuid, GUID_DEVCLASS_UNKNOWN);

    if (IsEqualGUID(ClassGuid, GUID_DEVCLASS_LEGACYDRIVER) ||
        IsEqualGUID(ClassGuid, GUID_DEVCLASS_VOLUME))
    {
        *IsHidden = TRUE;
    }

    return TRUE;
}



/* PRIVATE METHODS ******************************************/

BOOL
CDevices::CreateImageList()
{
    HBITMAP hRootImage;
    BOOL bSuccess;

    if (m_ImageListData.ImageList != NULL)
        return TRUE;

    m_ImageListData.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);
    bSuccess = SetupDiGetClassImageList(&m_ImageListData);
    if (bSuccess == FALSE) return FALSE;

    hRootImage = LoadBitmapW(g_hInstance,
                             MAKEINTRESOURCEW(IDB_ROOT_IMAGE));
    if (hRootImage == NULL) return FALSE;

    ImageList_Add(m_ImageListData.ImageList,
                  hRootImage,
                  NULL);

    DeleteObject(hRootImage);

    return bSuccess;
}
