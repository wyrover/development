#include "TdhUtil.h"
#include "TxtTrace.h"

ULONG
CBaseTrace::FormatProperty(__in PEVENT_RECORD Event,
                           __in PTRACE_EVENT_INFO EventInfo,
                           __in_opt PEVENT_MAP_INFO EventMapInfo,
                           __in PEVENT_PROPERTY_INFO Property,
                           __in USHORT PropertyLength,
                           __in ULONG PropertyIndex,
                           __inout PPROCESSING_CONTEXT LogContext)
/*++
Routine Description:
    This routine prepares the formatting of the raw byte data contained 
    in the property buffer. First, the offset from Event->UserData is calculated.
    Then the data in that offset is passed for concrete formatting in 
    GetFormattedBuffer() or FormatMapToString(). These methods return the 
    formatted  buffer and the amount of binary data consumed and use the amount 
    of data consumed to advance the current data offset.

Arguments:
    Event - Supplies the structure representing an event.
    EventInfo - Supplies the event meta-information.
    Property - Supplies the property information about the simple property 
               to be decoded.
    PropertyLength - Supplies the length of the simple property to be decoded.
    PropertyIndex - Supplies the index of the property to be decoded.
    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:
    ERROR_SUCCESS - Success
    Win32 error code - Formatting the property data failed.
--*/
{
    ULONG Status = ERROR_SUCCESS;
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    ULONG BufferSize = DataContext->BufferSize;
    USHORT PointerSize;
    ULONG DataLeft;
    FPTR_TDH_FORMATPROPERTY TdhFormatPropertyPtr = NULL;

    DataContext->BinDataLeft = Event->UserDataLength - DataContext->UserDataOffset;
    PBYTE Data = (PBYTE)Event->UserData + DataContext->UserDataOffset;

    // If no more data, just fill the buffer with one non-printable UNICODE_NULL.
    if (DataContext->BinDataLeft == 0)
    {
        Status = NullToBuffer(DataContext->Buffer, DataContext->BufferSize, &DataContext->BinDataConsumed);
        if (Status == ERROR_SUCCESS)
            TdhpUpdateRenderItem(DataContext);

        return Status;
    }

    // Get the pointer size on the machine where the event was fired.
    // Will be needed later when decoding certain types of properies.
    if ((Event->EventHeader.Flags & EVENT_HEADER_FLAG_64_BIT_HEADER) != 0)
        PointerSize = sizeof(ULONGLONG);
    else if ((Event->EventHeader.Flags & EVENT_HEADER_FLAG_32_BIT_HEADER) != 0)
        PointerSize = sizeof(ULONG);
    else
        PointerSize = (USHORT)LogContext->PointerSize;

    do
    {
        if (Status == ERROR_INSUFFICIENT_BUFFER)
        {
            Status = TdhpResizeBuffer(&DataContext->Buffer,
                                  &DataContext->BufferSize,
                                  ((DataContext->BufferSize / MIN_PROP_BUFFERSIZE) + 1) * MIN_PROP_BUFFERSIZE);
            if (Status != ERROR_SUCCESS)
                return Status;
        }

        // Check if the Windows 7 TDH API routine, TdhFormatProperty(), is avaliable.
        if ((LogContext->TdhDllHandle != NULL))
            TdhFormatPropertyPtr = LogContext->FormatPropertyPtr;

        if (TdhFormatPropertyPtr != NULL)
        {
            // The decoding process is on Windows 7 or later. In Windows 7, the TDH API
            // is updated with several new functions. One of them is TdhFormatProperty, which
            // deals with all valid TDH InTypes and OutTypes, and formats them properly.
            // In order to get the sample to compile on both Vista and Windows 7, load the 
            // TdhFormatProperty() dynamically.
            Status = (*TdhFormatPropertyPtr)(EventInfo,
                                             EventMapInfo,
                                             PointerSize,
                                             Property->nonStructType.InType,
                                             Property->nonStructType.OutType,
                                             PropertyLength,
                                             DataContext->BinDataLeft,
                                             Data,
                                             &BufferSize,
                                             (PWSTR)DataContext->Buffer,
                                             &DataContext->BinDataConsumed);

        }
        else
        {
            // The operating system is prior to Windows 7. The formatting for each 
            // InType and OutType property must be handled manually.
            if (EventMapInfo == NULL)
            {
                // This property has no map associated with it.  Directly pass the buffer 
                // referenced by the current offset for formatting to GetFormattedBuffer().
                // According to the in- and out-types of the property, proper formatting will 
                // be performed.
                Status = TdhpGetFormattedBuffer(Data,
                                            DataContext->BinDataLeft,
                                            PropertyLength,
                                            PointerSize,
                                            Property->nonStructType.InType,
                                            Property->nonStructType.OutType,
                                            DataContext->Buffer,
                                            DataContext->BufferSize,
                                            &DataContext->BinDataConsumed);
            }
            else
            {
                // This property has map associated with it. The map key value is 
                // in the Data buffer pointed by the property. It is a number pointing to 
                // some resource. GetFormattedMapValue() will find and format both the key
                // and its resource value and will return the formatted value as result.
                Status = TdhpGetFormattedMapValue(Data,
                                              DataContext->BinDataLeft,  
                                              EventMapInfo,
                                              Property->nonStructType.InType,
                                              DataContext->Buffer,
                                              DataContext->BufferSize,
                                              &DataContext->BinDataConsumed);
            }
        }

    } while (Status == ERROR_INSUFFICIENT_BUFFER);

    if (Status == ERROR_EVT_INVALID_EVENT_DATA)
    {
        // There can be cases when the string represented by the buffer Data, is 
        // not aligned with the event payload (i.e. it is longer than the actual data left).
        // Just copy and format the last DataContext->BinDataLeft bytes from the payload.
        if (Property->nonStructType.InType == TDH_INTYPE_UNICODESTRING)
        {
            DataLeft = DataContext->BinDataLeft;
            if (DataContext->BufferSize < DataLeft)
            {
                Status = TdhpResizeBuffer(&DataContext->Buffer,
                                      &DataContext->BufferSize,
                                      ((DataContext->BufferSize / MIN_PROP_BUFFERSIZE) + 1) * MIN_PROP_BUFFERSIZE);

                if (Status != ERROR_SUCCESS)
                    return Status;
            }
            RtlCopyMemory(DataContext->Buffer, Data, DataLeft);
            DataContext->Buffer[DataLeft] = 0;
            DataContext->Buffer[DataLeft + 1] = 0;
            DataContext->BinDataConsumed = (USHORT)DataLeft; 
            Status = ERROR_SUCCESS;

        }
        else if (Property->nonStructType.InType == TDH_INTYPE_ANSISTRING)
        {
            DataLeft = DataContext->BinDataLeft;
            BufferSize = (DataLeft + 1) * sizeof(WCHAR);
            if (DataContext->BufferSize < BufferSize)
            {
                Status = TdhpResizeBuffer(&DataContext->Buffer,
                                          &DataContext->BufferSize,
                                          ((DataContext->BufferSize / MIN_PROP_BUFFERSIZE) + 1) * MIN_PROP_BUFFERSIZE);
                if (Status != ERROR_SUCCESS)
                    return Status;
            }

            DataContext->BinDataConsumed = (USHORT)MultiByteToWideChar(CP_ACP,
                                                                       0,
                                                                       (PSTR)Data,
                                                                       DataLeft,
                                                                       (PWSTR)DataContext->Buffer,
                                                                       DataLeft);
            DataLeft *= sizeof(WCHAR);
            DataContext->Buffer[DataLeft] = 0;
            DataContext->Buffer[DataLeft + 1] = 0;
            Status = ERROR_SUCCESS;
        }
        else if (EventMapInfo != NULL)
        {
            // The integer key stored in Data was not matched as a valid map key entry.
            // Just try to print the formatted integer stored in Data.
            Status = FormatProperty(Event,
                                    EventInfo,
                                    NULL,
                                    Property,
                                    PropertyLength,
                                    PropertyIndex,
                                    LogContext);
        }
    }

    if (Status == ERROR_SUCCESS)
    {
        DataContext->UserDataOffset += DataContext->BinDataConsumed;
        TdhpUpdateRenderItem(DataContext);
    }

    return Status;
}

