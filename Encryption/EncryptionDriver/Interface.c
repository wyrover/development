/*
 * PROJECT:         Unified Client Architecture, File System Filter Driver
 * COPYRIGHT:       AppSense Ltd, 2013
 * PURPOSE:         Filter manager entry points and handlers
 * PROGRAMMERS:     Ged Murphy (gerard.murphy@appsense.com)
 *
 */

/* INCLUDES ******************************************************************/

#include "EncryptionDriver.h"
#include "Interface.h"
//#include "OutputBuffer.h"
//#include "WatchedObjects.h"
//#include "ClientConnections.h"
#include "Callbacks.h"
#include "Library.h"
//#ifdef UCA_DEBUG
#include "DriverDebug.h"
//#else
//#include "UcaLogging.h"
//#endif


/* DATA *********************************************************************/

#define PtrToBoolean(p)     ((BOOLEAN)(LONG_PTR)(p))

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, UcaFilterUnload)
#pragma alloc_text(PAGE, UcaConnect)
#pragma alloc_text(PAGE, UcaDisconnect)
#pragma alloc_text(PAGE, UcaMessage)
#pragma alloc_text(PAGE, UCaQueryTeardown)
#pragma alloc_text(PAGE, UcaInstanceSetup)
#endif

#define MAX_CLIENT_CONNECTIONS  100

CLIENT_CONNECTION g_ClientConnection;


// {514B23B7-31C5-4560-ADF6-2C41DB6A1C8E}
static const GUID UcaEventEngineGuid = { 0x514b23b7, 0x31c5, 0x4560, { 0xad, 0xf6, 0x2c, 0x41, 0xdb, 0x6a, 0x1c, 0x8e } };
HANDLE TraceHandle = NULL;

DRIVER_DATA DriverData;

extern CONST FLT_CONTEXT_REGISTRATION Contexts[];
extern CONST FLT_OPERATION_REGISTRATION Callbacks[];

CONST FLT_REGISTRATION FilterRegistration =
{
    sizeof(FLT_REGISTRATION),               //  Size
    FLT_REGISTRATION_VERSION,               //  Version
    0,                                      //  Flags
    Contexts,                               //  ContextRegistration
    Callbacks,                              //  Operation callbacks
    UcaFilterUnload,                        //  FilterUnload
    NULL, //UcaInstanceSetup,                       //  InstanceSetup
    NULL,                                   //  InstanceQueryTeardown
    NULL,                                   //  InstanceTeardownStart
    NULL,                                   //  InstanceTeardownComplete
    NULL,                                   //  GenerateFileName
    NULL,                                   //  GenerateDestinationFileName
    NULL,                                   //  NormalizeNameComponent
    NULL, //UcaTransactionNotificationCallback,     //  TransactionNotification
    NULL                                    //  NormalizeNameComponentEx
};


/* FUNCTIONS **********************************************/

static
NTSTATUS
HandleSetNotifyState(_In_ PCLIENT_CONNECTION ClientConnection,
                     _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
                     _In_ ULONG InputBufferLength)
{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(InputBufferLength);

    __try
    {
        /* Set the listen state */
        ClientConnection->IsListening = PtrToBoolean(((PCUSPIS_ENCRYPT_HEADER)InputBuffer)->Buffer);
        Status = STATUS_SUCCESS;
    }
    __except (ExceptionFilter(GetExceptionInformation(), TRUE))
    {
        Status = GetExceptionCode();
    }

    FLT_ASSERT(ClientConnection->IsListening == TRUE ||
               ClientConnection->IsListening == FALSE);

    return Status;
}

static
NTSTATUS
HandleGetNotificationBuffer(_In_ PCLIENT_CONNECTION ClientConnection,
                            _Out_writes_bytes_to_opt_(OutputBufferLength,*ReturnOutputBufferLength) PVOID OutputBuffer,
                            _In_ ULONG OutputBufferLength,
                            _Out_ PULONG ReturnOutputBufferLength)
{
    NTSTATUS Status;

    /* Sanity check */
    if (OutputBuffer == NULL || OutputBufferLength == 0)
        return STATUS_INVALID_PARAMETER;

#if defined(_WIN64)
    if (IoIs32bitProcess(NULL))
    {
        /* Validate alignment for the 32bit process on a 64bit system */
        if (!IS_ALIGNED(OutputBuffer, sizeof(ULONG)))
            return STATUS_DATATYPE_MISALIGNMENT;
    }
    else
    {
#endif
        if (!IS_ALIGNED(OutputBuffer, sizeof(PVOID)))
            return STATUS_DATATYPE_MISALIGNMENT;
#if defined(_WIN64)
    }
#endif

    return STATUS_SUCCESS;
}


