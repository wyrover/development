#pragma once
#include "stdafx.h"
#include "TraceManager.h"

DWORD CreateTraceSession(
        _In_z_ LPWSTR TraceName,
        _In_z_ LPWSTR TraceDirectory,
        _Out_ LPHANDLE TraceHandle)
{
    if (TraceName == NULL || TraceDirectory == NULL)
        return ERROR_INVALID_PARAMETER;

    CTraceManager *TraceManager = new CTraceManager();
    DWORD dwError = TraceManager->Create(TraceName, TraceDirectory);
    if (dwError == ERROR_SUCCESS)
    {
        *TraceHandle = (LPHANDLE)TraceManager;
    }

    return dwError;
}

DWORD AddTraceProvider(_In_ HANDLE TraceHandle,
        _In_ LPGUID ProviderGuid,
        _In_ ULONGLONG KeywordsAny,
        _In_ ULONGLONG KeywordsAll,
        _In_ DWORD Level,
        _In_ DWORD Properties,
        _In_z_ LPWSTR Filter)
{
    if (TraceHandle == NULL)
        return ERROR_INVALID_PARAMETER;

    CTraceManager *TraceManager = (CTraceManager *)TraceHandle;
    return TraceManager->AddTraceProvider(*ProviderGuid,
                                          KeywordsAny,
                                          KeywordsAll,
                                          Level,
                                          Properties,
                                          Filter);
}

DWORD DeleteTraceProvider(_In_ HANDLE TraceHandle,
                          _In_ LPGUID ProviderGuid)
{
    if (TraceHandle == NULL)
        return ERROR_INVALID_PARAMETER;

    CTraceManager *TraceManager = (CTraceManager *)TraceHandle;
    return TraceManager->DeleteTraceProvider(*ProviderGuid);

}

DWORD SetTraceBuffers(
        _In_ DWORD BufferSize,
        _In_ DWORD MinimumBuffers,
        _In_ DWORD MaximumBuffers
        );

DWORD SetStreamMode(
        _In_ StreamMode eStreamMode,
        _In_z_ LPWSTR LogFileame,
        _In_z_ LPWSTR LogFileDirectory,
        _In_ BOOL bAppend,
        _In_ BOOL Circular
        );

DWORD StartTraceSession(_In_ HANDLE TraceHandle)
{
    CTraceManager *TraceManager = (CTraceManager *)TraceHandle;

    DWORD dwError = TraceManager->StartTraceSession();
    return dwError;
}

DWORD StopTraceSession(_In_ HANDLE TraceHandle)
{
    CTraceManager *TraceManager = (CTraceManager *)TraceHandle;

    DWORD dwError = TraceManager->StopTraceSession();
    return dwError;
}