VOID
CBaseTrace::SaveReferenceValues(__in PEVENT_PROPERTY_INFO Property,
                                __in USHORT PropertyIndex,
                                __in PBYTE Data,
                                __inout PPROCESSING_DATA_CONTEXT DataContext)
/*++
Routine Description:
    This routine caches the value of a simple property, which is not an array,
    in ReferenceValues. This value can be further referenced as a property
    length or array count.

Arguments:
    Property - Supplies the single property whose value will be cached.
    PropertyIndex - Supplies the index of the property whose value will be cached.
    Data - Supplies the raw byte property value to be cached.
    DataContext - Container of the cache (ReferenceValues).

Return Value:
    None.
--*/
{
    // Only integer values can be cached. Find the integer type of the 
    // Property value and based on the type, do the proper formatting
    // and cache the result in DataContext->ReferenceValues.
    USHORT InType = Property->nonStructType.InType;

    // If Data is from a simple integer property whose value is NULL, ignore it.
    if (Data == NULL) return;

    if (InType == TDH_INTYPE_UINT8)
    {
        UINT8 Value;
        RtlCopyMemory(&Value, Data, sizeof(UINT8));
        DataContext->ReferenceValues[PropertyIndex] = Value;
    }
    else if (InType == TDH_INTYPE_UINT16)
    {
        UINT16 Value;
        RtlCopyMemory(&Value, Data, sizeof(UINT16));
        DataContext->ReferenceValues[PropertyIndex] = Value;
    }
    else if ((InType == TDH_INTYPE_UINT32) || (InType == TDH_INTYPE_HEXINT32))
    {
        UINT32 Value;
        RtlCopyMemory(&Value, Data, sizeof(UINT32));
        DataContext->ReferenceValues[PropertyIndex] = Value;
    }
}

