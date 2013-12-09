#include <windows.h>
#include <evntrace.h>
#include <initguid.h>


/****************** PRIVATE FUNCTIONS ********************/

static BOOL
SetTraceProperties(_In_z_ LPWSTR lpTraceName,
                   _In_ LPGUID lpTraceGuid,
                   _In_opt_z_ LPWSTR lpEtlPath,
                   _Out_ PEVENT_TRACE_PROPERTIES* ppTraceProperties)
{
    LPWSTR LoggerName, LogFileName;
    DWORD BufferSize = 0;
    DWORD nameSize;
    DWORD pathSize;
    BOOL bRet = FALSE;

    /* Store the buffer sizes */
    if (lpTraceName == NULL) return ERROR_INVALID_PARAMETER;


    nameSize = (DWORD)(wcslen(lpTraceName) + 1) * sizeof(WCHAR);


    if (lpEtlPath)
        pathSize = (DWORD)(wcslen(lpEtlPath) + 1) * sizeof(WCHAR);


    BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + pathSize + nameSize;


    /* Allocate our memory for the trace properties */
    *ppTraceProperties = (PEVENT_TRACE_PROPERTIES)HeapAlloc(GetProcessHeap(),
                                                            HEAP_ZERO_MEMORY,
                                                            BufferSize);
    if (*ppTraceProperties == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    /* Build up the struct */
    ZeroMemory(*ppTraceProperties, BufferSize);
    (*ppTraceProperties)->Wnode.BufferSize = BufferSize;
    (*ppTraceProperties)->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    (*ppTraceProperties)->Wnode.ClientContext = 2; //System Time

    if (lpTraceGuid)
    {
        CopyMemory(&(*ppTraceProperties)->Wnode.Guid, lpTraceGuid, sizeof(GUID));
    }

    (*ppTraceProperties)->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;

    (*ppTraceProperties)->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    (*ppTraceProperties)->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + nameSize;

    /* Set the memory locations where we'll store the strings */
    LoggerName = (LPWSTR)((char *)(*ppTraceProperties) + (*ppTraceProperties)->LoggerNameOffset);
    LogFileName = (LPWSTR)((char *)(*ppTraceProperties) + (*ppTraceProperties)->LogFileNameOffset);

    /* Tag the strings onto the end of the struct */
    CopyMemory(LoggerName, lpTraceName, nameSize);
    if (lpEtlPath)
    {
        CopyMemory(LogFileName, lpEtlPath, pathSize);
    }


    return bRet;
}


/******************* PUBLIC FUNCTIONS *********************/

BOOL
StartEventTrace(LPWSTR lpTraceName,
                GUID traceGuid,
                LPWSTR lpEtlPath,
                DWORD subsystemFlags,
                DWORD actionFlags)
{
    PEVENT_TRACE_PROPERTIES pTraceProperties;
    BOOL bResult = FALSE;

    if (!lpEtlPath)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return bResult;
    }

    /* Fill out the trace properties */
    if (SetTraceProperties(lpTraceName,
                           &traceGuid,
                           lpEtlPath,
                           &pTraceProperties))
    {
        TRACEHANDLE LoggerHandle = 0;
        LPWSTR LoggerName;
        ULONG ret = 0;

        /* Get the name of the logger */
        LoggerName = (LPWSTR)((char *)pTraceProperties + pTraceProperties->LoggerNameOffset);

        /* Register and start the session */
        ret = StartTraceW(&LoggerHandle,
                          LoggerName,
                          pTraceProperties);
        if (ret == ERROR_SUCCESS)
        {
            /* Enable the Wave trace provider */
            ret = EnableTrace(TRUE,
                              subsystemFlags,
                              actionFlags,
                              &pTraceProperties->Wnode.Guid,
                              LoggerHandle);
            if (ret == ERROR_SUCCESS)
            {
                bResult = TRUE;
            }
        }

        HeapFree(GetProcessHeap(),
                 0,
                 pTraceProperties);
    }


    return bResult;
}


