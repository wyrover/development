#include "stdafx.h"
#include "EventTrace.h"
#include <Objbase.h>


class TraceProvider
{
public:
    GUID ProviderGuid;
    DWORD KeywordsAny;
    DWORD KeywordsAll;
    DWORD Level;
    DWORD Properties;
    wstring Filter;

    TraceProvider(
        _In_ GUID _ProviderGuid,
        _In_ DWORD _KeywordsAny,
        _In_ DWORD _KeywordsAll,
        _In_ DWORD _Level,
        _In_ DWORD _Properties,
        _In_ wstring _Filter) :
        ProviderGuid(_ProviderGuid),
        KeywordsAny(_KeywordsAny),
        KeywordsAll(_KeywordsAll),
        Level(_Level),
        Properties(_Properties),
        Filter(_Filter)
    {
    }
};

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
CEventTrace::Create(_In_z_ wstring TraceName)
{
    // Copy the name and dir
    m_TraceName = TraceName;

    // Create a trace session guid
    CoInitialize(NULL);
    HRESULT hr = CoCreateGuid(&m_TraceSession);
    CoUninitialize();

    // Bail if we failed to create the guid
    if (FAILED(hr)) return HRESULT_CODE(hr);

    // Set the string sizes
    size_t LoggerNameSize = (TraceName.size() + 1) * sizeof(wchar_t);
    size_t LogFileNameSize = MAX_PATH * sizeof(wchar_t);

    // Allocate the memory
    size_t BufferSize;
    BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + LoggerNameSize + LogFileNameSize;
    m_EventTraceProperties = (PEVENT_TRACE_PROPERTIES)new byte[BufferSize];

    // Set the offsets
    m_EventTraceProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    m_EventTraceProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + LoggerNameSize;

    // Copy the trace name
    CopyMemory((LPWSTR)((byte *)m_EventTraceProperties + m_EventTraceProperties->LoggerNameOffset),
               TraceName.c_str(),
               LoggerNameSize);

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

    for (auto Prov : m_TraceProviders)
    {
        if (IsEqualGUID(Prov->ProviderGuid, ProviderGuid))
        {
            return ERROR_ALREADY_EXISTS;
        }
    }

    TraceProvider *Provider = new TraceProvider(
        ProviderGuid,
        KeywordsAny,
        KeywordsAll,
        Level,
        Properties,
        Filter);

    // Add it
    m_TraceProviders.push_back(Provider);
    return ERROR_SUCCESS;
}

DWORD
CEventTrace::DeleteTraceProvider(_In_ GUID &ProviderGuid)
{
    // Find the guid in the list
    TraceProvider *Provider;
    DWORD dwError = GetProvider(ProviderGuid, true, &Provider);
    if (dwError == ERROR_SUCCESS)
    {
        delete Provider;
    }
    return dwError;
}

DWORD
CEventTrace::SetTraceBuffers(_In_ DWORD BufferSize,
                             _In_ DWORD MinimumBuffers,
                             _In_ DWORD MaximumBuffers)
{
    return 0;
}

DWORD
CEventTrace::SetStreamMode(_In_ StreamMode eStreamMode,
                           _In_ FileInformation *fileInformation)
{
    return 0;
}


/* PRIVATE METHODS ******************************************/

DWORD
CEventTrace::GetProvider(_In_ GUID &ProviderGuid,
                         _In_ bool Remove,
                         _Out_ TraceProvider **Provider)
{
    bool bFound = false;
    for (auto Prov = m_TraceProviders.begin(); Prov != m_TraceProviders.end(); Prov++)
    {
        if (IsEqualGUID((*Prov)->ProviderGuid, ProviderGuid))
        {
            if (Remove)
                m_TraceProviders.erase(Prov);

            *Provider = *Prov;
            bFound = true;
        }
    }

    return bFound ? ERROR_SUCCESS : ERROR_NOT_FOUND;
}