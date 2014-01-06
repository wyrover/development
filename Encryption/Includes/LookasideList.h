#pragma once
#include "List.h"

class CLookasideList
{
    LIST_ENTRY m_ListHead;
    CRITICAL_SECTION m_Lock;
    DWORD m_TotalAllocates;
    DWORD m_TotalFrees;
    DWORD m_CurrentAllocates;
    SIZE_T m_Size;
    DWORD m_MaxDepth;

public:
    CLookasideList(SIZE_T Size);
    ~CLookasideList(void);

    LPVOID Allocate();
    BOOL Free(PVOID lpMem);

    VOID GetListMetrics(
        LPDWORD lpTotalAllocates,
        LPDWORD lpTotalFrees,
        LPDWORD lpCurrentAllocates);
};

