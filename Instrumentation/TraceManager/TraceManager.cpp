#include "stdafx.h"
#include <TraceManagerLib.h>

int wmain(int argc, WCHAR* argv[])
{
    DWORD dwError;
    
    HANDLE TraceHandle;
    dwError = CreateTraceSession(L"MyNewTraceSession", L"C:\\LogFiles", &TraceHandle);
    if (dwError == ERROR_SUCCESS)
    {
        //dwError = AddTraceProvider(&GUID_NULL,
        //                           0,
        //                           0,
        //                           0,
        //                           0,
        //                           NULL);
        if (dwError == ERROR_SUCCESS)
        {
            StartTraceSession(TraceHandle);

            StopTraceSession(TraceHandle);
        }
    }

    return 0;
}
