#include "stdafx.h"
#include "EventTrace.h"
#include <Objbase.h>

CEventTrace::CEventTrace(void) :
    m_TraceHandle(NULL)
{
    ZeroMemory(&m_EventTraceProperties, sizeof(EVENT_TRACE_PROPERTIES));
}

CEventTrace::~CEventTrace(void)
{
    m_TraceProviders.clear();
}

DWORD
CEventTrace::Create(_In_z_ wstring TraceName,
                    _In_z_ wstring RootDirectory)
{
    // Copy the name and dir
    m_TraceName = TraceName;
    m_RootDirectory = RootDirectory;

    // Create a trace session guid
    CoInitialize(NULL);
    HRESULT hr = CoCreateGuid(&m_TraceSession);
    CoUninitialize();

    // bail if we failed to create the guid
    if (FAILED(hr)) return HRESULT_CODE(hr);

    return ERROR_SUCCESS;
}

DWORD
CEventTrace::AddTraceProvider(_In_ GUID &TraceGuid)
{
    std::vector<GUID>::iterator it;

    // Check that this provider doesn't already exist
    it = std::find(m_TraceProviders.begin(), m_TraceProviders.end(), TraceGuid);
    if (it == m_TraceProviders.end())
    {
        // Add it
        m_TraceProviders.push_back(TraceGuid);
        return ERROR_SUCCESS;
    }

    return ERROR_ALREADY_EXISTS;
}

DWORD
CEventTrace::DeleteTraceProvider(_In_ GUID &TraceGuid)
{
    std::vector<GUID>::iterator it;
    
    // Find the guid in the list
    it = std::find(m_TraceProviders.begin(), m_TraceProviders.end(), TraceGuid);
    if (it != m_TraceProviders.end())
    {
        // Remove it
        m_TraceProviders.erase(it);
        return ERROR_SUCCESS;
    }

    return ERROR_NOT_FOUND;
}
