#pragma once

#ifdef __cplusplus
extern "C" {
#endif


enum StreamMode
{
    File,
    RealTime,
    FileAndRealTime
};

DWORD CreateTraceSession(
    _In_z_ LPWSTR TraceName,
    _In_z_ LPWSTR TraceDirectory,
    _Out_ LPHANDLE TraceHandle
    );

DWORD AddTraceProvider(
    _In_ HANDLE TraceHandle,
    _In_ LPGUID ProviderGuid,
    _In_ ULONGLONG KeywordsAny,
    _In_ ULONGLONG KeywordsAll,
    _In_ DWORD Level,
    _In_ DWORD Properties,
    _In_z_ LPWSTR Filter
    );

DWORD DeleteTraceProvider(
    _In_ HANDLE TraceHandle,
    _In_ LPGUID ProviderGuid
    );

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

DWORD StartTraceSession(
    _In_ HANDLE TraceHandle
    );

DWORD StopTraceSession(
    _In_ HANDLE TraceHandle
    );

#ifdef __cplusplus
}
#endif
