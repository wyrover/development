#pragma once
#include <FltUser.h>
#include "LookasideList.h"
//#include "UcaFltDriverShr.h"


typedef DWORD (WINAPI *CALLBACK_ROUTINE)(LPVOID);


extern HANDLE TraceHandle;


class CUcaFilterDriver
{
    HANDLE m_hPort;
    HANDLE m_hTerminate;
    WCHAR m_szDriverName[MAX_PATH];
    WCHAR m_szDriverPath[MAX_PATH];
    CLookasideList *m_FilterMessageList;

protected:
    CUcaFilterDriver(LPWSTR lpDriverName);
    ~CUcaFilterDriver(void);

    DWORD Load();
    DWORD Unload();
    DWORD Connect();
    DWORD Disconnect();

    DWORD SendFilterMessage(
        _In_reads_bytes_(dwInBufferSize) LPVOID lpInBuffer,
        _In_ DWORD dwInBufferSize,
        _Out_writes_bytes_to_opt_(dwOutBufferSize,*lpBytesReturned) LPVOID lpOutBuffer,
        _In_ DWORD dwOutBufferSize,
        _Out_ LPDWORD lpBytesReturned
        );

    DWORD GetFilterMessage(
        _Out_writes_bytes_(dwMessageBufferSize) PVOID lpMessageBuffer,
        _In_ DWORD dwMessageBufferSize,
        _Out_ LPDWORD lpBytesReceived,
        _Inout_opt_ LPOVERLAPPED lpOverlapped
        );

    DWORD ReplyFilterMessage(
        _In_reads_bytes_(dwReplyBufferSize) PVOID lpReplyBuffer,
        _In_ DWORD dwReplyBufferSize
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
