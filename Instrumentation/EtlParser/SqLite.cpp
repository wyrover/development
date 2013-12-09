#include "TdhUtil.h"
#include "SqLite.h"

//#include "IntEtlParser.h"

static LPWSTR retCode[] =
{
    L"Successful result",                           // SQLITE_OK          0
    L"SQL error or missing database",               // SQLITE_ERROR       1
    L"Internal logic error in SQLite",              // SQLITE_INTERNAL    2
    L"Access permission denied",                    // SQLITE_PERM        3
    L"Callback routine requested an abort",         // SQLITE_ABORT       4
    L"The database file is locked",                 // SQLITE_BUSY        5
    L"A table in the database is locked",           // SQLITE_LOCKED      6
    L"A malloc() failed",                           // SQLITE_NOMEM       7
    L"Attempt to write a readonly database",        // SQLITE_READONLY    8
    L"Operation terminated by sqlite3_interrupt()", // SQLITE_INTERRUPT   9
    L"Some kind of disk I/O error occurred",        // SQLITE_IOERR      10
    L"The database disk image is malformed",        // SQLITE_CORRUPT    11
    L"NOT USED. Table or record not found",         // SQLITE_NOTFOUND   12
    L"Insertion failed because database is full",   // SQLITE_FULL       13
    L"Unable to open the database file",            // SQLITE_CANTOPEN   14
    L"NOT USED. Database lock protocol error",      // SQLITE_PROTOCOL   15
    L"Database is empty",                           // SQLITE_EMPTY      16
    L"The database schema changed",                 // SQLITE_SCHEMA     17
    L"String or BLOB exceeds size limit",           // SQLITE_TOOBIG     18
    L"Abort due to constraint violation",           // SQLITE_CONSTRAINT 19
    L"Data type mismatch",                          // SQLITE_MISMATCH   20
    L"Library used incorrectly",                    // SQLITE_MISUSE     21
    L"Uses OS features not supported on host",      // SQLITE_NOLFS      22
    L"Authorization denied",                        // SQLITE_AUTH       23
    L"Auxiliary database format error",             // SQLITE_FORMAT     24
    L"2nd parameter to sqlite3_bind out of range",  // SQLITE_RANGE      25
    L"File opened that is not a database file",     // SQLITE_NOTADB     26
};

CSqLite::CSqLite(void) :
    m_db(NULL),
    m_pEventStmt(NULL)
{
}


CSqLite::~CSqLite(void)
{
    SqlCloseDatabase();
}

LPWSTR
CSqLite::GetErrorString(int error)
{
    int arraySize = sizeof(retCode) / sizeof(retCode[0]);

    if (error > arraySize)
        return L"invalid error code";

    return retCode[error];
}

// returns SQLITE_OK
int
CSqLite::SqlBindText(LPSTR lpStr,
                     INT index)
{
    if (!lpStr && m_pEventStmt)
        return SQLITE_ERROR;

    return sqlite3_bind_text(m_pEventStmt,
                             index,
                             lpStr,
                             strlen(lpStr),
                             SQLITE_TRANSIENT); // SQLITE_STATIC); //
}

// returns SQLITE_OK
int
CSqLite::SqlBindInt(INT val,
                    INT index)
{
    if (m_pEventStmt)
        return SQLITE_ERROR;

    return sqlite3_bind_int(m_pEventStmt,
                            index,
                            val);
}

// returns SQLITE_OK
int
CSqLite::SqlBindInt64(LONGLONG val,
                      INT index)
{
    if (m_pEventStmt)
        return SQLITE_ERROR;

    return sqlite3_bind_int64(m_pEventStmt,
                              index,
                              val);
}

// returns SQLITE_OK
int
CSqLite::SqlFinalize()
{
    sqlite3_stmt *pStmt;
    int Status = SQLITE_OK;

    do
    {
        pStmt = sqlite3_next_stmt(m_db, 0);
        if (pStmt != NULL)
        {
            Status = sqlite3_finalize(pStmt);
        }
    } while (Status != SQLITE_OK);

    return Status;
}

int
CSqLite::SqlReset()
{
    if (m_pEventStmt)
        return SQLITE_ERROR;

    return sqlite3_reset(m_pEventStmt);
}

int
CSqLite::SqlStep()
{
    if (m_pEventStmt)
        return SQLITE_ERROR;

    return sqlite3_step(m_pEventStmt);
}

// returns SQLITE_OK
int
CSqLite::SqlExecuteStatement(LPCSTR lpStmt)
{
    LPSTR lpErrMsg;
    BOOL bRet = FALSE;

    if (!m_db || !lpStmt)
        return SQLITE_ERROR;

    return sqlite3_exec(m_db,
                        lpStmt,
                        NULL,
                        NULL,
                        &lpErrMsg);
}

// returns SQLITE_OK
int
CSqLite::SqlBeginTransaction()
{
    return SqlExecuteStatement("BEGIN;");
}

// returns SQLITE_OK
int
CSqLite::SqlEndTransaction()
{
    return SqlExecuteStatement("END;");
}

// returns SQLITE_OK
int
CSqLite::SqlPrepareStatement(__in LPCSTR lpStmt,
                    __inout sqlite3_stmt **ppStmt)
{
    if (!m_db || !lpStmt || !ppStmt)
        return SQLITE_ERROR;

    return sqlite3_prepare_v2(m_db,
                              lpStmt,
                              -1,
                              ppStmt,
                              NULL);
}

// returns SQLITE_OK
// call sqlite3_errmsg16(m_db) on error
int
CSqLite::SqlOpenDatabase(LPWSTR lpPath)
{
    if (m_db || !lpPath)
        return SQLITE_ERROR;

    return sqlite3_open16(lpPath,
                          &m_db);
}

int
CSqLite::SqlCloseDatabase()
{
    int Status = SQLITE_OK;

    if (m_db != NULL)
    {
        Status = sqlite3_close(m_db);
        m_db = NULL;
    }

    return Status;
}
