#pragma once

#ifdef _DEBUG_MEM

#define UcaRtlCreateHeap() UcaMemCreate_dbg()
#define UcaRtlDestroyHeap(handle) UcaMemDestroy_dbg(handle)
#define UcaRtlAllocateHeap(handle, Flags, size) UcaMemAlloc_dbg(handle, Flags, size, __FILE__, __LINE__)
#define UcaRtlReAllocateHeap(handle, Flags, ptr, size) UcaMemReAlloc_dbg(handle, Flags, ptr, size, __FILE__, __LINE__)
#define UcaRtlFreeHeap(handle, Flags,  ptr) UcaMemFree_dbg(handle, Flags, ptr, __FILE__, __LINE__)
#define UcaRtlCheckHeap(handle, ptr) UcaMemCheckBuffer_dbg(handle, ptr, __FILE__, __LINE__)

HANDLE
UcaMemCreate_dbg(
    );

VOID
UcaMemDestroy_dbg(
    HANDLE hUcaMem
    );

PVOID
UcaMemAlloc_dbg(
    HANDLE hUcaMem,
    UINT Flags,
    size_t size,
    const char *file,
    int line
    );

PVOID
UcaMemReAlloc_dbg(
    HANDLE hUcaMem,
    UINT Flags,
    LPVOID ptr,
    size_t size,
    const char *file,
    int line
    );

VOID
UcaMemFree_dbg(
    HANDLE hUcaMem,
    UINT Flags,
    LPVOID ptr,
    const char *file,
    int line
    );

BOOL
UcaMemCheckBuffer_dbg(
    HANDLE hUcaMem,
    LPVOID ptr,
    const char *file, int line
    );

#else /* _DEBUG_MEM */



#define UcaRtlCreateHeap() GetProcessHeap()
#define UcaRtlDestroyHeap(handle)
#define UcaRtlAllocateHeap(handle, Flags, Size) HeapAlloc(handle, Flags, Size)
#define UcaRtlReAllocateHeap(handle, Flags, ptr, Size) HeapReAlloc(handle, Flags, ptr, Size)
#define UcaRtlFreeHeap(handle, Flags, ptr) HeapFree(handle, Flags, ptr)
#define UcaRtlCheckHeap(handle, ptr) TRUE

#endif
