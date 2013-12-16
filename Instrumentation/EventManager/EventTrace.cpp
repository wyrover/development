#include "stdafx.h"
#include "EventTrace.h"


CEventTrace::CEventTrace(void)
{
    ZeroMemory(&m_EventTraceProperties, sizeof(EVENT_TRACE_PROPERTIES));
}


CEventTrace::~CEventTrace(void)
{
}
