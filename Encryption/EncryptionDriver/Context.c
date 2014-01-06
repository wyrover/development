/*
 * PROJECT:         Unified Client Architecture, File System Filter Driver
 * COPYRIGHT:       AppSense Ltd, 2013
 * PURPOSE:         Handles the 
 * PROGRAMMERS:     Ged Murphy (gerard.murphy@appsense.com)
 *
 */

/* INCLUDES ******************************************************************/

#include "EncryptionDriver.h"
#include "Context.h"
#include "Interface.h"
//#ifdef UCA_DEBUG
#include "DriverDebug.h"
//#else
//#include "UcaLogging.h"
//#endif


/* DATA **********************************************************************/

static NTSTATUS
UcaAllocateContext(
    _In_ FLT_CONTEXT_TYPE ContextType,
    _Outptr_ PFLT_CONTEXT *Context
);

static NTSTATUS
UcaGetContext(
    _In_ PFLT_INSTANCE Instance,
    _In_ PVOID Target,
    _In_ FLT_CONTEXT_TYPE ContextType,
    _Outptr_ PFLT_CONTEXT *Context
);

static NTSTATUS
UcaSetContext(
    _In_ PFLT_INSTANCE Instance,
    _In_ PVOID Target,
    _In_ FLT_CONTEXT_TYPE ContextType,
    _In_ PFLT_CONTEXT NewContext,
    _Outptr_opt_result_maybenull_ PFLT_CONTEXT *OldContext
);


VOID FLTAPI
UcaStreamContextCleanupCallback(
    _In_ PUCA_STREAM_CONTEXT StreamContext,
    _In_ FLT_CONTEXT_TYPE ContextType
);

VOID FLTAPI
UcaInstanceContextCleanupCallback(
    _In_ PUCA_INSTANCE_CONTEXT InstanceContext,
    _In_ FLT_CONTEXT_TYPE ContextType
);

VOID FLTAPI
UcaTransactionContextCleanupCallback(
    _In_ PUCA_TRANSACTION_CONTEXT TransactionContext,
    _In_ FLT_CONTEXT_TYPE ContextType
);

CONST FLT_CONTEXT_REGISTRATION Contexts[] =
{
    { FLT_INSTANCE_CONTEXT,
      0,
      UcaInstanceContextCleanupCallback,
      sizeof(UCA_INSTANCE_CONTEXT),
      UCA_INSTANCE_CONTEXT_POOL_TAG,
      NULL,
      NULL,
      NULL },

    { FLT_STREAM_CONTEXT,
      0,
      UcaStreamContextCleanupCallback,
      sizeof(UCA_STREAM_CONTEXT),
      UCA_STREAM_CONTEXT_POOL_TAG,
      NULL,
      NULL,
      NULL },

    { FLT_FILE_CONTEXT,
      0,
      UcaStreamContextCleanupCallback,
      sizeof(UCA_STREAM_CONTEXT),
      UCA_STREAM_CONTEXT_POOL_TAG,
      NULL,
      NULL,
      NULL },

    { FLT_TRANSACTION_CONTEXT,
      0,
      UcaTransactionContextCleanupCallback,
      sizeof(UCA_TRANSACTION_CONTEXT),
      UCA_TRANSACTION_CONTEXT_POOL_TAG,
      NULL,
      NULL,
      NULL },

    { FLT_CONTEXT_END }
};



/* PUBLIC FUNCTIONS **********************************************************/

NTSTATUS
UcaGetOrAllocContext(_In_ PFLT_INSTANCE Instance,
                     _In_ PVOID Target,
                     _In_ FLT_CONTEXT_TYPE ContextType,
                     _In_ BOOLEAN CreateIfNotFound,
                     _Outptr_ PFLT_CONTEXT *Context)
{
    PFLT_CONTEXT OldContext;
    NTSTATUS Status;

    *Context = NULL;

    /* Try to get the existing context */
    Status = UcaGetContext(Instance, Target, ContextType, Context);
    if (!NT_SUCCESS(Status) && Status != STATUS_NOT_FOUND)
        return Status;

    /* Check if we need to allocate a new context */
    if (Status == STATUS_NOT_FOUND && CreateIfNotFound)
    {
        /* Allocate a new context */
        Status = UcaAllocateContext(ContextType, Context);
        if (!NT_SUCCESS(Status)) return Status;

        /* Set the new context */
        Status = UcaSetContext(Instance, Target, ContextType, *Context, &OldContext);
        if (!NT_SUCCESS(Status))
        {
            /* Something went wrong, free the context */
            FltReleaseContext(*Context);

            /* Check if a context was already set */
            if (Status == STATUS_FLT_CONTEXT_ALREADY_DEFINED)
            {
                /* We're racing with some other call which managed to set the
                   context before us. We will return that context instead */
                *Context = OldContext;
                Status = STATUS_SUCCESS;
            }
            else
            {
                *Context = NULL;
            }
        }
    }

    return Status;
}


