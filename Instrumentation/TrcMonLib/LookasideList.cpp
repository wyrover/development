/*
 * PROJECT:         Unified Client Architecture, Run Time Library
 * COPYRIGHT:       AppSense Ltd, 2011
 * PURPOSE:         A lookaside list algorithm for managing fixed memory
 * PROGRAMMERS:     Ged Murphy (gerard.murphy@appsense.com)
 *
 */

#include "StdAfx.h"
#include "LookasideList.h"
//#include "UcaDebug.h"


/* CONSTRUCTORS ***********************************/

CLookasideList::CLookasideList(SIZE_T Size)
{
    /* Initialize the menbers */
    InitializeCriticalSection(&m_Lock);
    InitializeListHead(&m_ListHead);
    m_TotalAllocates = 0;
    m_TotalFrees = 0;
    m_CurrentAllocates = 0;
    m_Size = max(Size, sizeof(LIST_ENTRY));
    m_MaxDepth = 256;
}

CLookasideList::~CLookasideList(void)
{
    LPVOID lpMem;

    /* Assert if anyone is still holding a list entry */
    if (m_CurrentAllocates != 0)
        RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE , 0, NULL);

    /* Loop all the list members */
    while (TRUE)
    {
        /* Quit the loop if the list is empty */
        if (IsListEmpty(&m_ListHead)) break;

        /* Remove and free the memory from the list */
        lpMem = RemoveTailList(&m_ListHead);
        HeapFree(GetProcessHeap(), 0, lpMem);
    }

    /* Delete the list lock */
    DeleteCriticalSection(&m_Lock);
}


/* PUBLIC METHODS ********************************/

LPVOID
CLookasideList::Allocate()
{
    LPVOID lpMem;

    /* Grab the list lock */
    EnterCriticalSection(&m_Lock);

    /* Increment the counters */
    m_TotalAllocates++;
    m_CurrentAllocates++;

    /* Check if the list is empty */
    if (IsListEmpty(&m_ListHead))
    {
        /* Allocate a new block */
        lpMem = HeapAlloc(GetProcessHeap(),
                          0,
                          m_Size);
    }
    else
    {
        /* Remove the free block from the list */
        lpMem = RemoveTailList(&m_ListHead);
    }

    /* Release the list lock */
    LeaveCriticalSection(&m_Lock);

    /* Return the memory */
    return lpMem;
}

BOOL
CLookasideList::Free(LPVOID lpMem)
{
    BOOL bSuccess;

    /* Grab the list lock */
    EnterCriticalSection(&m_Lock);

    /* Update the counters */
    m_TotalFrees++;
    m_CurrentAllocates--;

    /* Check if the list is full */
    if (m_CurrentAllocates >= m_MaxDepth)
    {
        /* Just free the memory */
        bSuccess = HeapFree(GetProcessHeap(), 0, lpMem);
    }
    else
    {
        /* Add the memory to the list */
        InsertTailList(&m_ListHead, (PLIST_ENTRY)lpMem);
        bSuccess = TRUE;
    }

    /* Release the list lock */
    LeaveCriticalSection(&m_Lock);

    return bSuccess;
}

VOID
CLookasideList::GetListMetrics(LPDWORD lpTotalAllocates,
                               LPDWORD lpTotalFrees,
                               LPDWORD lpCurrentAllocates)
{
    /* Set the requested data */
    if (lpTotalAllocates) *lpTotalAllocates = m_TotalAllocates;
    if (lpTotalFrees) *lpTotalFrees = m_TotalFrees;
    if (lpCurrentAllocates) *lpCurrentAllocates = m_CurrentAllocates;
}