USHORT
CBaseTrace::GetArrayCount(__in PEVENT_PROPERTY_INFO Property,
                          __in PULONG ReferenceValues)
/*++
Routine Description:
    This routine retrieves the number of elements in a single property (simple 
    or complex).

Arguments:
    Property - Supplies the property whose number of elements will be retrieved.
    ReferenceValues - Supplies previously stored simple property values, which can
                      potentionally be referenced as a property length or array count.

Return Value:
    1 - When the property is not an array or an array with 1 member.
    Otherwise - The number of elements in the array.
--*/
{
    if ((Property->Flags & PropertyParamCount) != 0)
        return (USHORT)ReferenceValues[Property->countPropertyIndex];
    else
        return Property->count;
}

USHORT
CBaseTrace::GetPropertyLength(__in PEVENT_PROPERTY_INFO Property,
                              __in PULONG ReferenceValues)
/*++
Routine Description:
    This routine retrieves the buffer length of Property.

Arguments:
    Property - Supplies the property whose buffer length will be retrieved.
    ReferenceValues - Supplies previously stored simple property values, which can
                      potentionally be referenced as a property length or array count.

Return Value:
    The length of the property buffer. 
--*/
{
    if ((Property->Flags & PropertyParamLength) != 0)
    {
        // The property is not a fixed size property. Search the cache
        // ReferenceValues for its length.
        return (USHORT)ReferenceValues[Property->lengthPropertyIndex];
    }
    else
    {
        // The property is a fixed size property. Its length is stored
        // in the property information structure.
        return Property->length;
    }
}

ULONG
CBaseTrace::GetSimpleType(__in PEVENT_RECORD Event,
                          __in PTRACE_EVENT_INFO EventInfo,
                          __in PEVENT_PROPERTY_INFO Property,
                          __in USHORT PropertyIndex,
                          __inout PPROCESSING_CONTEXT LogContext)
/*++
Routine Description:
    This routine iterates over each property member in the
    simple property and passes it to the property formatting 
    function FormatProperty().  In case of single simple types, 
    only one iteration is performed.

Arguments:
    Event - Supplies the structure representing an event.
    EventInfo - Supplies the event meta-information.
    Property - Supplies the property information about the simple property 
               to be decoded.
    PropertyIndex - Supplies the index of the property to be decoded.
    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:
    ERROR_SUCCESS - Success.
    Win32 error code - Failure in obtaining the resulting map association for the property
                       or in formatting the property data.
--*/
{
    ULONG Status = ERROR_SUCCESS;
    PEVENT_MAP_INFO EventMapInfo = NULL;
    USHORT ArrayCount;
    USHORT PropertyLength;
    USHORT InType = Property->nonStructType.InType;
    USHORT OutType = Property->nonStructType.OutType;
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    PBYTE Data = (PBYTE)Event->UserData + DataContext->UserDataOffset;

    // Get the number of property elements. In the case where the property
    // is an array, the number of array members is stored in ArrayCount;
    // otherwise ArrayCount = 1.
    ArrayCount = GetArrayCount(Property, DataContext->ReferenceValues);

    // There are two special cases where the ArrayCount is equivalent to
    // the PropertyLength.
    if (((InType == TDH_INTYPE_UNICODECHAR) || (InType == TDH_INTYPE_ANSICHAR)) &&
        (OutType == TDH_OUTTYPE_STRING))
    {
        PropertyLength = ArrayCount;
        ArrayCount = 1;
    }
    else
    {
        PropertyLength = GetPropertyLength(Property, DataContext->ReferenceValues);
    }

    // Iterate through each member of the array represented by the simple property.
    // In the case of a simple single property, just format its data (ArrayCount = 1).
    for (USHORT Counter = 0; Counter < ArrayCount; Counter++)
    {
        Status = FormatProperty(Event,
                                EventInfo,
                                EventMapInfo,
                                Property,
                                PropertyLength,
                                PropertyIndex,
                                LogContext);
        if (Status != ERROR_SUCCESS) return Status;
    }

    if (ArrayCount == 1)
    {
        // This is single simple single type (not an array), with the value stored 
        // in the Data variable (computed in FormatProperty). As it may be
        // referenced later, as some property length or array count, it should be cached
        // for eventual further useage.
        SaveReferenceValues(Property, PropertyIndex, Data, DataContext);
    }

    return Status;
}

