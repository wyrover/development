
// MFCApplication5.h : main header file for the MFCApplication5 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMFCApplication5App:
// See MFCApplication5.cpp for the implementation of this class
//

class CMFCApplication5App : public CWinAppEx
{
public:
	CMFCApplication5App();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMFCApplication5App theApp;
