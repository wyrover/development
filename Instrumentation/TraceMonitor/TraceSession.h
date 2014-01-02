#pragma once
#include "TraceProvider.h"

class CTraceSession
{
private:
    HANDLE m_TraceHandle;

public:
    CTraceSession(void);
    ~CTraceSession(void);

    bool Create(
        _In_z_ LPWSTR TraceName,
        _In_z_ LPWSTR TraceDirectory
        );

    bool Start(void);

    bool Stop(void);

    bool AddProvider(
        _In_ CTraceProvider *TraceProvider
        );

    bool DeleteProvider(
        _In_ CTraceProvider *TraceProvider
        );

};