//HANDLE ThreadHandle = NULL;
//
//VOID
//EventHandlerThread(__in PVOID pContext)
//{
//    PCUSPIS_ENCRYPT_NOTIFICATION EncryptNotification;
//    ULONG BufferSize;
//    NTSTATUS Status;
//    LARGE_INTEGER x;
//    int i;
//    __debugbreak();
//    for (i = 0; i < 5; i++)
//    {
//        BufferSize = sizeof(CUSPIS_ENCRYPT_NOTIFICATION) + sizeof(L"test");
//        EncryptNotification = (PCUSPIS_ENCRYPT_NOTIFICATION)ExAllocatePoolWithTag(PagedPool,
//                                                                                  BufferSize,
//                                                                                  UCA_POOL_TAG);
//        if (EncryptNotification == NULL) return;
//
//        RtlZeroMemory(EncryptNotification, BufferSize);
//        EncryptNotification->Cookie = EncryptNotification;
//        EncryptNotification->Irp = IRP_MJ_CREATE;
//        EncryptNotification->File = (LPWSTR)(EncryptNotification + 1);
//        RtlCopyMemory(EncryptNotification->File, L"test", sizeof(L"test"));
//        //EncryptNotification->File[FileNameInfo->Name.Length / sizeof(WCHAR)] = UNICODE_NULL;
//
//        Status = FltSendMessage(DriverData.FilterHandle,
//                                &g_ClientConnection.ClientPort,
//                                EncryptNotification,
//                                BufferSize,
//                                NULL,
//                                0,
//                                NULL);
//
//        ExFreePoolWithTag(EncryptNotification, UCA_POOL_TAG);
//
//        x.QuadPart = 100000000I64; // wait 10 seconds
//        KeDelayExecutionThread(KernelMode, FALSE, &x);
//    }
//
//    ZwClose(ThreadHandle);
//}

// This is called when user-mode connects to the server port
NTSTATUS
FLTAPI
UcaConnect(_In_ PFLT_PORT ClientPort,
           _In_opt_ PVOID ServerPortCookie,
           _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
           _In_ ULONG SizeOfContext,
           _Outptr_result_maybenull_ PVOID *ConnectionPortCookie)
{
    NTSTATUS Status;
    __debugbreak();
    PAGED_CODE();
    TRACE_ENTER(TraceHandle);

    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);

    RtlZeroMemory(&g_ClientConnection, sizeof(CLIENT_CONNECTION));
    g_ClientConnection.ClientPort = ClientPort;
    g_ClientConnection.ConnectionCookie = &g_ClientConnection;
    g_ClientConnection.UserProcess = PsGetCurrentProcess();
    g_ClientConnection.UserThread = PsGetCurrentThread();
    g_ClientConnection.RefCount = 1;

    /* Create a terminate event */
    KeInitializeEvent(&g_ClientConnection.ClientTerminating,
                      NotificationEvent,
                      FALSE);

    /* Set the connection cookie to be the address of the client descriptor */
    *ConnectionPortCookie = &g_ClientConnection;

    //Status = PsCreateSystemThread(&ThreadHandle,
    //                              THREAD_ALL_ACCESS,
    //                              NULL,
    //                              0,
    //                              NULL,
    //                              EventHandlerThread,
    //                              NULL);

    TRACE_EXIT(TraceHandle);

    return STATUS_SUCCESS;
}

// This is called when user-mode disconnects
VOID
FLTAPI
UcaDisconnect(_In_opt_ PVOID ConnectionCookie)
{
    NTSTATUS Status;
    __debugbreak();
    PAGED_CODE();
    TRACE_ENTER(TraceHandle);

    /* Close the handle to the client */
    FltCloseClientPort(DriverData.FilterHandle, &g_ClientConnection.ClientPort);

    TRACE_EXIT(TraceHandle);
}

NTSTATUS
FLTAPI
UcaMessage(_In_opt_ PVOID ConnectionCookie,
           _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
           _In_ ULONG InputBufferLength,
           _Out_writes_bytes_to_opt_(OutputBufferLength,*ReturnOutputBufferLength) PVOID OutputBuffer,
           _In_ ULONG OutputBufferLength,
           _Out_ PULONG ReturnOutputBufferLength)
{
    CUSPIS_ENCRYPT_COMMAND Message;
    NTSTATUS Status;
    __debugbreak();
    TRACE_ENTER(TraceHandle);

    PAGED_CODE();

    /* Sanity check */
    if (!InputBuffer || (InputBufferLength < sizeof(CUSPIS_ENCRYPT_HEADER)))
        return STATUS_INVALID_PARAMETER;

    __try
    {
        /* Store the message in a safe buffer */
        Message = ((PCUSPIS_ENCRYPT_HEADER)InputBuffer)->Message;
    }
    __except (ExceptionFilter(GetExceptionInformation(), TRUE))
    {
        return GetExceptionCode();
    }

    FLT_ASSERT(&g_ClientConnection == ConnectionCookie);

    switch (Message)
    {
        case SetNotifyState:
            Status = HandleSetNotifyState(&g_ClientConnection,
                                          InputBuffer,
                                          InputBufferLength);
            break;

        case GetNotification:
            Status = HandleGetNotificationBuffer(&g_ClientConnection,
                                                 OutputBuffer,
                                                 OutputBufferLength,
                                                 ReturnOutputBufferLength);
            break;

        default:
            Status = STATUS_INVALID_PARAMETER;
            break;
    }
    

    TRACE_EXIT(TraceHandle);

    return Status;
}