ULONG
CBaseTrace::GetComplexType(__in PEVENT_RECORD Event,
                           __in PTRACE_EVENT_INFO EventInfo,
                           __in PEVENT_PROPERTY_INFO ComplexProperty,
                           __inout PPROCESSING_CONTEXT LogContext)
/*++
Routine Description:
    This routine iterates over each simple property member in the complex 
    property, calculates its index, and passes it to DumpSimpleType().
    Complex properties can contain only simple properties (including arrays 
    as simple types).

Arguments:
    Event - Supplies the structure representing an event.
    EventInfo - Supplies the event meta-information.
    ComplexProperty - Supplies the property information about the complex property 
                      to be decoded.
    LogContext - Supplies the structure that persists contextual information
                 across callbacks.

Return Value:
    ERROR_SUCCESS - Success.
    Win32 error code - DumpSimpleType() failed.
--*/
{
    ULONG Status = ERROR_SUCCESS;
    PEVENT_PROPERTY_INFO SimpleProperty;
    ULONG SimplePropertyCount;
    USHORT ArrayCount;
    PPROCESSING_DATA_CONTEXT DataContext = &LogContext->DataContext;
    
    //VPrintFToFile(FALSE,
    //              LogContext,
    //              L"\r\n\t\t<ComplexData Name=\"%s\">",
    //              TEI_PROPERTY_NAME(EventInfo, ComplexProperty));

    // Get the number of structures if the the complex property is an 
    // array of structures.
    // N.B. A Complex property can be an array of structures, but cannot 
    // contain members that are structures.
    ArrayCount = GetArrayCount(ComplexProperty, DataContext->ReferenceValues);
    
    for (USHORT I = 0; I < ArrayCount; I++)
    {
        SimplePropertyCount = ComplexProperty->structType.NumOfStructMembers;
        SimpleProperty = &EventInfo->EventPropertyInfoArray[ComplexProperty->structType.StructStartIndex];

        for (USHORT J = 0; J < SimplePropertyCount; J++, SimpleProperty++)
        {
            // Dump the J-th simple member from the I-th structure. 
            Status = GetSimpleType(Event,
                                   EventInfo,
                                   SimpleProperty,
                                   ComplexProperty->structType.StructStartIndex + J,
                                   LogContext);

            if (Status != ERROR_SUCCESS)
                return Status;
        }
    }

    //VPrintFToFile(FALSE, LogContext, L"\r\n\t\t</ComplexData>");

    return Status;
}

DWORD
CBaseTrace::GetTraceEventInfo(__in PEVENT_RECORD Event,
                              __out PTRACE_EVENT_INFO* EventInfo)
