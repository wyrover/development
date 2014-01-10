/*
 * PROJECT:         Unified Client Architecture, File System Filter Driver
 * COPYRIGHT:       AppSense Ltd, 2013
 * PURPOSE:         Handles the pre and post-op callbacks from the filter manager
 * PROGRAMMERS:     Ged Murphy (gerard.murphy@appsense.com)
 *
 */

/* INCLUDES ******************************************************************/

#include "EncryptionDriver.h"
#include "Interface.h"
#include "Callbacks.h"
#include "Context.h"
#include "Library.h"
//#ifdef UCA_DEBUG
#include "DriverDebug.h"
//#else
//#include "UcaLogging.h"
//#endif


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UcaFilterPreCreate)
#pragma alloc_text(PAGE, UcaFilterPostCreate)
#pragma alloc_text(PAGE, UcaFilterPreSetInfo)
#pragma alloc_text(PAGE, UcaFilterPreCleanup)
#pragma alloc_text(PAGE, UcaTransactionNotificationCallback)
#endif


/* DATA *********************************************************************/

#define FlagOnAll(F, T)     (FlagOn(F, T) == T)

CONST FLT_OPERATION_REGISTRATION Callbacks[] =
{
    { IRP_MJ_CREATE,
      0,
      UcaFltDriverPreOperation,
      UcaFltDriverPostOperation },

    { IRP_MJ_SET_INFORMATION,
      FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO,
      UcaFltDriverPreOperation,
      UcaFltDriverPostOperation },

    { IRP_MJ_CLEANUP,
      0,
      UcaFltDriverPreOperation,
      UcaFltDriverPostOperation },

    { IRP_MJ_OPERATION_END }
};


/* PUBLIC FUNCTIONS *********************************************************/


FLT_PREOP_CALLBACK_STATUS
FLTAPI
UcaFltDriverPreOperation(_Inout_ PFLT_CALLBACK_DATA Data,
                         _In_ PCFLT_RELATED_OBJECTS FltObjects,
                         _Outptr_result_maybenull_ PVOID *CompletionContext)
{
    FLT_PREOP_CALLBACK_STATUS Status;
    PFLT_IO_PARAMETER_BLOCK Iopb = Data->Iopb;

    switch (Iopb->MajorFunction)
    {

    case IRP_MJ_CREATE:
        Status = UcaFilterPreCreate(Data, FltObjects, CompletionContext);
        break;
#if 0
    case IRP_MJ_SET_INFORMATION:
        Status = UcaFilterPreSetInfo(Data, FltObjects, CompletionContext);
        break;

    case IRP_MJ_CLEANUP:
        Status = UcaFilterPreCleanup(Data, FltObjects, CompletionContext);
        break;
#endif
    default:
        Status = FLT_PREOP_SUCCESS_NO_CALLBACK;
        break;
    }

    return Status;
}

FLT_POSTOP_CALLBACK_STATUS
FLTAPI
UcaFltDriverPostOperation(_Inout_ PFLT_CALLBACK_DATA Data,
                          _In_ PCFLT_RELATED_OBJECTS FltObjects,
                          _In_opt_ PVOID CompletionContext,
                          _In_ FLT_POST_OPERATION_FLAGS Flags)
{
    FLT_POSTOP_CALLBACK_STATUS Status;
    PFLT_IO_PARAMETER_BLOCK Iopb = Data->Iopb;

    switch (Iopb->MajorFunction)
    {

    case IRP_MJ_CREATE:
        Status = UcaFilterPostCreate(Data, FltObjects, CompletionContext, Flags);
        break;
#if 0
    case IRP_MJ_SET_INFORMATION:
        Status = UcaFilterPostSetInfo(Data, FltObjects, CompletionContext, Flags);
        break;

    case IRP_MJ_CLEANUP:
        Status = UcaFilterPostCleanup(Data, FltObjects, CompletionContext, Flags);
        break;
#endif
    default:
        Status = FLT_POSTOP_FINISHED_PROCESSING;
        break;
    }

    return Status;
}


    // check if file object is a directory
    //Data->Iopb->Parameters.Create.Options == FILE_DIRECTORY_FILE
    // check if the file was requested to be deleted
    //Data->Iopb->Parameters.Create.Options & FILE_DELETE_ON_CLOSE