NTSTATUS
FLTAPI
UcaInstanceSetup(_In_ PCFLT_RELATED_OBJECTS FltObjects,
                 _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
                 _In_ DEVICE_TYPE VolumeDeviceType,
                 _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
    BOOLEAN IsWritable = FALSE;
    NTSTATUS Status;
    __debugbreak();
    TRACE_ENTER(TraceHandle);

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    PAGED_CODE();
    
    TRACE_INFO(TraceHandle, "%lu, %lu", VolumeDeviceType, VolumeFilesystemType);

    /* Ignore read only volumes */
    Status = FltIsVolumeWritable(FltObjects->Volume, & IsWritable);
    if (!NT_SUCCESS(Status) || IsWritable == FALSE)
        return STATUS_FLT_DO_NOT_ATTACH;

    /* Only attach to local disks and network volumes */
    if (VolumeDeviceType == FILE_DEVICE_DISK_FILE_SYSTEM ||
        VolumeDeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM)
    {
        TRACE_INFO(TraceHandle, "Attaching volume");
        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_FLT_DO_NOT_ATTACH;
    }

    TRACE_EXIT(TraceHandle);

    return Status;
}

NTSTATUS
FLTAPI
UcaFilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags)
{
    TRACE_ENTER(TraceHandle);
    __debugbreak();
    UNREFERENCED_PARAMETER(Flags);
    PAGED_CODE();

    /* Close the port and unregister the filter */
    if (DriverData.ServerPort) FltCloseCommunicationPort(DriverData.ServerPort);
    if (DriverData.FilterHandle) FltUnregisterFilter(DriverData.FilterHandle);

    /* Delete the context data lookaside list */
    ExDeleteNPagedLookasideList(&DriverData.ClientConnectionsLookasideList);

    /* Unregister the trace logger */
    TRACE_EXIT(TraceHandle);
    TRACE_UNREGISTER(TraceHandle);

    return STATUS_SUCCESS;
}

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject,
            _In_ PUNICODE_STRING RegistryPath)
{
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(ENC_DRIVER_NAME);
    NTSTATUS Status;

    /* Default to NonPagedPoolNx for non paged pool allocations where supported */
    ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

    /* Register the trace logger */
    TRACE_REGISTER(&UcaEventEngineGuid, UCA_ETW_TRACE, &TraceHandle);
    TRACE_ENTER(TraceHandle);

    UNREFERENCED_PARAMETER(RegistryPath);

    /* Initialize the global data */
    RtlZeroMemory(&DriverData, sizeof(DRIVER_DATA));
    DriverData.DriverObject = DriverObject;
    DriverData.FilterHandle = NULL;
    DriverData.ServerPort = NULL;

    /* Set the list size data */
    DriverData.MaxNotificationsToAllocate = 0;//MAX_NOTIFICATION_ENTRIES;
    DriverData.NotificationsAllocated = 0;

    /* Initialize the client connections list */
    InitializeListHead(&DriverData.ClientConnections);
    FltInitializePushLock(&DriverData.ClientConnectionsLock);
    ExInitializeNPagedLookasideList(&DriverData.ClientConnectionsLookasideList,
                                    NULL,
                                    NULL,
                                    0,
                                    sizeof(CLIENT_CONNECTION),
                                    UCA_POOL_TAG,
                                    0);

    /* Register with Filter Manager and give it our callback routines */
    Status = FltRegisterFilter(DriverObject,
                               &FilterRegistration,
                               &DriverData.FilterHandle);
    if (NT_SUCCESS(Status))
    {
        /* Create an all access security descriptor */
        Status = FltBuildDefaultSecurityDescriptor(&SecurityDescriptor,
                                                   FLT_PORT_ALL_ACCESS); //FLT_PORT_CONNECT

        if (NT_SUCCESS(Status))
        {
            /* Initialize the security descriptor object */
            InitializeObjectAttributes(&ObjectAttributes,
                                       &DeviceName,
                                       OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       SecurityDescriptor);

            /* Create the usermode communication port */
            Status = FltCreateCommunicationPort(DriverData.FilterHandle,
                                                &DriverData.ServerPort,
                                                &ObjectAttributes,
                                                NULL,
                                                UcaConnect,
                                                UcaDisconnect,
                                                UcaMessage,
                                                MAX_CLIENT_CONNECTIONS);
            if (NT_SUCCESS(Status))
            {
                /* Start filtering the requests */
                Status = FltStartFiltering(DriverData.FilterHandle); //fixme: don't start filtering until we have a request
            }

            /* Free the security descriptor */
            FltFreeSecurityDescriptor(SecurityDescriptor);
        }
    }

    /* Check if anything failed */
    if (!NT_SUCCESS(Status))
    {
        TRACE_ERROR(TraceHandle, "Error creating filter driver : %X", Status);
        TRACE_UNREGISTER(TraceHandle);

        /* Free the resources */
        if (DriverData.ServerPort) FltCloseCommunicationPort(DriverData.ServerPort);
        if (DriverData.FilterHandle) FltUnregisterFilter(DriverData.FilterHandle);
    }

    TRACE_EXIT(TraceHandle);

    return Status;
}

