#pragma once
class CEventTrace
{
private:
    wstring m_TraceName;
    wstring m_RootDirectory;
    GUID m_TraceSession;
    vector<GUID> m_TraceProviders; //enabletrace
    EVENT_TRACE_PROPERTIES m_EventTraceProperties;
    TRACEHANDLE m_TraceHandle;

public:
    CEventTrace(void);
    ~CEventTrace(void);

    DWORD Create(
        _In_z_ wstring TraceName,
        _In_z_ wstring RootDirectory
        );

    DWORD AddTraceProvider(
        _In_ GUID &TraceGuid
        );

    DWORD DeleteTraceProvider(
        _In_ GUID &TraceGuid
        );
};

