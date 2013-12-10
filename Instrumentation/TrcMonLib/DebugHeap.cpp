/*
 * PROJECT:         Unified Client Architecture, Run Time Library
 * COPYRIGHT:       AppSense Ltd, 2012 - 2013
 * PURPOSE:         Debug heap manager, detects overflows, underflows and leaks
 * PROGRAMMERS:     Ged Murphy (ged.murphy@appsense.com)
 *
 */

#include "StdAfx.h"
#include "List.h"
//#include "UcaLogging.h"
//#include "UcaDebug.h"
#define _DEBUG_LOGGING

/* DATA *********************************************/

VOID
UcaMemFree_dbg(HANDLE hUcaMem, UINT Flags, PVOID ptr, const char *file, int line);

typedef struct _DEBUG_MEM_INFO
{
    HANDLE hHeap;
    LIST_ENTRY ListHead;
    SRWLOCK Lock;
    size_t Allocations;
    size_t AllocatedMemory;

} DEBUG_MEM_INFO, *PDEBUG_MEM_INFO;

#define REDZONE_SIZE  32
#define REDZONE_LEFT  0x78
#define REDZONE_RIGHT 0x87

typedef struct _MEM_ALLOC_INFO
{
    size_t size;
    LIST_ENTRY list_entry;
    const char *file;
    int line;
} MEM_ALLOC_INFO, *PMEM_ALLOC_INFO;

#ifdef _DEBUG_LOGGING
extern HANDLE TraceHandle;
#define LOG(Detail, ...) \
{ \
    CHAR str[256]; \
    sprintf_s(str, 256, Detail, __VA_ARGS__); \
    OutputDebugStringA(str); \
}
#else
#define LOG(Detail, ...)
#endif

/* PRIVATE FUNCTIONS ***********************************/


static PVOID
GetBasePtr(PVOID ptr)
{
    return (PVOID)((UINT_PTR)ptr - REDZONE_SIZE - sizeof(MEM_ALLOC_INFO));
}

static PVOID
GetPtrFromBase(PMEM_ALLOC_INFO info)
{
    return (PVOID)((size_t)(info + 1) + REDZONE_SIZE);
}

static PVOID
WriteRedzone(PVOID ptr, size_t size, const char *file, int line)
{
    PVOID ret;
    PMEM_ALLOC_INFO info = (PMEM_ALLOC_INFO)ptr;

    info->size = size;
    info->file = file;
    info->line = line;

    ptr = (PVOID)(info + 1);
    memset(ptr, REDZONE_LEFT, REDZONE_SIZE);
    ret = (PVOID)((size_t)ptr + REDZONE_SIZE);
    ptr = (PVOID)((size_t)ret + size);
    memset(ptr, REDZONE_RIGHT, REDZONE_SIZE);
    return ret;
}

static INT
CheckRedzoneRegion(PVOID ptr, unsigned char sig, PVOID *newptr)
{
    unsigned char *p, *q;
    int ret = 1;

    p = (unsigned char *)ptr;
    q = p + REDZONE_SIZE;
    while (p != q)
    {
        if (*(p++) != sig)
            ret = 0;
    }

    if (newptr != NULL)
        *newptr = p;
    return ret;
}

static VOID
RedzoneError(const char * msg, PMEM_ALLOC_INFO info, PVOID ptr, const char * file, int line)
{
    LOG("DEBUG HEAP: %s\n", msg);
    LOG("     Block: 0x%p Size: %lu\n", ptr, info->size);
    LOG("     Allocated from %s:%d\n", info->file, info->line);
    LOG("     Detected at: %s:%d\n", file, line);
}

static BOOL
CheckRedzone(PVOID ptr, const char *file, int line)
{
    PMEM_ALLOC_INFO info = (PMEM_ALLOC_INFO)ptr;
    ptr = (PVOID)(info + 1);
    if (!CheckRedzoneRegion(ptr, REDZONE_LEFT, &ptr))
    {
        RedzoneError("Detected buffer underflow!\n", info, ptr, file, line);
        return TRUE;
    }
    ptr = (PVOID)((UINT_PTR)ptr + info->size);
    if (!CheckRedzoneRegion(ptr, REDZONE_RIGHT, NULL))
    {
        RedzoneError("Detected buffer overflow!\n", info, ptr, file, line);
        return TRUE;
    }
    return FALSE;
}

