#pragma once
class CEventTrace
{
private:
    wstring m_TraceName;
    GUID m_TraceSession;
    vector<GUID> m_TraceGuids; //enabletrace
    EVENT_TRACE_PROPERTIES m_EventTraceProperties;
    TRACEHANDLE m_TraceHandle;

public:
    CEventTrace(void);
    ~CEventTrace(void);
};

