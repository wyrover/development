#pragma once
class CTraceProvider
{
    std::wstring m_TraceName;
    GUID m_TraceGuid;
    ULONGLONG m_MatchAnyKeyword;
    ULONGLONG m_MatchAllKeyword;
    UCHAR m_Level;
    DWORD m_Properties;
    std::wstring m_Filter;

public:
    CTraceProvider(void) :
        m_MatchAnyKeyword(0),
        m_MatchAllKeyword(0),
        m_Level(0),
        m_Properties(0)
    {
    }
    CTraceProvider(_In_z_ std::wstring TraceName,
                   _In_ GUID TraceGuid) :
        m_TraceName(TraceName),
        m_MatchAnyKeyword(0),
        m_MatchAllKeyword(0),
        m_Level(0),
        m_Properties(0)
    {
        CopyMemory(&m_TraceGuid, &TraceGuid, sizeof(GUID));
    }
    ~CTraceProvider(void);

    void SetTraceName(_In_z_ std::wstring TraceName)
    {
        m_TraceName = TraceName;
    }
    std::wstring GetTraceName()
    {
        return m_TraceName;
    }

    void SetTraceGuid(_In_ GUID &TraceGuid)
    {
        CopyMemory(&m_TraceGuid, &TraceGuid, sizeof(GUID));
    }
    LPCGUID GetTraceGuid()
    {
        return &m_TraceGuid;
    }

    void SetKeywordsAny(_In_ ULONGLONG MatchAllKeyword)
    {
        m_MatchAnyKeyword = MatchAllKeyword;
    }
    ULONGLONG GetKeywordsAny()
    {
        return m_MatchAnyKeyword;
    }

    void SetKeywordsAll(_In_ ULONGLONG MatchAllKeyword)
    {
        m_MatchAllKeyword = MatchAllKeyword;
    }
    ULONGLONG GetKeywordsAll()
    {
        return m_MatchAllKeyword;
    }

    void SetLevel(_In_ UCHAR Level)
    {
        m_Level = Level;
    }
    UCHAR GetLevel()
    {
        return m_Level;
    }
};