static size_t
CalculateSizeWithRedzone(size_t size)
{
    return sizeof(MEM_ALLOC_INFO) + size + (2 * REDZONE_SIZE);
}

static VOID
AddMemToList(HANDLE hUcaMem, PVOID ptr)
{
    PDEBUG_MEM_INFO DbgMem = (PDEBUG_MEM_INFO)hUcaMem;
    PMEM_ALLOC_INFO info = (PMEM_ALLOC_INFO)ptr;
    InsertTailList(&DbgMem->ListHead, &info->list_entry);
}

static VOID
DeleteMemFromList(PVOID ptr)
{
    PMEM_ALLOC_INFO info = (PMEM_ALLOC_INFO)ptr;
    RemoveEntryList(&info->list_entry);
}

static VOID
DumpMemList(HANDLE hUcaMem)
{
    PDEBUG_MEM_INFO DbgMem = (PDEBUG_MEM_INFO)hUcaMem;
    PMEM_ALLOC_INFO info;
    PLIST_ENTRY entry;
    PVOID ptr;

    entry = DbgMem->ListHead.Flink;
    while (entry != &DbgMem->ListHead)
    {
        info = CONTAINING_RECORD(entry, MEM_ALLOC_INFO, list_entry);

        LOG(" * Block: 0x%p Size: %lu allocated from %s:%d\n", GetPtrFromBase(info), info->size, info->file, info->line);

        ptr = (PVOID)(info + 1);
        if (!CheckRedzoneRegion(ptr, REDZONE_LEFT, &ptr))
        {
            LOG("   !!! Detected buffer underflow !!!\n");
        }

        ptr = (PVOID)((UINT_PTR)ptr + info->size);
        if (!CheckRedzoneRegion(ptr, REDZONE_RIGHT, NULL))
        {
            LOG("   !!! Detected buffer overflow !!!\n");
        }

        entry = entry->Flink;
    }
}


/* PUBLIC FUNCTIONS ************************************/

HANDLE
UcaMemCreate_dbg()
{
    PDEBUG_MEM_INFO DbgMem;
    HANDLE hHeap;

    hHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS,
                       0,
                       0);
    if (hHeap == NULL) return NULL;

    LOG("Created debug heap %p\n", hHeap);

    DbgMem = (PDEBUG_MEM_INFO)HeapAlloc(hHeap,
                                        0,
                                        sizeof(DEBUG_MEM_INFO));
    if (DbgMem == NULL)
    {
        HeapDestroy(hHeap);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    DbgMem->hHeap = hHeap;
    InitializeListHead(&DbgMem->ListHead);
    InitializeSRWLock(&DbgMem->Lock);
    DbgMem->Allocations = 0;
    DbgMem->AllocatedMemory = 0;

    return (HANDLE)DbgMem;
}

VOID
UcaMemDestroy_dbg(HANDLE hUcaMem)
{
    PDEBUG_MEM_INFO DbgMem = (PDEBUG_MEM_INFO)hUcaMem;
    HANDLE hHeap;
    BOOL bLeak = FALSE;

    LOG("Destroying debug heap %p\n", DbgMem->hHeap);

    AcquireSRWLockExclusive(&DbgMem->Lock);

    if (DbgMem->Allocations != 0 || DbgMem->AllocatedMemory != 0)
    {
        LOG("***Leaking %lu bytes of memory in %lu blocks!***\n", DbgMem->AllocatedMemory, DbgMem->Allocations);
        if (DbgMem->Allocations != 0) DumpMemList(hUcaMem);
        bLeak = TRUE;
    }

    ReleaseSRWLockExclusive(&DbgMem->Lock);

    if (bLeak)
        RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE , 0, NULL);

    hHeap = DbgMem->hHeap;
    HeapFree(DbgMem->hHeap, 0, DbgMem);
    HeapDestroy(hHeap);
}


