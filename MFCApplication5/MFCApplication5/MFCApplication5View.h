
// MFCApplication5View.h : interface of the CMFCApplication5View class
//

#pragma once


class CMFCApplication5View : public CView
{
protected: // create from serialization only
	CMFCApplication5View();
	DECLARE_DYNCREATE(CMFCApplication5View)

// Attributes
public:
	CMFCApplication5Doc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CMFCApplication5View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MFCApplication5View.cpp
inline CMFCApplication5Doc* CMFCApplication5View::GetDocument() const
   { return reinterpret_cast<CMFCApplication5Doc*>(m_pDocument); }
#endif

