#include "stdafx.h"
#include "EncryptionDriver.h"
#include "DriverShared.h"
#include "UcaRtl.h"

CEncryptionDriver::CEncryptionDriver(void) :
    CUcaFilterDriver(ENC_DRIVER_NAME),
    m_hTerminateEvent(NULL),
    m_hListenThread(NULL),
    m_bListenForMessages(FALSE)
{
}


CEncryptionDriver::~CEncryptionDriver(void)
{
}

DWORD CEncryptionDriver::Initialize(void)
{
    DWORD dwError;

    dwError = EnablePrivilegeInCurrentProcess(SE_LOAD_DRIVER_NAME, TRUE);
    if (dwError != ERROR_SUCCESS)
    {
        //TRACE_ERROR(TraceHandle, "Failed to set \'%S\' : %lu", SE_LOAD_DRIVER_NAME, dwError);
        return dwError;
    }
    __debugbreak();
    dwError = Load();
    if (dwError != ERROR_SUCCESS)
        return dwError;

    dwError = Connect();
    if (dwError != ERROR_SUCCESS)
    {
        Unload();
        return dwError;
    }

    m_hTerminateEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

    /* Create the listener thread */
    m_hListenThread = (HANDLE)_beginthreadex(NULL,
                                             0,
                                             ListenThread,
                                             (LPVOID)this,
                                             0,
                                             NULL);
    if (!m_hListenThread)
    {
        //TRACE_ERROR(TraceHandle, "Failed to create the thread : %lu (errno)", errno);
    }

    return dwError;
}

DWORD CEncryptionDriver::Uninitialize(
    void
        )
{
    Disconnect();
    Unload();
    return 0;
}



UINT WINAPI
CEncryptionDriver::ListenThread(LPVOID lpParam)
{
    CEncryptionDriver* pThis = reinterpret_cast<CEncryptionDriver *>(lpParam);
    PCUSPIS_ENCRYPT_NOTIFICATION EncryptNotification;
    CUSPIS_ENCRYPT_NOTIFICATION EncryptReply;
    OVERLAPPED OverlappedGet;
    DWORD BytesReceieved;
    DWORD dwError;

    //TRACE_ENTER(TraceHandle);
    __debugbreak();

    ZeroMemory(&OverlappedGet, sizeof(OVERLAPPED));
    OverlappedGet.hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

    EncryptNotification = (PCUSPIS_ENCRYPT_NOTIFICATION)HeapAlloc(GetProcessHeap(), 0, 1024);

    pThis->m_bListenForMessages = TRUE;

    while (pThis->m_bListenForMessages)
    {
        dwError = pThis->GetFilterMessage(EncryptNotification, 1024, &BytesReceieved, &OverlappedGet);
        if (dwError != ERROR_SUCCESS)
            break;

        EncryptNotification->File = (LPWSTR)(EncryptNotification + 1);

        // handle it
        ZeroMemory(&EncryptReply, sizeof(CUSPIS_ENCRYPT_NOTIFICATION));
        EncryptReply.Cookie = EncryptNotification->Cookie;
        //pThis->ReplyFilterMessage(&EncryptReply, sizeof(CUSPIS_ENCRYPT_NOTIFICATION));
    }

    HeapFree(GetProcessHeap(), 0, EncryptNotification);

    CloseHandle(OverlappedGet.hEvent);

    //TRACE_INFO(TraceHandle, "Exiting thread %lu", GetCurrentThreadId());
    //TRACE_EXIT(TraceHandle);

    return 0;
}


DWORD
CEncryptionDriver::WaitForNotification(_Out_ /*PUCA_NOTIFICATION*/ PVOID *Notification,
                                       _Out_ LPDWORD lpdwBufferSize)
{
#if 0
    UCA_FLT_QUERY_MESSAGE QueryMessage;
    UCA_FLT_GET_MESSAGE RequestMessage;
    UCA_NOTIFICATION_INFO NotificationInfo;
    DWORD BytesReturned;
    HRESULT hResult;
    DWORD dwError;

    TRACE_ENTER(TraceHandle);

    if (!Notification || !lpdwBufferSize) return ERROR_INVALID_PARAMETER;

    *Notification = NULL;
    *lpdwBufferSize = 0;

    /* Request the next message info */
    QueryMessage.Header.Message = FLT_GET_NEXT_MESSAGE;

    /* This call blocks, so set the cancel IO event */
    QueryMessage.Header.hCancelIo = m_hCancelIo;

    /* Send the message to the driver */
    hResult = FilterSendMessage(m_hPort,
                                &QueryMessage,
                                sizeof(UCA_FLT_QUERY_MESSAGE),
                                &NotificationInfo,
                                sizeof(UCA_NOTIFICATION_INFO),
                                &BytesReturned);
    if (hResult != S_OK)
    {
        TRACE_ERROR(TraceHandle, "Failed to send a query message to the driver : %X", hResult);
        return SCODE_CODE(hResult);
    }

    TRACE_INFO(TraceHandle, "Handle is %X", NotificationInfo.NotificationHandle);
    TRACE_INFO(TraceHandle, "Allocating %lu bytes", NotificationInfo.NotificationSize);

    /* Allocate memory to hold the data */
    *Notification = (PUCA_NOTIFICATION)HeapAlloc(GetProcessHeap(),
                                                 0,
                                                 NotificationInfo.NotificationSize);
    if (*Notification == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    /* Set the header to actually get the message */
    RequestMessage.Header.Message = FLT_GET_MESSAGE;
    RequestMessage.Header.hCancelIo = NULL;

    /* Set the handle to the data we want */
    RequestMessage.NotificationHandle = NotificationInfo.NotificationHandle;

    /* Send the message to the driver */
    hResult = FilterSendMessage(m_hPort,
                                &RequestMessage,
                                sizeof(UCA_FLT_GET_MESSAGE),
                                *Notification,
                                NotificationInfo.NotificationSize,
                                &BytesReturned);
    if (hResult == S_OK)
    {
        /* Store the buffer size */
        *lpdwBufferSize = NotificationInfo.NotificationSize;
        dwError = ERROR_SUCCESS;
    }
    else
    {
        /* Cleanup */
        TRACE_ERROR(TraceHandle, "Failed to send a get message to the driver : %X", hResult);
        dwError = SCODE_CODE(hResult);
        HeapFree(GetProcessHeap(), 0, *Notification);
        *Notification = NULL;
    }

    TRACE_EXIT(TraceHandle);
#endif
    return 0;//dwError;
}
