/*
 * PROJECT:         Unified Client Architecture, File System Filter Driver
 * COPYRIGHT:       AppSense Ltd, 2013
 * PURPOSE:         Miscalanous functions
 * PROGRAMMERS:     Ged Murphy (gerard.murphy@appsense.com)
 *
 */

/* INCLUDES ******************************************************************/

#include "EncryptionDriver.h"
//#include "UcaFltDriver.h"
#include "Library.h"
//#ifdef UCA_DEBUG
#include "DriverDebug.h"
//#else
//#include "UcaLogging.h"
//#endif


/* DATA *********************************************************************/

#define UcaSizeofFileId(FID) (               \
    ((FID).FileId64.UpperZeroes == 0ll) ?   \
        sizeof((FID).FileId64.Value)    :   \
        sizeof((FID).FileId128)             \
    )


/* PUBLIC FUNCTIONS *********************************************************/

LONG
ExceptionFilter(_In_ PEXCEPTION_POINTERS ExceptionPointer,
                _In_ BOOLEAN AccessingUserBuffer)
{
    NTSTATUS Status;

    /* Handle if the exception is valid or this is a usermode buffer */
    Status = ExceptionPointer->ExceptionRecord->ExceptionCode;
    if (FsRtlIsNtstatusExpected(Status) || AccessingUserBuffer)
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

NTSTATUS
UcaGetFileNameInformation(_In_  PFLT_CALLBACK_DATA Data,
                          _Out_ PFLT_FILE_NAME_INFORMATION *FileNameInformation)
{
    NTSTATUS Status;

    /* Get the file name info */
    Status = FltGetFileNameInformation(Data,
                                       FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
                                       FileNameInformation);
    if (!NT_SUCCESS(Status))
    {
        /* If we failed to get the normalized, the opened should succeed */
        Status = FltGetFileNameInformation(Data,
                                           FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_DEFAULT,
                                           FileNameInformation);
    }

    /* Exit if we failed to get a filename */
    if (!NT_SUCCESS(Status)) return Status;

    /* Fill in the rest of the structure */
    Status = FltParseFileNameInformation(*FileNameInformation);
    if (!NT_SUCCESS(Status))
    {
        FltReleaseFileNameInformation(*FileNameInformation);
    }

    return Status;
}

#if 0
NTSTATUS
UcaGetVolumeGuidName(_In_ PCFLT_RELATED_OBJECTS FltObjects,
                     _Inout_ PUNICODE_STRING VolumeGuidName)
{
    PUCA_INSTANCE_CONTEXT InstanceContext = NULL;
    NTSTATUS Status;

    //PAGED_CODE();

    /* Get the instance context, or create one if none exists */
    Status = UcaGetOrAllocContext(FltObjects->Instance,
                                  NULL,
                                  FLT_INSTANCE_CONTEXT,
                                  TRUE,
                                  (PFLT_CONTEXT *)&InstanceContext);
    if (!NT_SUCCESS(Status)) return Status;

    /* Check if we've already cached the volume guid in a previous call */
    if (InstanceContext->VolumeGuidName.Buffer == NULL)
    {
        UNICODE_STRING TempString;
        ULONG RequiredBytes;

        /* Get the required buffer for the string, plug an extra one for a trailing backslash */
        RequiredBytes = UCA_VOLUME_GUID_NAME_SIZE * sizeof(WCHAR) +  sizeof(WCHAR);

        /* Set the string data */
        TempString.Length = 0;
        TempString.MaximumLength = (USHORT)RequiredBytes;

        /* Allocate some memory to hold the target */
        TempString.Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool,
                                                        RequiredBytes,
                                                        UCA_POOL_TAG);
        if (TempString.Buffer == NULL)
        {
            FltReleaseContext(InstanceContext);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        /* Now get the volume guid name */
        Status = FltGetVolumeGuidName(FltObjects->Volume,
                                      &TempString,
                                      NULL);
        if (!NT_SUCCESS(Status))
        {
            TRACE_ERROR(TraceHandle, "FltGetVolumeGuidName returned %X", Status);
            ExFreePoolWithTag(TempString.Buffer, UCA_POOL_TAG);
            FltReleaseContext(InstanceContext);
            return Status;
        }

        /* Add the trailing backslash */
        RtlAppendUnicodeToString(&TempString, L"\\");

        /* Set the SourceGuidName to the TempString.
         * It's okay to set Length and MaximumLength with no synchronization
         * because those will always be the same value */
        InstanceContext->VolumeGuidName.Length = TempString.Length;
        InstanceContext->VolumeGuidName.MaximumLength = TempString.MaximumLength;

        /*  Setting the buffer, however, requires some synchronization,
         *  because another thread might be attempting to do the same,
         *  and even though they're exactly the same string, they're
         *  different allocations (buffers) so if the other thread we're
         *  racing with manages to set the buffer before us, we need to
         *  free our temporary string buffer */
        InterlockedCompareExchangePointer(&InstanceContext->VolumeGuidName.Buffer,
                                          TempString.Buffer,
                                          NULL);

        if (InstanceContext->VolumeGuidName.Buffer != TempString.Buffer)
        {
            /* We didn't manage to set the buffer, so free the TempString buffer */
            ExFreePoolWithTag(TempString.Buffer, UCA_POOL_TAG);
        }
    }

    /* Copy the guid name to the caller */
    RtlCopyUnicodeString(VolumeGuidName, &InstanceContext->VolumeGuidName);

    /* We're done with the instance context */
    FltReleaseContext(InstanceContext);

    return Status;
}
#endif