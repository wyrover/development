#pragma once
#include "TraceProvider.h"


struct FileInformation
{
    std::wstring LogFileame;
    std::wstring LogFileDirectory;
    bool Append;
    bool Circular;
};

enum StreamMode
{
    File,
    RealTime,
    FileAndRealTime
};

class CTraceSession
{
private:
    std::wstring m_TraceName;
    GUID m_TraceSessionGuid;
    std::vector<CTraceProvider *> m_TraceProviders;
    PEVENT_TRACE_PROPERTIES m_EventTraceProperties;
    TRACEHANDLE m_TraceHandle;
    DWORD m_LoggerThreadId;
    FileInformation m_FileInformation;

public:
    CTraceSession(void);
    ~CTraceSession(void);

    HRESULT Create(
        _In_z_ LPWSTR TraceName,
        _In_z_ LPWSTR TraceDirectory
        );

    HRESULT AddTraceProvider(
        _In_ CTraceProvider *Provider
        );

    HRESULT DeleteTraceProvider(
        _In_ GUID &ProviderGuid
        );

    HRESULT SetTraceBuffers(
        _In_ DWORD BufferSize,
        _In_ DWORD MinimumBuffers,
        _In_ DWORD MaximumBuffers
        );

    HRESULT SetStreamMode(
        _In_ StreamMode eStreamMode,
        _In_ FileInformation *fileInformation
        );

    HRESULT StartTraceSession();
    HRESULT StopTraceSession();

private:
    HRESULT GetProvider(
        _In_ GUID &ProviderGuid,
        _In_ bool Remove,
        _Out_ CTraceProvider **Provider
        );
};