/*  ********************************************************************/

VOID FLTAPI
UcaStreamContextCleanupCallback(_In_ PUCA_STREAM_CONTEXT StreamContext,
                                _In_ FLT_CONTEXT_TYPE ContextType)
{
    PAGED_CODE();

    //__debugbreak();

    FLT_ASSERT(ContextType == FLT_STREAM_CONTEXT || ContextType == FLT_FILE_CONTEXT);

    if (StreamContext->FileNameInfo)
    {
        FltReleaseFileNameInformation(StreamContext->FileNameInfo);
        StreamContext->FileNameInfo = NULL;
    }
}

VOID FLTAPI
UcaInstanceContextCleanupCallback(_In_ PUCA_INSTANCE_CONTEXT InstanceContext,
                                  _In_ FLT_CONTEXT_TYPE ContextType)
{
    UNREFERENCED_PARAMETER(ContextType);

    PAGED_CODE();
    __debugbreak();
    FLT_ASSERT(ContextType == FLT_INSTANCE_CONTEXT);

    ExFreePoolWithTag(&InstanceContext->VolumeGuidName, UCA_POOL_TAG);
}

VOID FLTAPI
UcaTransactionContextCleanupCallback(_In_ PUCA_TRANSACTION_CONTEXT TransactionContext,
                                     _In_ FLT_CONTEXT_TYPE ContextType)
{
    PUCA_DELETE_NOTIFY DeleteNotify = NULL;

    UNREFERENCED_PARAMETER(ContextType);

    PAGED_CODE();
    __debugbreak();
    ASSERT(ContextType == FLT_TRANSACTION_CONTEXT);

    if (TransactionContext->Resource != NULL)
    {
        FltAcquireResourceExclusive( TransactionContext->Resource );

        while (!IsListEmpty( &TransactionContext->DeleteNotifyList))
        {
            //  Remove every UCA_DELETE_NOTIFY, releasing their corresponding
            //  FLT_FILE_NAME_INFORMATION objects and freeing pool used by them
            DeleteNotify = CONTAINING_RECORD(RemoveHeadList(&TransactionContext->DeleteNotifyList),
                                             UCA_DELETE_NOTIFY,
                                             Links);

            FltReleaseContext(DeleteNotify->StreamContext);
            ExFreePoolWithTag(DeleteNotify, UCA_ERESOURCE_POOL_TAG);
        }

        FltReleaseResource(TransactionContext->Resource);

        //  Delete and free the DeleteNotifyList synchronization resource.
        ExDeleteResourceLite(TransactionContext->Resource);
        ExFreePoolWithTag(TransactionContext->Resource, UCA_ERESOURCE_POOL_TAG);
    }
}


/* PRIVATE FUNCTIONS **********************************************************/


static NTSTATUS
UcaAllocateContext(_In_ FLT_CONTEXT_TYPE ContextType,
                   _Outptr_ PFLT_CONTEXT *Context)

