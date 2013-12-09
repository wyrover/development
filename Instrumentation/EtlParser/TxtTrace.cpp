#include "TdhUtil.h"
#include "TxtTrace.h"

#define LOG_STRING_SIZE 1024

#define WINDOWS_ANCIENT "*Pre Windows XP*"
#define WINDOWS_XP      "Windows XP"
#define WINDOWS_2003    "Windows 2003"
#define WINDOWS_VISTA   "Windows Vista"
#define WINDOWS_2K8     "Windows 2008"
#define WINDOWS_7       "Windows 7"
#define WINDOWS_2K8R2   "Windows 2008 R2"
#define WINDOWS_NEW     "*Post Windows 2008 R2*"

CTxtTrace::CTxtTrace(void) :
    m_hFile(NULL)
{
}

CTxtTrace::~CTxtTrace(void)
{
    /* Check if the file handle is valid and close it */
    if (m_hFile && m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFile);
}

LPCSTR
CTxtTrace::GetOSString(__in USHORT MajorVersion,
                       __in USHORT MinorVersion,
                       __in USHORT ProductType)
{
    /* Windows 2000 and previous */
    if ((MajorVersion == 5 && MinorVersion < 1) || MajorVersion < 5)
    {
        return WINDOWS_ANCIENT;
    }
    /* Windows XP */
    else if (MajorVersion == 5 && MinorVersion == 1)
    {
        return WINDOWS_XP;
    }
    /* Windows Server 2003 */
    else if (MajorVersion == 5 && MinorVersion == 2)
    {
        return WINDOWS_2003;
    }
    /* Windows Vista, Windows Server 2008 */
    else if (MajorVersion == 6 && MinorVersion == 0)
    {
        if (ProductType == VER_NT_WORKSTATION)
            return WINDOWS_VISTA;
        else
            return WINDOWS_2K8;
    }
    /* Windows 7, Windows Server 2008 R2 */
    else if (MajorVersion == 6 && MinorVersion == 1)
    {
        if (ProductType == VER_NT_WORKSTATION)
            return WINDOWS_7;
        else
            return WINDOWS_2K8R2;
    }
    else if (MajorVersion == 6 && MinorVersion > 1 || MajorVersion > 6)
    {
        return WINDOWS_NEW;
    }

    return "ERROR";
}

DWORD
CTxtTrace::InitializeTrace(__in PDUMP_INFO DumpInfo)
{
    LPWSTR Dot;
    DWORD Status = ERROR_SUCCESS;

    /* Call the base class */
    __super::InitializeTrace(DumpInfo);

    /* Check if we need to build an output file name */
    if (DumpInfo->OutputFileName[0] == UNICODE_NULL)
    {
        /* Take a copy of the path and find the extension */
        wcscpy_s(DumpInfo->OutputFileName, MAX_PATH, DumpInfo->EtlFileName);
        Dot = wcsrchr(DumpInfo->OutputFileName, L'.');
        if (!Dot) return ERROR_BAD_PATHNAME;

        /* Terminate after the dot and concat the new extension */
        *(++Dot) = UNICODE_NULL;
        wcscat_s(DumpInfo->OutputFileName, MAX_PATH, L"log");
    }

    /* Create the log file */
    m_hFile = CreateFileW(DumpInfo->OutputFileName,
                          GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        Status = GetLastError();
        if (Status == ERROR_ACCESS_DENIED)
        {
            wprintf(L"Access denied creating log file %s\n", DumpInfo->OutputFileName);
        }
    }

    return Status;
}

CHAR szLevels[][3] =
{
    "",
    "L1", // LEVEL_INFO
    "L2", // LEVEL_WARNING
    "L3", // LEVEL_ERROR
    "L4"  // LEVEL_CRITICAL
};

BOOL
CTxtTrace::OutputHeader(__in PEVENT_RECORD Event)
{
    PTRACE_LOGFILE_HEADER LogHeader = (PTRACE_LOGFILE_HEADER)Event->UserData;
    WCHAR TimeString[STRLEN_UTC_DATETIME];
    CHAR szDetail[LOG_STRING_SIZE];
    DWORD dwBytes, dwBytesWritten, DateTimeStringSize = STRLEN_UTC_DATETIME * sizeof(WCHAR);;
    USHORT Consumed;
    LPCSTR OsString;
    ULONG Status;
    BOOL bSuccess = FALSE;

    SetLastError(ERROR_SUCCESS);

    OsString = GetOSString(LogHeader->VersionDetail.MajorVersion,
                           LogHeader->VersionDetail.MinorVersion,
                           LogHeader->VersionDetail.SubVersion);

    Status = FileTimeToBuffer((PBYTE)&LogHeader->BootTime,
                              sizeof(FILETIME),
                              (PBYTE)&TimeString[0],
                              DateTimeStringSize,
                              &Consumed);
    dwBytes = _snprintf_s(szDetail,
                          LOG_STRING_SIZE,
                          _TRUNCATE,
                          "Operating system\t%s\r\nBoot time\t\t%S\r\n",
                          OsString,
                          TimeString);

    Status = FileTimeToBuffer((PBYTE)&LogHeader->StartTime,
                              sizeof(FILETIME),
                              (PBYTE)&TimeString[0],
                              DateTimeStringSize,
                              &Consumed);
    dwBytes += _snprintf_s(szDetail + dwBytes,
                           LOG_STRING_SIZE - dwBytes,
                           _TRUNCATE,
                           "Trace Start\t\t%S\r\n",
                           TimeString);

    Status = FileTimeToBuffer((PBYTE)&LogHeader->EndTime,
                              sizeof(FILETIME),
                              (PBYTE)&TimeString[0],
                              DateTimeStringSize,
                              &Consumed);
    dwBytes += _snprintf_s(szDetail + dwBytes,
                           LOG_STRING_SIZE - dwBytes,
                           _TRUNCATE,
                           "Trace End\t\t%S\r\n",
                           TimeString);


    /* Terminate the string the windows way */
    dwBytes += _snprintf_s(szDetail + dwBytes,
                           LOG_STRING_SIZE - dwBytes,
                           _TRUNCATE,
                           "\r\n");

    /* Write it to file */
    bSuccess = WriteFile(m_hFile,
                         szDetail,
                         dwBytes,
                         &dwBytesWritten,
                         NULL);

    return bSuccess;
}

