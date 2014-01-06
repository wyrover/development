#pragma once
//#include "UcaFltDriverShr.h"


typedef DWORD (WINAPI *CALLBACK_ROUTINE)(LPVOID);


extern HANDLE TraceHandle;


class CUcaFilterDriver
{
    HANDLE m_hPort;
    WCHAR m_szDriverName[MAX_PATH];
    WCHAR m_szDriverPath[MAX_PATH];

public:
    CUcaFilterDriver(void);
    ~CUcaFilterDriver(void);

    DWORD Start();
    DWORD Stop();

    DWORD Connect();
    DWORD Disconnect();

    DWORD WaitForNotification(
        _Out_ PVOID /*PUCA_NOTIFICATION*/ *lpBuffer,
        _Out_ LPDWORD lpdwBufferSize
        );

private:
    BOOL IsPathDirectory(
        _In_ LPWSTR lpPath
        );

    DWORD GetVolumeMountPoint(
        _In_z_ LPWSTR lpPath,
        _Out_ LPWSTR *lpMountPoint
        );

    DWORD GetVolumeNameFromMountPoint(
        _In_z_ LPWSTR lpMountPoint,
        _Out_ LPWSTR *lpVolumeName
        );

    DWORD DosPathToDevicePath(
        _In_z_ LPWSTR lpDosPath,
        _Out_ LPWSTR *lpDevicePath,
        _Out_ LPWSTR *lpFolderPath
        );
};
