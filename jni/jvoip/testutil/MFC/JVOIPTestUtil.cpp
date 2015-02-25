#include "stdafx.h"
#include "JVOIPTestUtil.h"
#include "JVOIPTestUtilDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJVOIPTestUtilApp

BEGIN_MESSAGE_MAP(CJVOIPTestUtilApp, CWinApp)
	//{{AFX_MSG_MAP(CJVOIPTestUtilApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJVOIPTestUtilApp construction

CJVOIPTestUtilApp::CJVOIPTestUtilApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CJVOIPTestUtilApp object

CJVOIPTestUtilApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CJVOIPTestUtilApp initialization

BOOL CJVOIPTestUtilApp::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CJVOIPTestUtilDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
