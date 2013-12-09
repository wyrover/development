#include "TdhUtil.h"
#include "AtlTrace.h"


CAtlTrace::CAtlTrace(void)
{
}


CAtlTrace::~CAtlTrace(void)
{
}

DWORD
CAtlTrace::InitializeTrace(__in PDUMP_INFO DumpInfo)
{
    WCHAR szDbPath[MAX_PATH];
    LPWSTR Dot;
    int DbStatus;
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
        wcscat_s(DumpInfo->OutputFileName, MAX_PATH, L"atl");
    }

    DbStatus = m_DataBase.SqlOpenDatabase(szDbPath);
    if (DbStatus != SQLITE_OK)
    {
        m_DataBase.GetErrorString(DbStatus);
        Status = ERROR_DATABASE_FAILURE;
    }

    return Status;
}

BOOL
CAtlTrace::OutputHeader(__in PEVENT_RECORD Event)
{
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}

BOOL
CAtlTrace::OutputEvent(__in PEVENT_RECORD Event,
                       __in PTRACE_EVENT_INFO EventInfo)
{
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}