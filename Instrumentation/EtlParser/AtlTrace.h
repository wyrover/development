#pragma once
#include "BaseTrace.h"
#include "SqLite.h"

class CAtlTrace :
    public CBaseTrace
{
    CSqLite m_DataBase;
    HANDLE m_hFile;

private:
    BOOL OutputHeader(
        __in PEVENT_RECORD Event
        );

    BOOL OutputEvent(
        __in PEVENT_RECORD Event,
        __in PTRACE_EVENT_INFO EventInfo
        );

public:
    CAtlTrace(void);
    virtual ~CAtlTrace(void);

    DWORD InitializeTrace(
        __in PDUMP_INFO DumpInfo);
};

