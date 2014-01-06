/*
 * PROJECT:         Unified Client Architecture, Event Engine
 * COPYRIGHT:       AppSense Ltd, 2011
 * PURPOSE:         Implements the interface for the filter driver
 * PROGRAMMERS:     Ged Murphy (gerard.murphy@appsense.com)
 *
 */

#include "StdAfx.h"
//#include "EventEngine.h"
#include <Fltuser.h>
#include "UcaFilterDriver.h"
#include "DriverShared.h"
//#include "UcaLogging.h"
//#include "UcaDebug.h"

/* DATA *******************************************************/


/* PUBLIC METHODS *********************************************/

CUcaFilterDriver::CUcaFilterDriver(void) :
    m_hPort(NULL)
{
    /* Store the driver name */
    wcscpy_s(m_szDriverName, MAX_PATH, ENC_DRIVER_NAME);
}

CUcaFilterDriver::~CUcaFilterDriver(void)
{
    //UCAASSERT(m_hPort == NULL);
}

DWORD
CUcaFilterDriver::Start()
{
    HRESULT hResult;
    DWORD dwError = 0;

    //TRACE_ENTER(TraceHandle);

    /* Load the filter driver */
    hResult = FilterLoad(&m_szDriverName[1]);
    if (hResult == S_OK)
    {
        dwError = ERROR_SUCCESS;
    }
    else
    {
        /* Get the error code */
        dwError = SCODE_CODE(hResult);

        /* Check if it was alrweady running */
        if (dwError == ERROR_SERVICE_ALREADY_RUNNING)
        {
            /* Set success */
            dwError = ERROR_SUCCESS;
        }
        else
        {
            //TRACE_ERROR(TraceHandle, "Failed to start the filter driver : %lu (%X)", dwError, hResult);
        }
    }

    //TRACE_EXIT(TraceHandle);

    return dwError;
}

DWORD
CUcaFilterDriver::Stop()
{
    HRESULT hResult;
    DWORD dwError = ERROR_SUCCESS;

    /* Unload the filter driver */
    hResult = FilterUnload(&m_szDriverName[1]);
    if (hResult != S_OK)
    {
        switch (hResult)
        {
        case ERROR_FLT_FILTER_NOT_FOUND:
            dwError = ERROR_NOT_FOUND;
            break;

        default:
            dwError = SCODE_CODE(hResult);
            break;
        }

        //TRACE_ERROR(TraceHandle, "Failed to unload the filter driver : %lu (%X)", dwError, hResult);
    }

    return dwError;
}

DWORD
CUcaFilterDriver::Connect(void)
{
    HRESULT hResult;
    DWORD dwError = ERROR_SUCCESS;

   // TRACE_ENTER(TraceHandle);

    /* Check we aren't already connected */
    if (m_hPort) return ERROR_SUCCESS;
    //__debugbreak();
    /* Connect to the filter driver */
    hResult = FilterConnectCommunicationPort(m_szDriverName,
                                             0,
                                             NULL,
                                             0,
                                             NULL,
                                             &m_hPort);
    if (hResult != S_OK)
    {
        switch (hResult)
        {
        default:
            dwError = SCODE_CODE(hResult);
            break;
        }

        //TRACE_ERROR(TraceHandle, "Failed to connect to the filter driver : %lu", dwError);

        /* Reset the handle */
        if (m_hPort == INVALID_HANDLE_VALUE)
            m_hPort = NULL;
    }

    //TRACE_EXIT(TraceHandle);

    return dwError;
}

DWORD
CUcaFilterDriver::Disconnect()
{
    DWORD dwError = ERROR_SUCCESS;

    //TRACE_ENTER(TraceHandle);

    /* Check that the port is valid */
    if (m_hPort == NULL) return ERROR_SUCCESS;

    /* Close the port handle */
    if (CloseHandle(m_hPort))
    {
        m_hPort = NULL;
    }
    else
    {
        dwError = GetLastError();
        //TRACE_ERROR(TraceHandle, "Failed to close the filter driver handle %X : %lu", m_hPort, dwError);
    }

    //TRACE_EXIT(TraceHandle);

    return dwError;
}


/* PRIVATE METHODS *******************************************************/