BOOL
StopEventTrace(LPWSTR lpTraceName)
{
    PEVENT_TRACE_PROPERTIES pTraceProperties;
    LPWSTR LoggerName;
    ULONG ret = 0;
    BOOL bResult = FALSE;


    /* Fill out the trace properties */
    if (SetTraceProperties(lpTraceName,
                           NULL,
                           NULL,
                           &pTraceProperties))
    {
        /* Get the name of the logger */
        LoggerName = (LPWSTR)((char *)pTraceProperties + pTraceProperties->LoggerNameOffset);

        /* Stop the trace session */
        ret = ControlTraceW(0,
                            LoggerName,
                            pTraceProperties,
                            EVENT_TRACE_CONTROL_STOP);
        if (ret == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }

        HeapFree(GetProcessHeap(),
                 0,
                 pTraceProperties);
    }


    return bResult;
}


PEVENT_TRACE_PROPERTIES
QueryEventTrace(LPWSTR lpTraceName)
{
    PEVENT_TRACE_PROPERTIES pTraceProperties = NULL;
    LPWSTR LoggerName;
    ULONG ret;

    /* Fill out the trace properties */
    if (SetTraceProperties(lpTraceName,
                           NULL,
                           NULL,
                           &pTraceProperties))
    {
        /* Get the name of the logger */
        LoggerName = (LPWSTR)((char *)pTraceProperties + pTraceProperties->LoggerNameOffset);

        /* Get the session properties and statistics */
        ret = ControlTraceW(0,
                            LoggerName,
                            pTraceProperties,
                            EVENT_TRACE_CONTROL_QUERY);
        if (ret != ERROR_SUCCESS)
        {
            /* Failed, free the memory */
            HeapFree(GetProcessHeap(),
                     0,
                     pTraceProperties);

            pTraceProperties = NULL;
        }
    }

    /* Return the session properties and statistics */
    return pTraceProperties;
}


BOOL
UpdateEventTrace(PEVENT_TRACE_PROPERTIES pTraceProperties)
{
    LPWSTR LoggerName;
    BOOL bResult = FALSE;

    if (pTraceProperties)
    {
        /* Get the name of the logger */
        LoggerName = (LPWSTR)((char *)pTraceProperties + pTraceProperties->LoggerNameOffset);

        /* Update the session properties */
        if (ControlTraceW(0,
                          LoggerName,
                          pTraceProperties,
                          EVENT_TRACE_CONTROL_UPDATE) == ERROR_SUCCESS)
        {
            bResult = TRUE;
        }
    }

    return bResult;
}



BOOL
GetEventTraceFlags(LPWSTR lpTraceName,
                   LPDWORD subsystemFlags,
                   LPDWORD actionFlags)
{
    PEVENT_TRACE_PROPERTIES pTraceProperties = NULL;
    DWORD sub = 0, action = 0;
    TRACEHANDLE handle;
    BOOL bResult = FALSE;

    if (!subsystemFlags || !actionFlags)
    {
        return FALSE;
    }

    /* Get the trace properties */
    if ((pTraceProperties = QueryEventTrace(lpTraceName)))
    {
        handle = GetTraceLoggerHandle(pTraceProperties);
        if ((HANDLE)handle != INVALID_HANDLE_VALUE)
        {
            bResult = TRUE;

            /* Get Subsystem flags */
            SetLastError(ERROR_SUCCESS);
            sub = GetTraceEnableFlags(handle);
            if (GetLastError() != ERROR_SUCCESS)
            {
                bResult = FALSE;
            }

            if (bResult)
            {
                /* Get Action flags */
                SetLastError(ERROR_SUCCESS);
                action = GetTraceEnableLevel(handle);
                if (GetLastError() != ERROR_SUCCESS)
                {
                    bResult = FALSE;
                }
            }
        }

        HeapFree(GetProcessHeap(),
                 0,
                 pTraceProperties);
    }

    *subsystemFlags = sub;
    *actionFlags = action;

    return bResult;
}

BOOL
IsEventTraceRunning(LPWSTR lpTraceName)
{
    PEVENT_TRACE_PROPERTIES pTraceProperties;
    BOOL bResult = FALSE;

    if ((pTraceProperties = QueryEventTrace(lpTraceName)))
    {
        bResult = TRUE;

        HeapFree(GetProcessHeap(),
                 0,
                 pTraceProperties);
    }

    return bResult;
}
