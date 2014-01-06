#pragma once

#define DPRINT(ch, lvl, fmt, ...) DbgPrintEx(DPFLTR_##ch##_ID, DPFLTR_##lvl##_LEVEL,    \
                                             "%I64u ([P:%lu T:%lu] %s:%d)  " fmt "\n",  \
                                             SharedUserData->TickCountQuad,             \
                                             HandleToUlong(PsGetCurrentProcessId()),    \
                                             HandleToUlong(PsGetCurrentThreadId()),     \
                                             __FILE__, __LINE__, ##__VA_ARGS__)


#define TRACE_REGISTER(Guid, Type, Handle) ((PVOID)*Handle) = NULL
#define TRACE_UNREGISTER(Handle) 

#define TRACE_ENTER(Handle)                 DPRINT(IHVDRIVER, TRACE, "Entered %s", __FUNCTION__)
#define TRACE_EXIT(Handle)                  DPRINT(IHVDRIVER, TRACE, "Leaving %s", __FUNCTION__)

#define TRACE_INFO(Handle, fmt, ...)        DPRINT(IHVDRIVER, INFO, fmt, ##__VA_ARGS__)
#define TRACE_WARNING(Handle, fmt, ...)     DPRINT(IHVDRIVER, WARNING, fmt, ##__VA_ARGS__)
#define TRACE_ERROR(Handle, fmt, ...)       DPRINT(IHVDRIVER, ERROR, fmt, ##__VA_ARGS__)
#define TRACE_CRITICAL(Handle, fmt, ...)                        \
do                                                              \
{                                                               \
    DPRINT(IHVDRIVER, ERROR, "CRITICAL:" fmt, ##__VA_ARGS__);   \
    ASSERT(FALSE);                                              \
} while (0)
