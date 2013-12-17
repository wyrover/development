#include "stdafx.h"
#include "EventTrace.h"
#include <Objbase.h>

CEventTrace::CEventTrace(void) :
    m_TraceHandle(NULL),
    m_EventTraceProperties(NULL)
{
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
CEventTrace::AddTraceProvider(_In_ GUID &ProviderGuid,
                              _In_ DWORD KeywordsAny,
                              _In_ DWORD KeywordsAll,
                              _In_ DWORD Level,
                              _In_ DWORD Properties,
                              _In_z_ wstring Filter)
{
    std::vector<GUID>::iterator it;

    // Check that this provider doesn't already exist
    it = std::find(m_TraceProviders.begin(), m_TraceProviders.end(), ProviderGuid);
    if (it == m_TraceProviders.end())
    {
        // Add it
        m_TraceProviders.push_back(ProviderGuid);
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

DWORD
CEventTrace::SetTraceBuffers(_In_ DWORD BufferSize,
                             _In_ DWORD MinimumBuffers,
                             _In_ DWORD MaximumBuffers)
{

}

DWORD
CEventTrace::SetStreamMode(_In_ StreamMode eStreamMode,
                           _In_ FileInformation *fileInformation)
{

}
