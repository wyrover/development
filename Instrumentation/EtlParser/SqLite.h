#pragma once
#include "sqlite3.h"

class CSqLite
{
    sqlite3 *m_db;
    sqlite3_stmt *m_pEventStmt;

public:
    CSqLite(void);
    ~CSqLite(void);

    LPWSTR GetErrorString(int error);

    int SqlBindText(
        __in LPSTR lpStr,
        __in INT index);

    int SqlBindInt(
        __in INT val,
        __in INT index);

    int SqlBindInt64(
        __in LONGLONG val,
        __in INT index);

    int SqlFinalize();

    int SqlReset();
    int SqlStep();
    int SqlExecuteStatement(__in LPCSTR lpStmt);
    int SqlBeginTransaction();
    int SqlEndTransaction();
    int SqlPrepareStatement(
        __in LPCSTR lpStmt,
        __inout sqlite3_stmt **ppStmt);
    int SqlOpenDatabase(LPWSTR lpPath);
    int SqlCloseDatabase();
};

