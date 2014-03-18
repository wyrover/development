#pragma once
//#include <TraceManagerLib.h>

struct FileInformation
{
    std::wstring LogFileame;
    std::wstring LogFileDirectory;
    bool Append;
    bool Circular;
};


class CTraceProvider
{
private:
    std::wstring m_TraceName;
    GUID m_TraceSessionGuid;
    //std::vector<TraceProvider *> m_TraceProviders; //enabletrace
    PEVENT_TRACE_PROPERTIES m_EventTraceProperties;
    TRACEHANDLE m_TraceHandle;
    DWORD m_LoggerThreadId;
    FileInformation m_FileInformation;

public:
    CTraceProvider(void);
    ~CTraceProvider(void);

    DWORD Create(
        _In_z_ LPWSTR TraceName,
        _In_z_ LPWSTR TraceDirectory
        );

    DWORD AddTraceProvider(
        _In_ GUID &ProviderGuid,
        _In_ ULONGLONG KeywordsAny,
        _In_ ULONGLONG KeywordsAll,
        _In_ DWORD Level,
        _In_ DWORD Properties,
        _In_z_ LPWSTR Filter
        );

    DWORD DeleteTraceProvider(
        _In_ GUID &ProviderGuid
        );

    DWORD SetTraceBuffers(
        _In_ DWORD BufferSize,
        _In_ DWORD MinimumBuffers,
        _In_ DWORD MaximumBuffers
        );

    DWORD SetStreamMode(
        _In_ StreamMode eStreamMode,
        _In_ FileInformation *fileInformation
        );

    DWORD StartTraceSession();
    DWORD StopTraceSession();

private:
    DWORD GetProvider(
        _In_ GUID &ProviderGuid,
        _In_ bool Remove,
        _Out_ TraceProvider **Provider
        );
};