VOID
ConvertTimeStamp(__in LPSTR Buffer,
                 __in DWORD BufferSize,
                 __in PLARGE_INTEGER Time)
{
    SYSTEMTIME SystemTime;
    ULONGLONG NanoSeconds;
    INT StrLen;

    FileTimeToSystemTime((LPFILETIME)Time, &SystemTime);

    StrLen = GetTimeFormatA(LOCALE_USER_DEFAULT,
                            0,
                            &SystemTime,
                            "HH':'mm':'ss",
                            Buffer,
                            BufferSize);
    if (StrLen == 0) return;

    NanoSeconds = (Time->QuadPart % ONE_HUNDRED_NANOSECONDS_PER_SECOND) / 1000;
    sprintf_s(Buffer + (StrLen-1),
              BufferSize - StrLen,
              ".%.4I64u",
              NanoSeconds);
}

BOOL
CTxtTrace::OutputEvent(__in PEVENT_RECORD Event,
                       __in PTRACE_EVENT_INFO EventInfo)
{
    CHAR szLogString[LOG_STRING_SIZE];
    CHAR szTimeStamp[STRLEN_UTC_DATETIME];
    DWORD dwBytes, dwBytesWritten;
    INT iStart;
    DWORD SessionId;
    LPWSTR ProcessName, ModuleName, FunctionName, Detail;
    WCHAR EmptyString[] = L"";
    BOOL bSuccess = FALSE;

    SetLastError(ERROR_SUCCESS);

    /* Check if this is a UCA log */
    if (m_pDumpInfo->Flags & DUMP_UCA)
    {
        /* Set the UCA field data */
        FunctionName = m_LogContext.DataContext.RenderItems[2];
        SessionId = (ULONG)_wtoi(m_LogContext.DataContext.RenderItems[3]);
        ProcessName = EmptyString;
        ModuleName = EmptyString;
        Detail = m_LogContext.DataContext.RenderItems[6];

        /* Check if the process name should be dumped */
        if (m_pDumpInfo->Flags & DUMP_PROCNAME)
        {
            ProcessName = m_LogContext.DataContext.RenderItems[4];
        }

        /* Check if the module name should be dumped */
        if (m_pDumpInfo->Flags & DUMP_MODULENAME)
        {
            ModuleName = m_LogContext.DataContext.RenderItems[5];
        }

        /* Check if only one particular session should be dumped */
        if (m_pDumpInfo->Flags & DUMP_SESSION)
        {
            /* Exit early if we aren't interested in this session */
            if (SessionId != m_pDumpInfo->SessionId)
                return bSuccess;
        }

        /* There should be 6 items available to log */
        if (m_LogContext.DataContext.RenderItemsCount == 7)
        {
            /* Convert the timestamp to a human readable form */
            ConvertTimeStamp(szTimeStamp, STRLEN_UTC_DATETIME, &Event->EventHeader.TimeStamp);

            /* Create the log string */
            iStart = _snprintf_s(szLogString,
                                 LOG_STRING_SIZE,
                                 _TRUNCATE,
                                 "%s (%s) S%lu P%-5lu T%-5lu %S %S [%S] %S\r\n",
                                 szLevels[Event->EventHeader.EventDescriptor.Level],
                                 szTimeStamp,
                                 SessionId,
                                 Event->EventHeader.ProcessId,
                                 Event->EventHeader.ThreadId,
                                 ProcessName,
                                 ModuleName,
                                 FunctionName,
                                 Detail);
        }
        else
        {
            iStart = _snprintf_s(szLogString,
                                 LOG_STRING_SIZE,
                                 _TRUNCATE,
                                 "Size error : %lu\r\n",
                                 m_LogContext.DataContext.RenderItemsCount);
        }
    }
    else
    {
        /* Add the standard header information */
        iStart = _snprintf_s(szLogString,
                             LOG_STRING_SIZE,
                             _TRUNCATE,
                             "%s %I64d P%-5lu T%-5lu",
                             szLevels[Event->EventHeader.EventDescriptor.Level],
                             Event->EventHeader.TimeStamp,
                             Event->EventHeader.ProcessId,
                             Event->EventHeader.ThreadId);

        /* Add all the manifest data */
        for (ULONG i = 0; i < m_LogContext.DataContext.RenderItemsCount; i++)
        {
            iStart += _snprintf_s(szLogString + iStart,
                                  LOG_STRING_SIZE - iStart,
                                  _TRUNCATE,
                                  "\t%S",
                                  m_LogContext.DataContext.RenderItems[i]);
        }

        /* Finally terminate the string the windows way */
        _snprintf_s(szLogString + iStart,
                    LOG_STRING_SIZE - iStart,
                    _TRUNCATE,
                    "\r\n");
    }

    /* Write it to file */
    dwBytes = (DWORD)strlen(szLogString) * sizeof(CHAR);
    bSuccess = WriteFile(m_hFile,
                         szLogString,
                         dwBytes,
                         &dwBytesWritten,
                         NULL);

    return bSuccess;
}
