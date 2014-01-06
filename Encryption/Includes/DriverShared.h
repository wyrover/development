#pragma once

#define ENC_DRIVER_NAME L"\\EncryptionDriver"

typedef enum _UCA_FLT_COMMAND {

    SetNotifyState,
    GetNotificationBuffer,
    AddWatchedObject,
    RemoveWatchedObject

} UCA_FLT_COMMAND;

typedef struct _UCA_FLT_HEADER
{
    UCA_FLT_COMMAND Message;
    PVOID Buffer;
    ULONG BufferSize;

} UCA_FLT_HEADER, *PUCA_FLT_HEADER;



typedef ULONG FILE_OPERATION_TYPE;
    #define FILEOP_CREATE               0x0001
    #define FILEOP_OPEN                 0x0002
    #define FILEOP_OVERWRITE            0x0004
    #define FILEOP_CHANGE_FILENAME      0x0010
    #define FILEOP_CHANGE_DIRNAME       0x0020
    #define FILEOP_CHANGE_ATTR          0x0040
    #define FILEOP_CHANGE_SIZE          0x0080
    #define FILEOP_CHANGE_LASTWRITE     0x0100
    #define FILEOP_CHANGE_LASTACCESS    0x0200
    #define FILEOP_CHANGE_CREATION      0x0400
    #define FILEOP_CHANGE_SECURITY      0x0800
    #define FILEOP_WRITE                0x1000
    #define FILEOP_DELETE_FILE          0x2000
    #define FILEOP_DELETE_ADS           0x4000


typedef enum _FILE_OPERATION
{
    FileOpCreate,
    FileOpModify,
    FileOpWrite,
    FileOpDelete

} FILE_OPERATION;




#ifndef POINTER_ALIGNMENT
#if defined(_WIN64)
#define POINTER_ALIGNMENT DECLSPEC_ALIGN(8)
#else
#define POINTER_ALIGNMENT
#endif
#endif

#if _MSC_VER >= 1200
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

typedef struct _UCA_FLT_FILE_NOTIFICATION
{
    LIST_ENTRY ListEntry;
    FILE_OPERATION FileOperation;
    ULONG Size;
    PVOID Cookie;
    
} UCA_FLT_FILE_NOTIFICATION, *PUCA_FLT_FILE_NOTIFICATION;


//typedef struct _UCA_FILE_INFORMATION
//{
//    ULONG ChangedAttributes;
//    LARGE_INTEGER CreationTime;
//    LARGE_INTEGER LastAccessTime;
//    LARGE_INTEGER LastWriteTime;
//    LARGE_INTEGER ChangeTime;
//    ULONG FileAttributes;
//} UCA_FILE_INFORMATION, *PUCA_FILE_INFORMATION;

/*
#define NOTIFY_FILE_CREATION_TIME       0x01
#define NOTIFY_FILE_LAST_ACCESS_TIME    0x02
#define NOTIFY_FILE_LAST_WRITE_TIME     0x04
#define NOTIFY_FILE_CHANGE_TIME         0x08
#define NOTIFY_FILE_ATTRIBUTES          0x10
*/
