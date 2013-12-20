#pragma once

enum StreamMode
{
    File,
    RealTime,
    FileAndRealTime
};

struct FileInformation
{
    wstring LogFileame;
    wstring LogFileDirectory;
    bool Append;
    bool Circular;
};

class TraceProvider;


class CTraceManager
{
private:
    wstring m_TraceName;
    GUID m_TraceSessionGuid;
    std::vector<TraceProvider *> m_TraceProviders; //enabletrace
    PEVENT_TRACE_PROPERTIES m_EventTraceProperties;
    TRACEHANDLE m_TraceHandle;
    DWORD m_LoggerThreadId;
    FileInformation m_FileInformation;

public:
    CTraceManager(void);
    ~CTraceManager(void);

    DWORD Create(
        _In_z_ wstring TraceName
        );

    DWORD AddTraceProvider(
        _In_ GUID &ProviderGuid,
        _In_ DWORD KeywordsAny,
        _In_ DWORD KeywordsAll,
        _In_ DWORD Level,
        _In_ DWORD Properties,
        _In_z_ wstring Filter
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

