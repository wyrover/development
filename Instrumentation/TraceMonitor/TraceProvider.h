#pragma once

class CTraceProvider
{
public:
    std::wstring ProviderName;
    GUID ProviderGuid;
    ULONGLONG KeywordsAny;
    ULONGLONG KeywordsAll;
    DWORD Level;
    DWORD Properties;
    std::wstring Filter;

    CTraceProvider(
        _In_z_ std::wstring _ProviderName,
        _In_ GUID _ProviderGuid,
        _In_ ULONGLONG _KeywordsAny = 0,
        _In_ ULONGLONG _KeywordsAll = 0,
        _In_ DWORD _Level = 0,
        _In_ DWORD _Properties = 0,
        _In_ std::wstring _Filter = L"") :
        ProviderName(_ProviderName),
        ProviderGuid(_ProviderGuid),
        KeywordsAny(_KeywordsAny),
        KeywordsAll(_KeywordsAll),
        Level(_Level),
        Properties(_Properties),
        Filter(_Filter)
    {
    }
};

#if 0
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
#endif