{
    PUCA_TRANSACTION_CONTEXT TransactionContext;
    NTSTATUS Status;

    PAGED_CODE();

    switch (ContextType)
    {
        case FLT_STREAM_CONTEXT:
            /* Allocate stream context */
            Status = FltAllocateContext(DriverData.FilterHandle,
                                        FLT_STREAM_CONTEXT,
                                        sizeof(UCA_STREAM_CONTEXT),
                                        UCA_CONTEXT_POOL_TYPE,
                                        Context);

            if (NT_SUCCESS(Status))
            {
                RtlZeroMemory(*Context, sizeof(UCA_STREAM_CONTEXT));
                FltInitializePushLock(&((PUCA_STREAM_CONTEXT)*Context)->Lock);
            }
            break;

        case FLT_FILE_CONTEXT:
            /* Allocate file context */
            Status = FltAllocateContext(DriverData.FilterHandle,
                                        FLT_FILE_CONTEXT,
                                        sizeof(UCA_STREAM_CONTEXT),
                                        UCA_CONTEXT_POOL_TYPE,
                                        Context);
            if (NT_SUCCESS(Status))
            {
                RtlZeroMemory(*Context, sizeof(UCA_STREAM_CONTEXT));
                FltInitializePushLock(&((PUCA_STREAM_CONTEXT)*Context)->Lock);
            }
            break;

        case FLT_TRANSACTION_CONTEXT:
            /* Allocate transaction context */
            Status = FltAllocateContext(DriverData.FilterHandle,
                                        FLT_TRANSACTION_CONTEXT,
                                        sizeof(UCA_TRANSACTION_CONTEXT),
                                        UCA_CONTEXT_POOL_TYPE,
                                        Context);

            if (NT_SUCCESS(Status))
            {
                TransactionContext = (PUCA_TRANSACTION_CONTEXT)*Context;

                /* Zero the memory */
                RtlZeroMemory(TransactionContext, sizeof(UCA_TRANSACTION_CONTEXT));

                /* Initialize the notify list */
                InitializeListHead(&TransactionContext->DeleteNotifyList);

                /* The resource needs to be in NPP */
                TransactionContext->Resource = (PERESOURCE)ExAllocatePoolWithTag(NonPagedPool,
                                                                                 sizeof(ERESOURCE),
                                                                                 UCA_ERESOURCE_POOL_TAG);
                if (TransactionContext->Resource == NULL)
                {
                    FltReleaseContext(TransactionContext);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                /* Initialize the lock */
                ExInitializeResourceLite(TransactionContext->Resource);
            }
            break;

        case FLT_INSTANCE_CONTEXT:
            /* Allocate instance context */
            Status = FltAllocateContext(DriverData.FilterHandle,
                                        FLT_INSTANCE_CONTEXT,
                                        sizeof(UCA_INSTANCE_CONTEXT),
                                        UCA_CONTEXT_POOL_TYPE,
                                        Context);

            if (NT_SUCCESS(Status))
            {
                RtlZeroMemory(*Context, sizeof(UCA_INSTANCE_CONTEXT));
            }
            break;

        default:
            Status = STATUS_INVALID_PARAMETER;
            break;
    }

    return Status;
}

static NTSTATUS
UcaSetContext(_In_ PFLT_INSTANCE Instance,
              _In_ PVOID Target,
              _In_ FLT_CONTEXT_TYPE ContextType,
              _In_ PFLT_CONTEXT NewContext,
              _Outptr_opt_result_maybenull_ PFLT_CONTEXT *OldContext)
{
    NTSTATUS Status;

    PAGED_CODE();

    switch (ContextType)
    {
        case FLT_STREAM_CONTEXT:

            Status = FltSetStreamContext(Instance,
                                         (PFILE_OBJECT)Target,
                                         FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                                         NewContext,
                                         OldContext);
            break;

        case FLT_FILE_CONTEXT:

            Status = FltSetFileContext(Instance,
                                       (PFILE_OBJECT)Target,
                                       FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                                       NewContext,
                                       OldContext);
            break;

        case FLT_TRANSACTION_CONTEXT:

            Status = FltSetTransactionContext(Instance,
                                              (PKTRANSACTION)Target,
                                              FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                                              NewContext,
                                              OldContext);
            break;

        case FLT_INSTANCE_CONTEXT:

            Status = FltSetInstanceContext(Instance,
                                           FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                                           NewContext,
                                           OldContext);
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
    }

    return Status;
}


static NTSTATUS
UcaGetContext(_In_ PFLT_INSTANCE Instance,
              _In_ PVOID Target,
              _In_ FLT_CONTEXT_TYPE ContextType,
              _Outptr_ PFLT_CONTEXT *Context)
{
    NTSTATUS Status;

    PAGED_CODE();

    switch (ContextType)
    {
        case FLT_STREAM_CONTEXT:
            Status = FltGetStreamContext(Instance,
                                         (PFILE_OBJECT)Target,
                                         Context);
            break;

        case FLT_FILE_CONTEXT:
            Status = FltGetFileContext(Instance,
                                       (PFILE_OBJECT)Target,
                                       Context);
            break;

        case FLT_TRANSACTION_CONTEXT:
            Status = FltGetTransactionContext(Instance,
                                              (PKTRANSACTION)Target,
                                              Context);
            break;

        case FLT_INSTANCE_CONTEXT:
            Status = FltGetInstanceContext(Instance,
                                           Context);
            break;

        default:
            Status = STATUS_INVALID_PARAMETER;
            break;
    }

    return Status;
}

