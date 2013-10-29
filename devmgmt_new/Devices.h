#pragma once

#define ROOT_NAME_SIZE  MAX_COMPUTERNAME_LENGTH + 1

class CDevices
{
private:
    SP_CLASSIMAGELIST_DATA m_ImageListData;
    BOOL m_bInitialized;

    DEVINST m_RootDevInst;
    WCHAR m_RootName[ROOT_NAME_SIZE];
    INT m_RootImageIndex;

public:
    CDevices(void);
    ~CDevices(void);

    BOOL Initialize(
        );

    BOOL Uninitialize(
        );

    BOOL GetDeviceTreeRoot(
        _Out_ LPWSTR RootName,
        _In_ DWORD RootNameSize,
        _Out_ PINT RootImageIndex
        );

    BOOL GetChildDevice(
        _In_ DEVINST ParentDevInst,
        _Out_ PDEVINST DevInst
        );

    BOOL GetSiblingDevice(
        _In_ DEVINST PrevDevice,
        _Out_ PDEVINST DevInst
        );

    BOOL EnumClasses(
        _In_ ULONG ClassIndex,
        _Out_ LPWSTR ClassName,
        _In_ DWORD ClassNameSize,
        _Out_ LPWSTR ClassDesc,
        _In_ DWORD ClassDescSize,
        _Out_ PINT ClassImage,
        _Out_ LPBOOL IsUnknown,
        _Out_ LPBOOL IsHidden
        );

    BOOL EnumClassDevices(
        _In_ ULONG ClassIndex,
        _In_ DWORD MemberIndex,
        _Out_ LPBOOL HasChild,
        _Out_ LPTSTR DeviceName,
        _In_ DWORD DeviceNameSize,
        _Out_ LPTSTR *DeviceID
        );

    BOOL GetDeviceStatus(
        _In_ LPWSTR DeviceId,
        _Out_ PULONG pulStatus,
        _Out_ PULONG pulProblemNumber
        );
  

    HIMAGELIST GetImageList() { return m_ImageListData.ImageList; }

private:
    BOOL CreateImageList(
        );

    BOOL CreateRootDevice(
        );

    DWORD ConvertResourceDescriptorToString(
        _Inout_z_ LPWSTR ResourceDescriptor
        );
};

