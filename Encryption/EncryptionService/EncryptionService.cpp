// EncryptionService.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "EncryptionService_i.h"
#include "EncryptionDriver.h"

using namespace ATL;

#include <stdio.h>

class CEncryptionServiceModule : public ATL::CAtlServiceModuleT< CEncryptionServiceModule, IDS_SERVICENAME >
{
    CEncryptionDriver *m_EncryptionDriver;

public :
	DECLARE_LIBID(LIBID_EncryptionServiceLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ENCRYPTIONSERVICE, "{28B5F41C-FA40-4AB4-BE27-B634EBEF219A}")
		HRESULT InitializeSecurity() throw()
	{
		// TODO : Call CoInitializeSecurity and provide the appropriate security settings for your service
		// Suggested - PKT Level Authentication, 
		// Impersonation Level of RPC_C_IMP_LEVEL_IDENTIFY 
		// and an appropriate Non NULL Security Descriptor.

		return S_OK;
	}

    HRESULT PreMessageLoop(_In_ int nShowCmd) throw();
    HRESULT PostMessageLoop() throw();
};

CEncryptionServiceModule _AtlModule;


HRESULT CEncryptionServiceModule::PreMessageLoop(_In_ int nShowCmd) throw()
{
    HRESULT hr = __super::PreMessageLoop(nShowCmd);
    if (FAILED(hr)) return hr;

    m_EncryptionDriver = new CEncryptionDriver();
    DWORD Status = m_EncryptionDriver->Initialize();
    if (Status != ERROR_SUCCESS)
        return S_FALSE;

    m_status.dwCurrentState = SERVICE_RUNNING;
    ::SetServiceStatus(m_hServiceStatus, &m_status);

    return S_OK;
}

HRESULT CEncryptionServiceModule::PostMessageLoop() throw()
{
    m_EncryptionDriver->Uninitialize();
    delete m_EncryptionDriver;
    m_EncryptionDriver = nullptr;

    return __super::PostMessageLoop();
}

//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
								LPTSTR /*lpCmdLine*/, int nShowCmd)
{
	return _AtlModule.WinMain(nShowCmd);
}