PVOID
UcaMemAlloc_dbg(HANDLE hUcaMem, UINT Flags, size_t size, const char * file, int line)
{
    PDEBUG_MEM_INFO DbgMem = (PDEBUG_MEM_INFO)hUcaMem;
    PVOID mem = NULL;

    AcquireSRWLockExclusive(&DbgMem->Lock);

    mem = HeapAlloc(DbgMem->hHeap, Flags, (CalculateSizeWithRedzone(size)));
    if (mem != NULL)
    {
        DbgMem->Allocations++;
        DbgMem->AllocatedMemory += size;
        AddMemToList(hUcaMem, mem);
        mem = WriteRedzone(mem, size, file, line);
    }

    ReleaseSRWLockExclusive(&DbgMem->Lock);

    return mem;
}

PVOID
UcaMemReAlloc_dbg(HANDLE hUcaMem, UINT Flags, PVOID ptr, size_t size, const char *file, int line)
{
    PDEBUG_MEM_INFO DbgMem = (PDEBUG_MEM_INFO)hUcaMem;
    BOOL bOutOfBounds = FALSE;
    size_t prev_size;
    PVOID mem = NULL;

    if (ptr == NULL)
        return UcaMemAlloc_dbg(hUcaMem, Flags, size, file, line);
    if (size == 0)
    {
        UcaMemFree_dbg(hUcaMem, 0, ptr, file, line);
        return NULL;
    }

    AcquireSRWLockExclusive(&DbgMem->Lock);

    ptr = GetBasePtr(ptr);
    prev_size = ((PMEM_ALLOC_INFO)ptr)->size;
    bOutOfBounds = CheckRedzone(ptr, file, line);
    DeleteMemFromList(ptr);

    mem = HeapReAlloc(DbgMem->hHeap, Flags, ptr, CalculateSizeWithRedzone(size));
    if (mem != NULL)
    {
        DbgMem->AllocatedMemory += size - prev_size;
        AddMemToList(hUcaMem, mem);
        mem = WriteRedzone(mem, size, file, line);
    }
    else
    {
        AddMemToList(hUcaMem, ptr);
    }

    ReleaseSRWLockExclusive(&DbgMem->Lock);

    if (bOutOfBounds)
        RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE , 0, NULL);

    return mem;
}

VOID
UcaMemFree_dbg(HANDLE hUcaMem, UINT Flags, PVOID ptr, const char *file, int line)
{
    PDEBUG_MEM_INFO DbgMem = (PDEBUG_MEM_INFO)hUcaMem;
    BOOL bOutOfBounds = FALSE;

    AcquireSRWLockExclusive(&DbgMem->Lock);

    if (ptr != NULL)
    {
        ptr = GetBasePtr(ptr);
        bOutOfBounds = CheckRedzone(ptr, file, line);
        DbgMem->Allocations--;
        DbgMem->AllocatedMemory -= ((PMEM_ALLOC_INFO)ptr)->size;
        DeleteMemFromList(ptr);
    }

    HeapFree(DbgMem->hHeap, Flags, ptr);

    ReleaseSRWLockExclusive(&DbgMem->Lock);

    if (bOutOfBounds)
        RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE , 0, NULL);
}

BOOL
UcaMemCheckBuffer_dbg(HANDLE hUcaMem, PVOID ptr, const char *file, int line)
{
    PDEBUG_MEM_INFO DbgMem = (PDEBUG_MEM_INFO)hUcaMem;
    BOOL bOutOfBounds = FALSE;

    AcquireSRWLockShared(&DbgMem->Lock);

    if (ptr != NULL)
    {
        ptr = GetBasePtr(ptr);
        bOutOfBounds = CheckRedzone(ptr, file, line);
    }

    ReleaseSRWLockShared(&DbgMem->Lock);

    if (bOutOfBounds)
        RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE , 0, NULL);

    return !bOutOfBounds;
}
