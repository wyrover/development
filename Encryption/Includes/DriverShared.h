#pragma once

#define ENC_DRIVER_NAME L"\\EncryptionDriver"

typedef enum _CUSPIS_ENCRYPT_COMMAND {

    SetNotifyState,
    GetNotification,
    ReplyNoficiation

} CUSPIS_ENCRYPT_COMMAND;

typedef struct _CUSPIS_ENCRYPT_HEADER
{
    CUSPIS_ENCRYPT_COMMAND Message;
    PVOID Buffer;
    ULONG BufferSize;

} CUSPIS_ENCRYPT_HEADER, *PCUSPIS_ENCRYPT_HEADER;


typedef PVOID ENCRYPT_FILE_COOKIE;

typedef struct _CUSPIS_ENCRYPT_NOTIFICATION
{
    ENCRYPT_FILE_COOKIE Cookie;
    UCHAR Irp;
    PWCHAR File;

} CUSPIS_ENCRYPT_NOTIFICATION, *PCUSPIS_ENCRYPT_NOTIFICATION;

typedef struct _CUSPIS_ENCRYPT_NOTIFICATION_REPLY
{
    ENCRYPT_FILE_COOKIE Cookie;

} CUSPIS_ENCRYPT_NOTIFICATION_REPLY, *PCUSPIS_ENCRYPT_NOTIFICATION_REPLY;

typedef struct _CUSPIS_ENCRYPT_SET_STATE
{
    CUSPIS_ENCRYPT_HEADER Header;

} CUSPIS_ENCRYPT_SET_STATE, *PCUSPIS_ENCRYPT_SET_STATE;




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

