#include "stdafx.h"
#include "TraceManager.h"


class TraceProvider
{
public:
    GUID ProviderGuid;
    ULONGLONG KeywordsAny;
    ULONGLONG KeywordsAll;
    DWORD Level;
    DWORD Properties;
    wstring Filter;

    TraceProvider(
        _In_ GUID _ProviderGuid,
        _In_ ULONGLONG _KeywordsAny,
        _In_ ULONGLONG _KeywordsAll,
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

CTraceManager::CTraceManager(void) :
    m_TraceHandle(NULL),
    m_EventTraceProperties(NULL)
{
    m_TraceName = L"New Trace Session";
    m_FileInformation.LogFileDirectory = L"%LOCALAPPDATA%";
}

CTraceManager::~CTraceManager(void)
{
    m_TraceProviders.clear();
}

DWORD
CTraceManager::Create(_In_z_ LPWSTR TraceName,
                      _In_z_ LPWSTR TraceDirectory)
{
    // Copy the name and dir
    m_TraceName = TraceName;

    // Create a trace session guid
    CoInitialize(NULL);
    HRESULT hr = CoCreateGuid(&m_TraceSessionGuid);
    CoUninitialize();

    // Bail if we failed to create the guid
    if (FAILED(hr)) return HRESULT_CODE(hr);

    m_FileInformation.LogFileame = TraceName;
    m_FileInformation.LogFileDirectory = TraceDirectory;

    // Set the string sizes
    size_t LoggerNameSize = (m_TraceName.size() + 1) * sizeof(wchar_t);
    size_t LogFileNameSize = MAX_PATH * sizeof(wchar_t);

    // Allocate the memory
    size_t BufferSize;
    BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + LoggerNameSize + LogFileNameSize;
    m_EventTraceProperties = (PEVENT_TRACE_PROPERTIES)new byte[BufferSize];
    if (m_EventTraceProperties == nullptr) return ERROR_NOT_ENOUGH_MEMORY;

    ZeroMemory(m_EventTraceProperties, BufferSize);

    // Setup the wnode header
    m_EventTraceProperties->Wnode.BufferSize = BufferSize;
    m_EventTraceProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    m_EventTraceProperties->Wnode.ClientContext = 1; // consider using 2
    CopyMemory(&m_EventTraceProperties->Wnode.Guid, &m_TraceSessionGuid, sizeof(GUID));

    SYSTEMTIME SystemTime;
    GetSystemTime(&SystemTime);
    SystemTimeToFileTime(&SystemTime, (LPFILETIME)&m_EventTraceProperties->Wnode.TimeStamp);

    // Set the defaults
    m_EventTraceProperties->BufferSize = 8; //kb
    m_EventTraceProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;

    // Set the string offsets
    m_EventTraceProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    m_EventTraceProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + LoggerNameSize;

    // Copy the trace name
    CopyMemory((LPWSTR)((byte *)m_EventTraceProperties + m_EventTraceProperties->LoggerNameOffset),
               m_TraceName.c_str(),
               LoggerNameSize);

    wstring LogFile = m_FileInformation.LogFileDirectory + L"\\" + m_FileInformation.LogFileame + L".etl";
    CopyMemory((LPWSTR)((byte *)m_EventTraceProperties + m_EventTraceProperties->LogFileNameOffset),
               LogFile.c_str(),
               LogFile.size() * sizeof(wchar_t));

    return ERROR_SUCCESS;
}

DWORD
CTraceManager::AddTraceProvider(_In_ GUID &ProviderGuid,
                               _In_ ULONGLONG KeywordsAny,
                               _In_ ULONGLONG KeywordsAll,
                               _In_ DWORD Level,
                               _In_ DWORD Properties,
                               _In_z_ LPWSTR Filter)
{

    for (auto Prov : m_TraceProviders)
    {
        if (IsEqualGUID(Prov->ProviderGuid, ProviderGuid))
        {
            return ERROR_ALREADY_EXISTS;
        }
    }

    ENABLE_TRACE_PARAMETERS EnableTraceParameters;
    EnableTraceParameters.Version = ENABLE_TRACE_PARAMETERS_VERSION;
    EnableTraceParameters.EnableProperty = EVENT_ENABLE_PROPERTY_TS_ID;// | EVENT_ENABLE_PROPERTY_STACK_TRACE;
    EnableTraceParameters.ControlFlags = 0;
    EnableTraceParameters.EnableFilterDesc = NULL;
    CopyMemory(&EnableTraceParameters.SourceId, &m_TraceSessionGuid, sizeof(GUID));

    DWORD dwError;
    dwError = EnableTraceEx2(m_TraceHandle,
                   &ProviderGuid,
                   0,
                   0,
                   KeywordsAny,
                   KeywordsAll,
                   0,
                   &EnableTraceParameters);
    if (dwError == ERROR_SUCCESS)
    {
        TraceProvider *Provider = new TraceProvider(
                ProviderGuid,
                KeywordsAny,
                KeywordsAll,
                Level,
                Properties,
                Filter);

        // Add it
        m_TraceProviders.push_back(Provider);
    }

    return dwError;
}

DWORD
CTraceManager::DeleteTraceProvider(_In_ GUID &ProviderGuid)
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
CTraceManager::SetTraceBuffers(_In_ DWORD BufferSize,
                              _In_ DWORD MinimumBuffers,
                              _In_ DWORD MaximumBuffers)
{
    if (m_EventTraceProperties == nullptr)
        return ERROR_BAD_CONFIGURATION;

    m_EventTraceProperties->BufferSize = BufferSize;
    m_EventTraceProperties->MinimumBuffers = MinimumBuffers;
    m_EventTraceProperties->MaximumBuffers = MaximumBuffers;

    return ERROR_SUCCESS;
}

DWORD
CTraceManager::SetStreamMode(_In_ StreamMode eStreamMode,
                            _In_ FileInformation *fileInformation)
{
    if (m_EventTraceProperties == nullptr)
        return ERROR_BAD_CONFIGURATION;

    if (eStreamMode == File || eStreamMode == FileAndRealTime)
    {
        m_FileInformation.LogFileame = fileInformation->LogFileame;
        m_FileInformation.LogFileDirectory = fileInformation->LogFileDirectory;
        m_FileInformation.Append = fileInformation->Append;
        m_FileInformation.Circular = fileInformation->Circular;

        m_EventTraceProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;

        if (eStreamMode == FileAndRealTime)
        {
            m_EventTraceProperties->LogFileMode |= EVENT_TRACE_REAL_TIME_MODE;
        }
    }
    else if (eStreamMode == RealTime)
    {
        m_EventTraceProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }

    return 0;
}

DWORD
CTraceManager::StartTraceSession()
{
    if (m_EventTraceProperties == nullptr)
        return ERROR_BAD_CONFIGURATION;

    if (m_TraceName.empty())
        return ERROR_BAD_CONFIGURATION;

    DWORD dwError;
    dwError = StartTraceW(&m_TraceHandle,
                          m_TraceName.c_str(),
                          m_EventTraceProperties);
    if (dwError == ERROR_SUCCESS)
    {
        for (TraceProvider *Prov : m_TraceProviders)
        {
            dwError = EnableTrace(TRUE, 0, 0, &Prov->ProviderGuid, m_TraceHandle);
            if (dwError != ERROR_SUCCESS)
                break;
        }
    }

    if (dwError != ERROR_SUCCESS)
    {
        StopTraceSession();
    }

    return dwError;
}

DWORD
CTraceManager::StopTraceSession()
{
    if (m_EventTraceProperties == nullptr)
        return ERROR_BAD_CONFIGURATION;

    if (m_TraceName.empty())
        return ERROR_BAD_CONFIGURATION;

    return StopTraceW(m_TraceHandle,
                       m_TraceName.c_str(),
                       m_EventTraceProperties);
}

/* PRIVATE METHODS ******************************************/

DWORD
CTraceManager::GetProvider(_In_ GUID &ProviderGuid,
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