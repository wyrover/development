#pragma once
#include "TraceManager.h"

#define DUMP_XML    1
#define DUMP_TXT    2
#define DUMP_SQL    3


class CBaseTrace
{
protected:
    PROCESSING_CONTEXT m_LogContext;
    EVENT_TRACE_LOGFILEW m_LogFile;
    PDUMP_INFO m_pDumpInfo;

private:
    static ULONG WINAPI BufferCallback(
        __in PEVENT_TRACE_LOGFILEW LogFile
        );

    static VOID WINAPI EventCallback(
        __in PEVENT_RECORD Event
        );

    DWORD GetTraceEventInfo(
        __in PEVENT_RECORD Event,
        __out PTRACE_EVENT_INFO* EventInfo
        );

    VOID
    SaveReferenceValues(
        __in PEVENT_PROPERTY_INFO Property,
        __in USHORT PropertyIndex,
        __in PBYTE Data,
        __inout PPROCESSING_DATA_CONTEXT DataContext
        );

    USHORT GetArrayCount(
        __in PEVENT_PROPERTY_INFO Property,
        __in PULONG ReferenceValues
        );

    USHORT GetPropertyLength(
        __in PEVENT_PROPERTY_INFO Property,
        __in PULONG ReferenceValues
        );

    ULONG GetComplexType(
        __in PEVENT_RECORD Event,
        __in PTRACE_EVENT_INFO EventInfo,
        __in PEVENT_PROPERTY_INFO ComplexProperty,
        __inout PPROCESSING_CONTEXT LogContext
        );

    ULONG GetSimpleType(
        __in PEVENT_RECORD Event,
        __in PTRACE_EVENT_INFO EventInfo,
        __in PEVENT_PROPERTY_INFO Property,
        __in USHORT PropertyIndex,
        __inout PPROCESSING_CONTEXT LogContext
        );

    ULONG FormatProperty(
        __in PEVENT_RECORD Event,
        __in PTRACE_EVENT_INFO EventInfo,
        __in_opt PEVENT_MAP_INFO EventMapInfo,
        __in PEVENT_PROPERTY_INFO Property,
        __in USHORT PropertyLength,
        __in ULONG PropertyIndex,
        __inout PPROCESSING_CONTEXT LogContext
        );

    DWORD HandleEvent(
        __in PEVENT_RECORD Event
        );

    DWORD OutputEventData(
        __in PEVENT_RECORD Event,
        __in PTRACE_EVENT_INFO EventInfo
        );

    virtual BOOL OutputHeader(
        __in PEVENT_RECORD Event
        ) = 0;

    virtual BOOL OutputEvent(
        __in PEVENT_RECORD Event,
        __in PTRACE_EVENT_INFO EventInfo
        ) = 0;

public:
    CBaseTrace(void) : m_pDumpInfo(NULL) {};
    virtual ~CBaseTrace(void) {};

    virtual DWORD InitializeTrace(
        __in PDUMP_INFO DumpInfo
        );

    DWORD ProcessTrace(
        );

    PPROCESSING_CONTEXT GetLogContext() { return &m_LogContext; }

    VOID GetStatistics(
        __inout PULONG BufferCount,
        __inout PULONGLONG EventProcCount,
        __inout PULONGLONG EventOutCount)
    {
        if (BufferCount) *BufferCount = m_LogContext.BufferCount;
        if (EventProcCount) *EventProcCount = m_LogContext.EventProcCount;
        if (EventOutCount) *EventOutCount = m_LogContext.EventOutCount;
    }
};

