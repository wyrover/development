#include "stdafx.h"
#include "TraceSession.h"
#include "TraceManagerLib.h"

CTraceSession::CTraceSession(void) :
    m_TraceHandle(NULL)
{
}


CTraceSession::~CTraceSession(void)
{
}

bool
CTraceSession::Create(_In_z_ LPWSTR TraceName,
                      _In_z_ LPWSTR TraceDirectory)
{
    if (m_TraceHandle == NULL)
        return false;

    DWORD dwError;
    dwError = CreateTraceSession(TraceName,
                                 TraceDirectory,
                                 &m_TraceHandle);
    if (dwError != ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

bool
CTraceSession::Start()
{
    DWORD dwError;
    dwError = StartTraceSession(m_TraceHandle);
    if (dwError != ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

bool
CTraceSession::Stop()
{
    DWORD dwError;
    dwError = StopTraceSession(m_TraceHandle);
    if (dwError != ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

bool
CTraceSession::AddProvider(_In_ CTraceProvider *TraceProvider)
{
    DWORD dwError;
    dwError = AddTraceProvider(m_TraceHandle,
                               (LPGUID)TraceProvider->GetTraceGuid(),
                               TraceProvider->GetKeywordsAny(),
                               TraceProvider->GetKeywordsAll(),
                               TraceProvider->GetLevel(),
                               0,
                               NULL);
    if (dwError != ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

bool
CTraceSession::DeleteProvider(_In_ CTraceProvider *TraceProvider)
{
    DWORD dwError;
    dwError = DeleteTraceProvider(m_TraceHandle,
                                  (LPGUID)TraceProvider->GetTraceGuid());
    if (dwError != ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}
