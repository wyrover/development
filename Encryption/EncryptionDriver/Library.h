#pragma once

LONG
ExceptionFilter(
    _In_ PEXCEPTION_POINTERS ExceptionPointer,
    _In_ BOOLEAN AccessingUserBuffer
);

NTSTATUS
UcaGetFileNameInformation(
    _In_  PFLT_CALLBACK_DATA Data,
    _Out_ PFLT_FILE_NAME_INFORMATION *FileNameInformation
);
