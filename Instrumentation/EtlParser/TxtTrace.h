#pragma once
#include "BaseTrace.h"

#define DUMP_UCA    0x01

class CTxtTrace :
    public CBaseTrace
{
    HANDLE m_hFile;

private:
    LPCSTR GetOSString(
        __in USHORT MajorVersion,
        __in USHORT MinorVersion,
        __in USHORT ProductType
        );

    BOOL OutputHeader(
        __in PEVENT_RECORD Event
        );

    BOOL OutputEvent(
        __in PEVENT_RECORD Event,
        __in PTRACE_EVENT_INFO EventInfo
        );

public:
    CTxtTrace(void);
    ~CTxtTrace(void);

    DWORD InitializeTrace(
        __in PDUMP_INFO DumpInfo);
};

