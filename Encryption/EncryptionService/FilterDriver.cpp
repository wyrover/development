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
#include "FilterDriver.h"

//#include "UcaLogging.h"
//#include "UcaDebug.h"

/* DATA *******************************************************/

#define LIST_BUFFER_SIZE    1024

/* PUBLIC METHODS *********************************************/

CUcaFilterDriver::CUcaFilterDriver(LPWSTR lpDriverName) :
    m_hPort(NULL),
    m_FilterMessageList(nullptr)
{
    /* Store the driver name */
    wcscpy_s(m_szDriverName, MAX_PATH, lpDriverName);
    
    m_FilterMessageList = new CLookasideList(LIST_BUFFER_SIZE);
}

CUcaFilterDriver::~CUcaFilterDriver(void)
{
    //UCAASSERT(m_hPort == NULL);

    delete m_FilterMessageList;
}

DWORD
CUcaFilterDriver::Load()
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
CUcaFilterDriver::Unload()
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

    m_hTerminate = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (m_hTerminate == NULL) return GetLastError();

    /* Connect to the filter driver */
    hResult = FilterConnectCommunicationPort(m_szDriverName,
                                             0,
                                             NULL,
                                             0,
                                             NULL,
                                             &m_hPort);
    if (hResult != S_OK)
    {
        dwError = SCODE_CODE(hResult);

        CloseHandle(m_hTerminate);
        m_hTerminate = NULL;


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

    if (m_hTerminate)
    {
        SetEvent(m_hTerminate);
        CloseHandle(m_hTerminate);
        m_hTerminate = NULL;
    }

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

DWORD
CUcaFilterDriver::SendFilterMessage(_In_reads_bytes_(dwInBufferSize) LPVOID lpInBuffer,
                                    _In_ DWORD dwInBufferSize,
                                    _Out_writes_bytes_to_opt_(dwOutBufferSize,*lpBytesReturned) LPVOID lpOutBuffer,
                                    _In_ DWORD dwOutBufferSize,
                                    _Out_ LPDWORD lpBytesReturned)
{
    HRESULT hResult;

    hResult = FilterSendMessage(m_hPort,
                                lpInBuffer,
                                dwInBufferSize,
                                lpOutBuffer,
                                dwOutBufferSize,
                                lpBytesReturned);
    if (hResult != S_OK)
    {
        //TRACE_ERROR(TraceHandle, "Failed to send a query message to the driver : %X", hResult);
        return SCODE_CODE(hResult);
    }

    return 0;
}

DWORD CUcaFilterDriver::GetFilterMessage(_Out_writes_bytes_(dwMessageBufferSize) PVOID lpMessageBuffer,
                                         _In_ DWORD dwMessageBufferSize,
                                         _Out_ LPDWORD lpBytesReceived,
                                         _Inout_opt_ LPOVERLAPPED lpOverlapped)
{
    PFILTER_MESSAGE_HEADER FilterMessageHeader;
    HANDLE WaitHandles[2];
    DWORD BufferSize;
    BOOL UsingLinkedList;
    HRESULT hResult;
    PBYTE ptr;
    DWORD dwError;

    *lpBytesReceived = 0;

    BufferSize = dwMessageBufferSize + sizeof(FILTER_MESSAGE_HEADER);

    if (BufferSize < LIST_BUFFER_SIZE)
    {
        FilterMessageHeader = (PFILTER_MESSAGE_HEADER)m_FilterMessageList->Allocate();
        UsingLinkedList = TRUE;
    }
    else
    {
        FilterMessageHeader = (PFILTER_MESSAGE_HEADER)HeapAlloc(GetProcessHeap(), 0, BufferSize);
        UsingLinkedList = FALSE;
    }

    if (FilterMessageHeader == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    ZeroMemory(FilterMessageHeader, BufferSize);

    hResult = FilterGetMessage(m_hPort,
                               FilterMessageHeader,
                               BufferSize,
                               lpOverlapped);
    dwError = SCODE_CODE(hResult);

    if (dwError == ERROR_IO_PENDING)
    {
        WaitHandles[0] = lpOverlapped->hEvent; //m_hPort
        WaitHandles[1] = m_hTerminate;

        dwError = WaitForMultipleObjects(2, WaitHandles, FALSE, INFINITE);
        if (dwError == WAIT_OBJECT_0)
        {
            //TRACE_INFO(TraceHandle, "The session %lu client event has fired", m_SessionId);
            if (!GetOverlappedResult(m_hPort, lpOverlapped, lpBytesReceived, FALSE))
            {
                dwError = GetLastError();
            }
        }
        else if (dwError == WAIT_OBJECT_0 + 1)
        {
            //TRACE_ERROR(TraceHandle, "The session %lu client process exited before event fired", m_SessionId);
            dwError = ERROR_OPERATION_ABORTED;

            //CancelIoEx(m_hPort, lpOverlapped);
        }
        else
        {
            /* Did we get a generic fail result? */
            if (dwError == WAIT_FAILED)
            {
                /* Get the real error code */
                dwError = GetLastError();
            }

            //TRACE_ERROR(TraceHandle, "Failed to wait for the client to connect : %lu", dwError);
        }
    }

    if (dwError == ERROR_SUCCESS)
    {
        ptr = (PBYTE)(FilterMessageHeader + 1);
        CopyMemory(lpMessageBuffer, ptr, dwMessageBufferSize);
    }
    else
    {
        //TRACE_ERROR(TraceHandle, "Failed to send a query message to the driver : %X", hResult);
    }

    if (UsingLinkedList)
    {
        m_FilterMessageList->Free(FilterMessageHeader);
    }
    else
    {
        HeapFree(GetProcessHeap(), 0, FilterMessageHeader);
    }

    return dwError;
}

DWORD CUcaFilterDriver::ReplyFilterMessage(_In_reads_bytes_(dwReplyBufferSize) PVOID lpReplyBuffer,
                                           _In_ DWORD dwReplyBufferSize)
{
    PFILTER_REPLY_HEADER FilterReplyHeader;
    DWORD BufferSize;
    BOOL UsingLinkedList;
    HRESULT hResult;
    PBYTE ptr;
    DWORD dwError;

    BufferSize = dwReplyBufferSize + sizeof(FILTER_REPLY_HEADER);

    if (BufferSize < LIST_BUFFER_SIZE)
    {
        FilterReplyHeader = (PFILTER_REPLY_HEADER)m_FilterMessageList->Allocate();
        UsingLinkedList = TRUE;
    }
    else
    {
        FilterReplyHeader = (PFILTER_REPLY_HEADER)HeapAlloc(GetProcessHeap(), 0, BufferSize);
        UsingLinkedList = FALSE;
    }

    if (FilterReplyHeader == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    ZeroMemory(FilterReplyHeader, BufferSize);

    ptr = (PBYTE)(FilterReplyHeader + 1);
    CopyMemory(ptr, lpReplyBuffer, dwReplyBufferSize);

    hResult = FilterReplyMessage(m_hPort,
                                 FilterReplyHeader,
                                 dwReplyBufferSize);
    if (hResult == S_OK)
    {
        dwError = ERROR_SUCCESS;
    }
    else
    {
        //TRACE_ERROR(TraceHandle, "Failed to send a query message to the driver : %X", hResult);
         dwError = SCODE_CODE(hResult);
    }

    if (UsingLinkedList)
    {
        m_FilterMessageList->Free(FilterReplyHeader);
    }
    else
    {
        HeapFree(GetProcessHeap(), 0, FilterReplyHeader);
    }

    return 0;
}

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

//fixme: can't we use FilterGetDosName for this??
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