/*++
Routine Description:
    This routine retrieves the TRACE_EVENT_INFO structure for the
    passed EVENT_RECORD Event. This structure contains the meta-
    information about the event.

Arguments:
    Event - Supplies the structure representing an event.
    EventInfo - Receives the event meta-information.

Return Value:
    ERROR_SUCCESS - Success.
    Win32 error code - TdhGetEventInformation() failed.
--*/
{
    DWORD Status = ERROR_SUCCESS;
    PPROCESSING_DATA_CONTEXT DataContext= &m_LogContext.DataContext;
    ULONG BufferSize = DataContext->EventInfoBufferSize;

    do
    {
        if (Status == ERROR_INSUFFICIENT_BUFFER)
        {
            Status = TdhpResizeBuffer(&DataContext->EventInfoBuffer,
                                      &DataContext->EventInfoBufferSize,
                                      BufferSize);
            if (DataContext->EventInfoBuffer == NULL)
            {
                return ERROR_OUTOFMEMORY;
            }

            DataContext->EventInfoBufferSize = BufferSize;
        }

        Status = TdhGetEventInformation(Event,
                                        0,
                                        NULL,
                                        (PTRACE_EVENT_INFO)DataContext->EventInfoBuffer,
                                        &BufferSize);

    } while (Status == ERROR_INSUFFICIENT_BUFFER);
    
    if (Status == ERROR_SUCCESS)
    {
        *EventInfo = (PTRACE_EVENT_INFO)DataContext->EventInfoBuffer;
    }

    return Status;
}

DWORD
CBaseTrace::OutputEventData(__in PEVENT_RECORD Event,
                            __in PTRACE_EVENT_INFO EventInfo)
{
    PPROCESSING_DATA_CONTEXT DataContext = &m_LogContext.DataContext;
    PEVENT_PROPERTY_INFO Property;
    DWORD Status = ERROR_SUCCESS;

    /* Init context info */
    DataContext->LastTopLevelIndex = -1;
    DataContext->BinDataLeft = Event->UserDataLength;
    DataContext->UserDataOffset = 0;

    if (EventInfo->TopLevelPropertyCount > 0)
    {
        // Allocated array of ULONGs for storing the simple integer property types.
        // This array can potentially be used for referencing some further property
        // array count or buffer length.
        DataContext->ReferenceValuesCount = EventInfo->PropertyCount;
        DataContext->ReferenceValues = (PULONG)malloc(DataContext->ReferenceValuesCount * sizeof(LONG));
        if (DataContext->ReferenceValues == NULL) {
            return ERROR_OUTOFMEMORY;
        }

        // Allocated array of strings, which will store the formatted values for each top-level
        // property. This array will be used in the end for formatting the event message. Alongside
        // this array, mantain another array of BOOLEANs which will hold the flags if some render
        // item was filled.
        DataContext->RenderItemsCount = EventInfo->TopLevelPropertyCount;
        DataContext->RenderItems = (PWSTR*)malloc(DataContext->RenderItemsCount * sizeof(PWSTR));
        if (DataContext->RenderItems == NULL) {
            return ERROR_OUTOFMEMORY;
        }

        // Iterate through each of the top-level properties and dump it acording to its type.
        for (USHORT Index = 0; Index < EventInfo->TopLevelPropertyCount; Index++)
        {
            DataContext->CurrentTopLevelIndex = Index;
            Property = &EventInfo->EventPropertyInfoArray[Index];

            if (PROPERTY_IS_STRUCTURE(Property))
            {
                Status = GetComplexType(Event,
                                        EventInfo,
                                        Property,
                                        &m_LogContext);
            }
            else
            {
                Status = GetSimpleType(Event,
                                       EventInfo,
                                       Property,
                                       Index,
                                       &m_LogContext);
            }

            if (Status != ERROR_SUCCESS)
                break;
        }
    }

    if (Status == ERROR_SUCCESS)
    {
        if (OutputEvent(Event, EventInfo))
        {
            m_LogContext.EventOutCount++;
        }
        else
        {
            Status = GetLastError();
        }
    }

    return Status;
}

DWORD
CBaseTrace::HandleEvent(__in PEVENT_RECORD Event)
/*++
Routine Description:
    This routine decodes a single Event and prints it to standard output.
    First, the event header is dumped, then the event data, and lastly, 
    the formatted event message.

Arguments:
    Event - Supplies the structure representing an event.

Return Value:
    ERROR_SUCCESS - Success.
    Win32 error codes - Failure in the dumping process.
--*/
{
    PTRACE_EVENT_INFO EventInfo = NULL;
    PPROCESSING_DATA_CONTEXT DataContext = &m_LogContext.DataContext;
    LPWSTR EventMessage = NULL;
    DWORD Status;

    Status = GetTraceEventInfo(Event, &EventInfo);
    if (Status == ERROR_SUCCESS)
    {
        m_LogContext.EventProcCount++;

        Status = OutputEventData(Event, EventInfo);
    }

    // Release the resources used for decoding the event payload.
    TdhpResetDataContext(DataContext);

    return Status;
}