BOOL
CUcaFilterDriver::IsPathDirectory(_In_ LPWSTR lpPath)
{
    WIN32_FILE_ATTRIBUTE_DATA FileAttributeData;
    BOOL bSuccess;

    /* Get the file attributes */
    bSuccess = GetFileAttributesExW(lpPath, GetFileExInfoStandard, &FileAttributeData);
    if (bSuccess)
    {
        /* Check if it has the directory flag */
        if (FileAttributeData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            return TRUE;
    }

    return FALSE;
}

DWORD
CUcaFilterDriver::GetVolumeMountPoint(_In_z_ LPWSTR lpPath,
                                      _Out_ LPWSTR *lpMountPoint)
{
    DWORD PathLength;
    BOOL bSuccess;
    DWORD dwError;


    /* Alloc enough memory to store the volume mount point */
    PathLength = wcslen(lpPath);
    *lpMountPoint = (LPWSTR)HeapAlloc(GetProcessHeap(),
                                      0,
                                      (PathLength + 1) * sizeof(WCHAR));
    if (*lpMountPoint == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    /* Get the volume mount point */
    bSuccess = GetVolumePathNameW(lpPath, *lpMountPoint, PathLength + 1);
    if (bSuccess == TRUE)
    {
        dwError = ERROR_SUCCESS;
    }
    else
    {
        dwError = GetLastError();
        HeapFree(GetProcessHeap(), 0, *lpMountPoint);
    }

    return dwError;
}

DWORD
CUcaFilterDriver::GetVolumeNameFromMountPoint(_In_z_ LPWSTR lpMountPoint,
                                              _Out_ LPWSTR *lpVolumeName)
{
    return 0;
}

DWORD
CUcaFilterDriver::DosPathToDevicePath(_In_z_ LPWSTR lpDosPath,
                                      _Out_ LPWSTR *lpDevicePath,
                                      _Out_ LPWSTR *lpFolderPath)
{
    WCHAR szVolumeName[MAX_PATH]; // vol name + guid should be < 256
    LPWSTR lpVolumeMountPoint, DeviceName, ptr;
    DWORD PathLen, DeviceLen = 128;
    BOOL bRemovedSeperator = FALSE;
    BOOL bSuccess;
    DWORD dwError;

    /* Get the mount point for this path */
    dwError = GetVolumeMountPoint(lpDosPath, &lpVolumeMountPoint);
    if (dwError != ERROR_SUCCESS)
    {
        return dwError;
    }

    /* Translate the mount point into a volume name (\\?\Volume\{guid}) */
    bSuccess = GetVolumeNameForVolumeMountPointW(lpVolumeMountPoint,
                                                 szVolumeName,
                                                 MAX_PATH);
    if (bSuccess == FALSE)
    {
        dwError = GetLastError();
        HeapFree(GetProcessHeap(), 0, lpVolumeMountPoint);
        return dwError;
    }

    /* Check for extended paths */
    if (szVolumeName[0] == L'\\' && szVolumeName[1] == L'\\' &&
        szVolumeName[2] == L'?' && szVolumeName[3] == L'\\')
    {
        /* Skip this */
        ptr = &szVolumeName[4];
    }
    else
    {
        ptr = szVolumeName;
    }

    /* Remove any trailing backslash */
    PathLen = wcslen(szVolumeName);
    if (szVolumeName[PathLen - 1] == L'\\')
    {
        szVolumeName[PathLen - 1] = UNICODE_NULL;
        bRemovedSeperator = TRUE;
    }

    do
    {
        /* Allocate memory to hold the device name */
        DeviceName = (LPWSTR)HeapAlloc(GetProcessHeap(),
                                       0,
                                       DeviceLen * sizeof(WCHAR));
        if (DeviceName == NULL) return ERROR_NOT_ENOUGH_MEMORY;

        /* Now get the NT device name */
        if (QueryDosDeviceW(ptr, DeviceName, DeviceLen-1) > 0)
        {
            dwError = ERROR_SUCCESS;
        }
        else
        {
            dwError = GetLastError();

            /* Increase the buffer if it was too small */
            if (dwError == ERROR_INSUFFICIENT_BUFFER)
                DeviceLen *= 2;
        }

    } while (dwError == ERROR_INSUFFICIENT_BUFFER);

    /* Put the trailing backslash back */
    if (bRemovedSeperator)
    {
        szVolumeName[PathLen - 1] = L'\\';
    }

    /* Check if we successfully got the device name */
    if (dwError == ERROR_SUCCESS)
    {
        /* Get the length of the mount point / drive */
        PathLen = wcslen(lpVolumeMountPoint);

        /* Point to the start of the folder path */
        ptr = &lpDosPath[PathLen];

        /* Calculate the required len for device\path */
        PathLen = wcslen(DeviceName) + wcslen(ptr) + 2;

        /* Alloc memory to hold the device path + folder path */
        *lpDevicePath = (LPWSTR)HeapAlloc(GetProcessHeap(),
                                              0,
                                              PathLen * sizeof(WCHAR));
        if (*lpDevicePath)
        {
            /* Create the device name path */
            swprintf_s(*lpDevicePath,
                       PathLen,
                       L"%s\\%s",
                       DeviceName,
                       ptr);

            /* Point to the start of the folder path in the device path */
            *lpFolderPath = *lpDevicePath + wcslen(DeviceName);
        }
    }

    /* Cleanup */
    HeapFree(GetProcessHeap(), 0, lpVolumeMountPoint);
    HeapFree(GetProcessHeap(), 0, DeviceName);

    return dwError;
}



DWORD
CUcaFilterDriver::WaitForNotification(_Out_ /*PUCA_NOTIFICATION*/ PVOID *Notification,
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
