#if !defined(AFX_JVOIPTESTUTIL_H__F4CB3157_61B6_4E5A_B981_9B9AE7CC1293__INCLUDED_)
#define AFX_JVOIPTESTUTIL_H__F4CB3157_61B6_4E5A_B981_9B9AE7CC1293__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CJVOIPTestUtilApp:
// See JVOIPTestUtil.cpp for the implementation of this class
//

class CJVOIPTestUtilApp : public CWinApp
{
public:
	CJVOIPTestUtilApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJVOIPTestUtilApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CJVOIPTestUtilApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JVOIPTESTUTIL_H__F4CB3157_61B6_4E5A_B981_9B9AE7CC1293__INCLUDED_)
