#pragma once

enum StreamMode
{
    File,
    RealTime,
    FileAndRealTime,
    Buffered
};

struct FileInformation
{
    wstring LogFileame;
    wstring LogFileDirectory;
    bool Append;
    bool Circular;
};


class CEventTrace
{
private:
    wstring m_TraceName;
    wstring m_RootDirectory;
    GUID m_TraceSession;
    vector<GUID> m_TraceProviders; //enabletrace
    PEVENT_TRACE_PROPERTIES m_EventTraceProperties;
    TRACEHANDLE m_TraceHandle;
    DWORD m_LoggerThreadId;

public:
    CEventTrace(void);
    ~CEventTrace(void);

    DWORD Create(
        _In_z_ wstring TraceName,
        _In_z_ wstring RootDirectory
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
};

