#pragma once

#define UCA_VOLUME_GUID_NAME_SIZE        48

#define UCA_INSTANCE_CONTEXT_POOL_TAG    'nIfD'
#define UCA_STREAM_CONTEXT_POOL_TAG      'xSfD'
#define UCA_TRANSACTION_CONTEXT_POOL_TAG 'xTfD'
#define UCA_ERESOURCE_POOL_TAG           'sRfD'
#define UCA_DELETE_NOTIFY_POOL_TAG       'nDfD'
#define UCA_STRING_POOL_TAG              'rSfD'

#define UCA_CONTEXT_POOL_TYPE            NonPagedPool

#define UCA_NOTIFICATION_MASK            (TRANSACTION_NOTIFY_COMMIT_FINALIZE | \
                                          TRANSACTION_NOTIFY_ROLLBACK)


//////////////////////////////////////////////////////////////////////////////
//  Types                                                                   //
//////////////////////////////////////////////////////////////////////////////

//  This helps us deal with ReFS 128-bit file IDs and NTFS 64-bit file IDs.
typedef union _UCA_FILE_REFERENCE
{
    struct {
        ULONGLONG Value;          //  The 64-bit file ID lives here.
        ULONGLONG UpperZeroes;    //  In a 64-bit file ID this will be 0.
    } FileId64;

    UCHAR FileId128[16];  //  The 128-bit file ID lives here.

} UCA_FILE_REFERENCE, *PUCA_FILE_REFERENCE;


//  This is the instance context for this minifilter, it stores the volume's
//  GUID name.
typedef struct _UCA_INSTANCE_CONTEXT
{
    //  Volume GUID name.
    UNICODE_STRING VolumeGuidName;

} UCA_INSTANCE_CONTEXT, *PUCA_INSTANCE_CONTEXT;


typedef struct _UCA_STREAM_CONTEXT
{
    EX_PUSH_LOCK Lock;

    PFLT_FILE_NAME_INFORMATION FileNameInfo; //fixme: what if this name changes after the stream is opened??

    //  File ID, obtained from querying the file system for FileInternalInformation.
    //  If the File ID is 128 bits (as in ReFS) we get it via FileIdInformation.
    UCA_FILE_REFERENCE FileId;

    struct {
        // create or open
        ULONG Foo;
    } Open;

    struct {
        // rename, attr, size, last write, last access, 
        ULONG Foo;
    } Modify;

    struct {
        // write to file
        ULONG Foo;
    } Write;

    struct {
        // file or stream deletes
        volatile LONG NumOps;       //  Number of SetDisp operations in flight.
        volatile LONG IsNotified;   //  IsNotified == 1 means a file/stream deletion was already notified.
        BOOLEAN FileIdSet;          //  Whether or not we've already queried the file ID.
        BOOLEAN DeleteFileDisp;     //SetDisp;
        BOOLEAN DeleteOnClose;
    } Delete;

} UCA_STREAM_CONTEXT, *PUCA_STREAM_CONTEXT;


//  This is the transaction context for this minifilter, attached at post-
//  -cleanup when notifying a delete within a transaction.
typedef struct _UCA_TRANSACTION_CONTEXT
{
    //  List of DF_DELETE_NOTIFY structures representing pending delete
    //  notifications.
    LIST_ENTRY DeleteNotifyList;                

    //  ERESOURCE for synchronized access to the DeleteNotifyList.
    //
    //  ERESOURCEs must be allocated from NonPagedPool. If an ERESOURCE was
    //  declared here as a direct member of a structure, instead of just a
    //  pointer, then the whole transaction context would need to be allocated
    //  out of NonPagedPool.
    //
    //  Therefore, declaring it as a pointer and only allocating at context
    //  initialization time helps us save some NonPagedPool. This is
    //  particularly important in larger context structures.
    PERESOURCE Resource;

} UCA_TRANSACTION_CONTEXT, *PUCA_TRANSACTION_CONTEXT;


//  This structure represents pending delete notifications for files that have
//  been deleted in an open transaction.
typedef struct _UCA_DELETE_NOTIFY
{
    //  Links to other DF_DELETE_NOTIFY structures in the list.
    LIST_ENTRY Links;

    //  Pointer to the stream context for the deleted stream/file.
    PUCA_STREAM_CONTEXT StreamContext;

    //  TRUE for a deleted file, FALSE for a stream.
    BOOLEAN FileDelete;

} UCA_DELETE_NOTIFY, *PUCA_DELETE_NOTIFY;




NTSTATUS
UcaGetOrAllocContext(
    _In_ PFLT_INSTANCE Instance,
    _In_ PVOID Target,
    _In_ FLT_CONTEXT_TYPE ContextType,
    _In_ BOOLEAN CreateIfNotFound,
    _Outptr_ PFLT_CONTEXT *Context
);