FLT_PREOP_CALLBACK_STATUS
UcaFilterPreCreate(_Inout_ PFLT_CALLBACK_DATA Data,
                   _In_ PCFLT_RELATED_OBJECTS FltObjects,
                   _Outptr_result_maybenull_ PVOID *CompletionContext)
{
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    PUCA_STREAM_CONTEXT StreamContext;
    NTSTATUS Status;

    PCUSPIS_ENCRYPT_NOTIFICATION EncryptNotification;
    ULONG BufferSize;

    PAGED_CODE();

    /* Get the full file information. Note: In pre-op the file may not exist but will succeed */
    Status = UcaGetFileNameInformation(Data, &FileNameInfo);
    if (!NT_SUCCESS(Status))
    {
        TRACE_ERROR(TraceHandle, "Failed to get the file information : %X", Status);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    BufferSize = sizeof(CUSPIS_ENCRYPT_NOTIFICATION) + FileNameInfo->Name.Length + sizeof(WCHAR);
    EncryptNotification = (PCUSPIS_ENCRYPT_NOTIFICATION)ExAllocatePoolWithTag(PagedPool,
                                                                              BufferSize,
                                                                              UCA_POOL_TAG);
    if (EncryptNotification == NULL) return FLT_PREOP_SUCCESS_NO_CALLBACK;

    RtlZeroMemory(EncryptNotification, BufferSize);
    EncryptNotification->Cookie = EncryptNotification;
    EncryptNotification->Irp = IRP_MJ_CREATE;
    EncryptNotification->File = (LPWSTR)(EncryptNotification + 1);
    RtlCopyMemory(EncryptNotification->File, FileNameInfo->Name.Buffer, FileNameInfo->Name.Length);
    EncryptNotification->File[FileNameInfo->Name.Length / sizeof(WCHAR)] = UNICODE_NULL;

    Status = FltSendMessage(DriverData.FilterHandle,
                            &g_ClientConnection.ClientPort,
                            EncryptNotification,
                            BufferSize,
                            NULL,
                            0,
                            NULL);

    ExFreePoolWithTag(EncryptNotification, UCA_POOL_TAG);

    FltReleaseFileNameInformation(FileNameInfo);

    return FLT_PREOP_SUCCESS_NO_CALLBACK;

#if 0
    /* Look for a quick exit */
    //if (== FALSE)
    //{
    //    /* Free the file info and exit without a post-op */
    //    FltReleaseFileNameInformation(FileNameInfo);
    //    return FLT_PREOP_SUCCESS_NO_CALLBACK;
    //}

    /* Allocate a new context for this stream */
    Status = UcaGetOrAllocContext(FltObjects->Instance,
                                  Data->Iopb->TargetFileObject,
                                  FLT_STREAM_CONTEXT,
                                  TRUE,
                                  (PFLT_CONTEXT *)&StreamContext);
    if (!NT_SUCCESS(Status))
    {
        TRACE_ERROR(TraceHandle, "Failed to allocate a new stream context : %X", Status);
        FltReleaseFileNameInformation(FileNameInfo);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    /* Store the file name information for this stream.
       This will be cleaned up in the stream context cleanup */
    StreamContext->FileNameInfo = FileNameInfo;

    /* Set the context info so we don't have to get it again */
    *CompletionContext = StreamContext;

    /* Do all the remaining work in the post-op */
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
#endif
}

FLT_POSTOP_CALLBACK_STATUS
UcaFilterPostCreate(_Inout_ PFLT_CALLBACK_DATA Data,
                    _In_ PCFLT_RELATED_OBJECTS FltObjects,
                    _In_opt_ PVOID CompletionContext,
                    _In_ FLT_POST_OPERATION_FLAGS Flags)
{
    PUCA_STREAM_CONTEXT StreamContext;
    NTSTATUS Status;

    /* This should never be called at DPC */
    PAGED_CODE();

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    /* We stored the stream context in the pre-op completion context */
    StreamContext = (PUCA_STREAM_CONTEXT)CompletionContext;

    /* Ignore if the operation failed */
    if (!NT_SUCCESS(Data->IoStatus.Status) || Data->IoStatus.Status == STATUS_REPARSE)
    {
        FltReleaseContext(StreamContext);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }


    ///* Only handle */
    //if (Data->IoStatus.Information != FILE_OPENED &&
    //    Data->IoStatus.Information != FILE_CREATED &&
    //    Data->IoStatus.Information != FILE_OVERWRITTEN)
    //{
    //    /* Ignore the request */
    //    return FLT_POSTOP_FINISHED_PROCESSING;
    //}

    FltReleaseContext(StreamContext);

    return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_PREOP_CALLBACK_STATUS
UcaFilterPreSetInfo(_Inout_ PFLT_CALLBACK_DATA Data,
                    _In_ PCFLT_RELATED_OBJECTS FltObjects,
                    _Outptr_result_maybenull_ PVOID *CompletionContext)
{
    FILE_INFORMATION_CLASS FileInformationClass;
    FLT_PREOP_CALLBACK_STATUS CallbackStatus;
    PUCA_STREAM_CONTEXT StreamContext;
    BOOLEAN Race, PostOp;
    NTSTATUS Status;

    PAGED_CODE();

    /* Store the class */
    FileInformationClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;

    /* We only handle basic and disp info */
    if (FileInformationClass != FileBasicInformation &&
        FileInformationClass != FileDispositionInformation)
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    /* Allocate a new context for this stream */
    Status = UcaGetOrAllocContext(FltObjects->Instance,
                                  Data->Iopb->TargetFileObject,
                                  FLT_STREAM_CONTEXT,
                                  TRUE, //FIXME: true or false?
                                  (PFLT_CONTEXT *)&StreamContext);
    if (!NT_SUCCESS(Status))
    {
        TRACE_ERROR(TraceHandle, "Failed to get the stream context : %X", Status);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }


    /* Assume no callback */
    CallbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

    /* Check if no callback has been requested */
    if (CallbackStatus == FLT_PREOP_SUCCESS_NO_CALLBACK)
    {
        /* Release the context */
        FltReleaseContext(StreamContext);
    }

    return CallbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS
UcaFilterPostSetInfo(_Inout_ PFLT_CALLBACK_DATA Data,
                     _In_ PCFLT_RELATED_OBJECTS FltObjects,
                     _In_opt_ PVOID CompletionContext,
                     _In_ FLT_POST_OPERATION_FLAGS Flags)
{
    FILE_INFORMATION_CLASS FileInformationClass;
    PUCA_STREAM_CONTEXT StreamContext;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    FLT_ASSERT(CompletionContext != NULL);
    StreamContext = (PUCA_STREAM_CONTEXT)CompletionContext;

    /* Ignore if the operation failed */
    if (!NT_SUCCESS(Data->IoStatus.Status))
    {
        FltReleaseContext(StreamContext);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    /* Store the class */
    FileInformationClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;

    if (FileInformationClass == FileBasicInformation)
    {
        PFILE_BASIC_INFORMATION FileBasicInfo;

        /* Set the buffer */
        FileBasicInfo = (PFILE_BASIC_INFORMATION)
                        Data->Iopb->Parameters.SetFileInformation.InfoBuffer;

        //FIXME: what changed??
    }
    else if (FileInformationClass == FileDispositionInformation)
    {
        //  No synchronization is needed to set the SetDisp field,
        //  because in case of races, the NumOps field will be perpetually
        //  positive, and it being positive is already an indication this
        //  file is a delete candidate, so it will be checked at post-
        //  -cleanup regardless of the value of SetDisp.
        StreamContext->Delete.DeleteFileDisp = ((PFILE_DISPOSITION_INFORMATION)
                                                Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->DeleteFile;

        /* Now that the operation is over, decrement NumOps */
        InterlockedDecrement(&StreamContext->Delete.NumOps);
    }

    /* Release the context we acquired in the pre-op */
    FltReleaseContext(StreamContext);

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS
UcaFilterPreCleanup(_Inout_ PFLT_CALLBACK_DATA Data,
                    _In_ PCFLT_RELATED_OBJECTS FltObjects,
                    _Outptr_result_maybenull_ PVOID *CompletionContext)
{
    PUCA_STREAM_CONTEXT StreamContext;
    NTSTATUS Status;

    PAGED_CODE();

    //
    //  Only streams with stream context will be sent for deletion check
    //  in post-cleanup, which makes sense because they would only ever
    //  have one if they were flagged as candidates at some point.
    //
    //  Gather file information here so that we have a name to report.
    //  The name will be accurate most of the times, and in the cases it
    //  won't, it serves as a good clue and the stream context pointer
    //  value should offer a way to disambiguate that in case of renames
    //  etc.
    //

    /* Get an existing context for this stream */
    Status = UcaGetOrAllocContext(FltObjects->Instance,
                                  Data->Iopb->TargetFileObject,
                                  FLT_STREAM_CONTEXT,
                                  FALSE,
                                  (PFLT_CONTEXT *)&StreamContext);
    if (!NT_SUCCESS(Status))
    {
        TRACE_ERROR(TraceHandle, "Failed to get the stream context : %X", Status);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    *CompletionContext = (PVOID)StreamContext;

    return FLT_PREOP_SYNCHRONIZE;
}

FLT_POSTOP_CALLBACK_STATUS
UcaFilterPostCleanup(_Inout_ PFLT_CALLBACK_DATA Data,
                     _In_ PCFLT_RELATED_OBJECTS FltObjects,
                     _In_opt_ PVOID CompletionContext,
                     _In_ FLT_POST_OPERATION_FLAGS Flags)
{
    FILE_STANDARD_INFORMATION FileStdInfo;
    PUCA_STREAM_CONTEXT StreamContext;
    NTSTATUS Status;

    PAGED_CODE();

    // Assert we're not draining.
    FLT_ASSERT(!FlagOn( Flags, FLTFL_POST_OPERATION_DRAINING));

    ASSERT(CompletionContext);
    StreamContext = (PUCA_STREAM_CONTEXT)CompletionContext;

    /* Ignore if the operation failed */
    if (!NT_SUCCESS(Data->IoStatus.Status))
    {
        FltReleaseContext(StreamContext);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    //  Determine whether or not we should check for deletion. What
    //  flags a file as a deletion candidate is one or more of the following:
    //
    //  1. NumOps > 0. This means there are or were racing changes to
    //  the file delete disposition state, and, in that case,
    //  we don't know what that state is. So, let's err to the side of
    //  caution and check if it was deleted.
    //
    //  2. SetDisp. If this is TRUE and we haven't raced in setting delete
    //  disposition, this reflects the true delete disposition state of the
    //  file, meaning we must check for deletes if it is set to TRUE.
    //
    //  3. DeleteOnClose. If the file was ever opened with
    //  FILE_DELETE_ON_CLOSE, we must check to see if it was deleted.
    //
    //  Also, if a deletion of this stream was already notified, there is no
    //  point notifying it again.
    //

    if ((StreamContext->Delete.NumOps > 0 ||
         StreamContext->Delete.DeleteFileDisp ||
         StreamContext->Delete.DeleteOnClose) &&
        StreamContext->Delete.IsNotified == 0)
    {
        //
        //  The check for deletion is done via a query to
        //  FileStandardInformation. If that returns STATUS_FILE_DELETED
        //  it means the stream was deleted.
        //
        Status = FltQueryInformationFile(Data->Iopb->TargetInstance,
                                         Data->Iopb->TargetFileObject,
                                         &FileStdInfo,
                                         sizeof(FILE_STANDARD_INFORMATION),
                                         FileStandardInformation,
                                         NULL);

        /* Check that the file is actually being deleted */
        if (Status == STATUS_FILE_DELETED)
        {

        }
    }

    FltReleaseContext(StreamContext);

    return FLT_POSTOP_FINISHED_PROCESSING;
}


NTSTATUS FLTAPI
UcaTransactionNotificationCallback(_In_ PCFLT_RELATED_OBJECTS FltObjects,
                                   _In_ PUCA_TRANSACTION_CONTEXT TransactionContext,
                                   _In_ ULONG NotificationMask)
/*++

Routine Description:

    This routine is the transaction notification callback for this minifilter.
    It is called when a transaction we're enlisted in is committed or rolled
    back so that it's possible to emit notifications about files that were
    deleted in that transaction.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    TransactionContext - The transaction context, set/modified when a delete
        is detected.

    NotificationMask - A mask of flags indicating the notifications received
        from FltMgr. Should be either TRANSACTION_NOTIFY_COMMIT or
        TRANSACTION_NOTIFY_ROLLBACK.

Return Value:

    STATUS_SUCCESS - This operation is never pended.

--*/
{
    PUCA_DELETE_NOTIFY DeleteNotify = NULL;
    BOOLEAN Commit;

    UNREFERENCED_PARAMETER( FltObjects );

    PAGED_CODE();

    Commit = BooleanFlagOn(NotificationMask, TRANSACTION_NOTIFY_COMMIT_FINALIZE);

    //  There is no such thing as a simultaneous commit and rollback, nor
    //  should we get notifications for events other than a commit or a
    //  rollback.

    ASSERT((!FlagOnAll( NotificationMask, (UCA_NOTIFICATION_MASK))) &&
           FlagOn( NotificationMask, (UCA_NOTIFICATION_MASK)));

    if (Commit)
    {
        TRACE_INFO(TraceHandle, "UcaTransactionNotificationCallback: COMMIT!\n" );
    }
    else
    {
        TRACE_INFO(TraceHandle, "UcaTransactionNotificationCallback: ROLLBACK!\n" );
    }

    ASSERT(TransactionContext->Resource);

    FltAcquireResourceExclusive(TransactionContext->Resource);

    while (!IsListEmpty(&TransactionContext->DeleteNotifyList))
    {
        DeleteNotify = CONTAINING_RECORD( RemoveHeadList( &TransactionContext->DeleteNotifyList ),
                                         UCA_DELETE_NOTIFY,
                                          Links );

        ASSERT( NULL != DeleteNotify->StreamContext );

        if (!Commit) InterlockedDecrement(&DeleteNotify->StreamContext->Delete.IsNotified);

        if (DeleteNotify->FileDelete)
        {
            TRACE_INFO(TraceHandle, "A file \"%wZ\" (%p) has been",
                       &DeleteNotify->StreamContext->FileNameInfo->Name,
                       DeleteNotify->StreamContext);
        }
        else
        {
            TRACE_INFO(TraceHandle, "An alternate data stream \"%wZ\" (%p) has been",
                       &DeleteNotify->StreamContext->FileNameInfo->Name,
                       DeleteNotify->StreamContext);
        }

        if (Commit)
        {
            TRACE_INFO(TraceHandle, " deleted due to a transaction commit");
        }
        else
        {
            TRACE_INFO(TraceHandle, " saved due to a transaction rollback");
        }

        // release stream context
        FltReleaseContext(DeleteNotify->StreamContext);
        ExFreePool(DeleteNotify);
    }

    FltReleaseResource(TransactionContext->Resource);

    return STATUS_SUCCESS;
}

/* PRIVATE FUNCTIONS ********************************************************/



