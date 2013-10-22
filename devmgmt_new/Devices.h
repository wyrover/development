#pragma once
class CDevices
{
private:
    SP_CLASSIMAGELIST_DATA m_ImageListData;

public:
    CDevices(void);
    ~CDevices(void);

    BOOL GetDeviceTreeRoot(
        _Out_ PDEVINST DevInst
        );

    BOOL GetChildDevice(
        _In_ DEVINST ParentDevInst,
        _Out_ PDEVINST DevInst
        );

    BOOL GetSiblingDevice(
        _In_ DEVINST PrevDevice,
        _Out_ PDEVINST DevInst
        );

    HIMAGELIST GetImageList(
        );

    BOOL EnumDeviceClasses(
        _In_ ULONG ClassIndex,
        _Out_ LPWSTR ClassName,
        _In_ DWORD ClassNameSize,
        _Out_ LPWSTR ClassDesc,
        _In_ DWORD ClassDescSize,
        _Out_ PINT ClassImage,
        _Out_ LPBOOL IsUnknown,
        _Out_ LPBOOL IsHidden
        );

private:
    BOOL CreateImageList(
        );
};

