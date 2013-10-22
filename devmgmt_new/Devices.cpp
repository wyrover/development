#include "StdAfx.h"
#include "devmgmt.h"
#include "Devices.h"

CDevices::CDevices(void) :
    m_bInitialized(FALSE),
    m_RootImageIndex(-1)
{
    ZeroMemory(&m_ImageListData, sizeof(SP_CLASSIMAGELIST_DATA));

    m_RootName[0] = UNICODE_NULL;
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

    /* Get the device image list */
    m_ImageListData.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);
    bSuccess = SetupDiGetClassImageList(&m_ImageListData);
    if (bSuccess == FALSE) return FALSE;

    bSuccess = CreateRootDevice();
    if (bSuccess) m_bInitialized = TRUE;

    return m_bInitialized;
}

BOOL
CDevices::Uninitialize()
{
    if (m_ImageListData.ImageList != NULL)
    {
        SetupDiDestroyClassImageList(&m_ImageListData);
        ZeroMemory(&m_ImageListData, sizeof(SP_CLASSIMAGELIST_DATA));
    }

    m_bInitialized = FALSE;

    return TRUE;
}

BOOL
CDevices::GetDeviceTreeRoot(_Out_ LPWSTR RootName,
                            _In_ DWORD RootNameSize,
                            _Out_ PINT RootImageIndex)
{
    wcscpy_s(RootName, RootNameSize, m_RootName);
    *RootImageIndex = m_RootImageIndex;

    return FALSE;
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

    RegCloseKey(hKey);

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


BOOL
CDevices::EnumChildDevices(
        _In_ ULONG ClassIndex,
        _In_ DWORD MemberIndex,
        _Out_ LPBOOL HasChild,
        _Out_ LPTSTR DeviceName,
        _In_ DWORD DeviceNameSize,
        _Out_ LPTSTR *DeviceID)
{
    SP_DEVINFO_DATA DeviceInfoData;
    CONFIGRET cr;
    ULONG Status, ProblemNumber;
    DWORD DevIdSize;

    HDEVINFO hDevInfo = NULL;
    BOOL IsUnknown;

    *HasChild = FALSE;
    *DeviceName = _T('\0');
    *DeviceID = NULL;


    GUID ClassGuid;
    cr = CM_Enumerate_Classes(ClassIndex,
                              &ClassGuid,
                              0);
    if (cr != CR_SUCCESS)
        return FALSE;

    IsUnknown = IsEqualGUID(ClassGuid, GUID_DEVCLASS_UNKNOWN);

    /* Get device info for all devices of a particular class */
    hDevInfo = SetupDiGetClassDevsW(IsUnknown ? NULL : &ClassGuid,
                                    NULL,
                                    NULL,
                                    DIGCF_PRESENT | (IsUnknown ? DIGCF_ALLCLASSES : 0));
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }


    ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if (!SetupDiEnumDeviceInfo(hDevInfo,
                               MemberIndex,
                               &DeviceInfoData))
    {
        //if (GetLastError() == ERROR_NO_MORE_ITEMS)
        //{
        //    return TRUE;
        //}
        //else
            return FALSE;
    }

    //if (!IsEqualGUID(DeviceInfoData.ClassGuid, GUID_NULL))
    //{
    //    /* we're looking for unknown devices and this isn't one */
    //    return FALSE;
    //}

    *HasChild = TRUE;

    /* get the device ID */
    if (!SetupDiGetDeviceInstanceId(hDevInfo,
                                    &DeviceInfoData,
                                    NULL,
                                    0,
                                    &DevIdSize))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            (*DeviceID) = (LPTSTR)HeapAlloc(GetProcessHeap(),
                                            0,
                                            DevIdSize * sizeof(TCHAR));
            if (*DeviceID)
            {
                if (!SetupDiGetDeviceInstanceId(hDevInfo,
                                                &DeviceInfoData,
                                                *DeviceID,
                                                DevIdSize,
                                                NULL))
                {
                    HeapFree(GetProcessHeap(),
                             0,
                             *DeviceID);
                    *DeviceID = NULL;
                }
            }
        }
    }

    if (DeviceID != NULL &&
        _tcscmp(*DeviceID, _T("HTREE\\ROOT\\0")) == 0)
    {
        HeapFree(GetProcessHeap(),
                 0,
                 *DeviceID);
        *DeviceID = NULL;
        return FALSE;
    }

    /* get the device's friendly name */
    if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                          &DeviceInfoData,
                                          SPDRP_FRIENDLYNAME,
                                          0,
                                          (BYTE*)DeviceName,
                                          256,
                                          NULL))
    {
        /* if the friendly name fails, try the description instead */
        SetupDiGetDeviceRegistryProperty(hDevInfo,
                                         &DeviceInfoData,
                                         SPDRP_DEVICEDESC,
                                         0,
                                         (BYTE*)DeviceName,
                                         256,
                                         NULL);
    }

    cr = CM_Get_DevNode_Status_Ex(&Status,
                                  &ProblemNumber,
                                  DeviceInfoData.DevInst,
                                  0,
                                  NULL);
    if (cr == CR_SUCCESS && (Status & DN_HAS_PROBLEM))
    {
        return FALSE;;
    }

    return TRUE;
}


/* PRIVATE METHODS ******************************************/


BOOL
CDevices::CreateRootDevice()
{
    HBITMAP hRootImage = NULL;
    DWORD Size = ROOT_NAME_SIZE;
    BOOL bSuccess = FALSE;
    CONFIGRET cr;

    /* The root name is the computer name */
    (VOID)GetComputerNameW(m_RootName, &Size);

    /* Load the bitmap we'll be using as the root image */
    hRootImage = LoadBitmapW(g_hInstance,
                             MAKEINTRESOURCEW(IDB_ROOT_IMAGE));
    if (hRootImage == NULL) goto Cleanup;

    /* Add this bitmap to the device image list. This is a bit hacky, but it's safe */
    m_RootImageIndex = ImageList_Add(m_ImageListData.ImageList,
                                    hRootImage,
                                    NULL);
    if (m_RootImageIndex == -1)
        goto Cleanup;

    /* Get the root instance */
    cr = CM_Locate_DevNodeW(&m_RootDevInst,
                            NULL,
                            CM_LOCATE_DEVNODE_NORMAL);
    if (cr == CR_SUCCESS)
        bSuccess = TRUE;

Cleanup:
    if (bSuccess == FALSE)
    {
        SetupDiDestroyClassImageList(&m_ImageListData);
        ZeroMemory(&m_ImageListData, sizeof(SP_CLASSIMAGELIST_DATA));
    }

    if (hRootImage) DeleteObject(hRootImage);

    return bSuccess;
}