VOID
WINAPI
CBaseTrace::EventCallback(__in PEVENT_RECORD Event)
/*++
Routine Description:
    This routine is called by ProcessTrace() for every event in the ETL file.
    It receives an EVENT_RECORD parameter, which contains the events header 
    and the event payload.

Arguments:
    Event - Supplies the structure that represents an Event.

Return Value:
    None.
--*/
{
    CBaseTrace *This = (CBaseTrace *)Event->UserContext;

    if ((Event->EventHeader.ProviderId == EventTraceGuid) &&
        (Event->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO))
    {
        // First event in every file is a header event, some information
        // from which is needed to correctly decode subsequent events.
        // N.B. This event is not available if consuming events in real-time mode.

        PTRACE_LOGFILE_HEADER LogHeader = (PTRACE_LOGFILE_HEADER)Event->UserData;

        if (LogHeader != NULL)
        {
            This->m_LogContext.TimerResolution = LogHeader->TimerResolution;
            This->m_LogContext.PointerSize = LogHeader->PointerSize;
            This->m_LogContext.IsPrivateLogger = (BOOLEAN)(LogHeader->LogFileMode &
                                                           EVENT_TRACE_PRIVATE_LOGGER_MODE);
        }

        This->OutputHeader(Event);
    }
    else if ((Event->EventHeader.Flags & EVENT_HEADER_FLAG_TRACE_MESSAGE) != 0)
    {
        // Ignore WPP events.
    }
    else
    {
        This->HandleEvent(Event);
    }
}

ULONG
WINAPI
CBaseTrace::BufferCallback(__in PEVENT_TRACE_LOGFILEW LogFile)
/*++
Routine Description:
    An ETL file is divided into a number of buffers that contain events.
    This routine is called by ProcessTrace() after all the events in a buffer
    are delivered.

Arguments:
    LogFile -  A pointer to the structure that contains information
               about the buffer.

Return Value:
    TRUE - Continue processing.
    FALSE - Returning false cancels processing and ProcessTrace() returns.
            This is the only way to cancel processing. This sample will always
            return true.
--*/
{
    CBaseTrace *This = (CBaseTrace *)LogFile->Context;
    This->m_LogContext.BufferCount++;
    return TRUE;
}

DWORD
CBaseTrace::ProcessTrace()
{
    TRACEHANDLE Handle;
    ULONG Status;

    ZeroMemory(&m_LogFile, sizeof(EVENT_TRACE_LOGFILEW));

    m_LogFile.LogFileName = m_pDumpInfo->EtlFileName;
    m_LogFile.ProcessTraceMode |= PROCESS_TRACE_MODE_EVENT_RECORD;
    m_LogFile.EventRecordCallback = EventCallback;
    m_LogFile.BufferCallback = BufferCallback;
    m_LogFile.Context = (PVOID)this;

    Handle = OpenTraceW(&m_LogFile);
    if (Handle == INVALID_PROCESSTRACE_HANDLE)
    {
        Status = GetLastError();
        return Status;
    }

    Status = ::ProcessTrace(&Handle, 1, NULL, NULL);
    if (Status != ERROR_SUCCESS)
    {
        wprintf(L"\nProcessTrace failed. Error code: %u.\n", Status);
    }

    CloseTrace(Handle);

    return Status;
}

DWORD
CBaseTrace::InitializeTrace(__in PDUMP_INFO DumpInfo)
{
    m_pDumpInfo = DumpInfo;
    return TdhpInitializeProcessingContext(&m_LogContext);
}

DWORD
WINAPI
DecodeFile(__in PDUMP_INFO DumpInfo,
           __inout PULONG BufferCount,
           __inout PULONGLONG EventProcCount,
           __inout PULONGLONG EventOutCount)
{
    CBaseTrace *TraceManager = NULL;
    ULONG Status;

    switch (DumpInfo->DumpType)
    {
    case DUMP_XML:
        //TraceManager = new CXmlTrace();
        break;

    case DUMP_TXT:
        TraceManager = new CTxtTrace();
        break;

    case DUMP_SQL:
        //TraceManager = new CSqlTrace();
        break;
    }

    Status = TraceManager->InitializeTrace(DumpInfo);
    if (Status == ERROR_SUCCESS)
    {
        Status = TraceManager->ProcessTrace();
        if (Status == ERROR_SUCCESS)
        {
            TraceManager->GetStatistics(BufferCount, EventProcCount, EventOutCount);
        }
    }

    delete TraceManager;

    return Status;
}