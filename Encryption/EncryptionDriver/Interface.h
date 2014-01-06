#pragma once

extern HANDLE TraceHandle;

typedef struct _CLIENT_CONNECTION
{
    LIST_ENTRY ListEntry;
    PVOID ConnectionCookie;
    ULONG RefCount;

    PFLT_PORT ClientPort;
    PEPROCESS UserProcess;
    PETHREAD UserThread;
    KEVENT ClientTerminating;
    BOOLEAN IsListening;


} CLIENT_CONNECTION, *PCLIENT_CONNECTION;

typedef struct _DRIVER_DATA
{
    PDRIVER_OBJECT DriverObject;
    FLT_REGISTRATION FilterRegistration;
    PFLT_FILTER FilterHandle;
    PFLT_PORT ServerPort;

    ULONG Flags;

    ULONG MaxNotificationsToAllocate;
    ULONG NotificationsAllocated;

    //NPAGED_LOOKASIDE_LIST ContextDataLookasideList;

    NPAGED_LOOKASIDE_LIST ClientConnectionsLookasideList;
    EX_PUSH_LOCK ClientConnectionsLock;
    LIST_ENTRY ClientConnections;

} DRIVER_DATA, *PDRIVER_DATA;

extern DRIVER_DATA DriverData;




DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

NTSTATUS
FLTAPI
UcaFilterUnload(
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
);

NTSTATUS
UCaQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
);

NTSTATUS
FLTAPI
UcaConnect(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
);

VOID
FLTAPI
UcaDisconnect(
    _In_opt_ PVOID ConnectionCookie
);

NTSTATUS
FLTAPI
UcaMessage(
    _In_opt_ PVOID ConnectionCookie,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength,*ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
);

NTSTATUS
FLTAPI
UcaInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
);


/* Callbacks.c */

FLT_PREOP_CALLBACK_STATUS
FLTAPI
UcaFltDriverPreOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
FLTAPI
UcaFltDriverPostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    );
