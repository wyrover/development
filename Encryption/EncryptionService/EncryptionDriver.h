#pragma once
#include "FilterDriver.h"


class CEncryptionDriver : CUcaFilterDriver
{
    HANDLE m_hListenThread;
    HANDLE m_hTerminateEvent;
    BOOL m_bListenForMessages;

public:
    CEncryptionDriver(void);
    ~CEncryptionDriver(void);

    DWORD Initialize(
        );

    DWORD Uninitialize(
        );

private:
    static UINT WINAPI ListenThread(
        LPVOID lpParam
        );

    DWORD WaitForNotification(
        _Out_ /*PUCA_NOTIFICATION*/ PVOID *Notification,
        _Out_ LPDWORD lpdwBufferSize
        );
};

