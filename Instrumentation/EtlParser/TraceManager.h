#pragma once

typedef struct _DUMP_INFO
{
    WCHAR EtlFileName[MAX_PATH];
    WCHAR OutputFileName[MAX_PATH];
    ULONG DumpType;
    ULONG Flags;
    ULONG SessionId;

} DUMP_INFO, *PDUMP_INFO;

#define DUMP_XML    1
#define DUMP_TXT    2
#define DUMP_SQL    3

#define DUMP_UCA        0x01
#define DUMP_SESSION    0x02
#define DUMP_PROCNAME   0x04
#define DUMP_MODULENAME 0x04

DWORD
WINAPI
DecodeFile(
    __in PDUMP_INFO DumpInfo,
    __inout PULONG BufferCount,
    __inout PULONGLONG EventProcCount,
    __inout PULONGLONG EventOutCount
    );

