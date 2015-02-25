/*

  This file is a part of the JVOIPLIB Test Utility
  Copyright (C) 2000-2005 Jori Liesenborgs

  Contact: jori@lumumba.uhasselt.be

  This file was developed at the 'Expertise Centre for Digital  Media' (EDM) 
  in Diepenbeek, Belgium (http://www.edm.uhasselt.be). The EDM is a research 
  institute of the Hasselt University (http://www.uhasselt.be).

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#if !defined(AFX_JVOIPTESTUTILDLG_H__DC30BD43_7C8F_4E79_99DD_96DF2EFC055F__INCLUDED_)
#define AFX_JVOIPTESTUTILDLG_H__DC30BD43_7C8F_4E79_99DD_96DF2EFC055F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "jvoipsession.h"

/////////////////////////////////////////////////////////////////////////////
// CJVOIPTestUtilDlg dialog

class CJVOIPTestUtilDlg : public CDialog,JVOIPSession
{
// Construction
public:
	CJVOIPTestUtilDlg(CWnd* pParent = NULL);	// standard constructor
	~CJVOIPTestUtilDlg();

// Dialog Data
	//{{AFX_DATA(CJVOIPTestUtilDlg)
	enum { IDD = IDD_JVOIPTESTUTIL_DIALOG };
	CListCtrl	list_compinf;
	CEdit	edit_sampint;
	CEdit	edit_portbase;
	CEdit	edit_log;
	CEdit	edit_destport;
	CEdit	edit_destip;
	CComboBox	combo_outtype;
	CComboBox	combo_outrate;
	CComboBox	combo_outenc;
	CComboBox	combo_intype;
	CComboBox	combo_inrate;
	CComboBox	combo_inenc;
	CComboBox	combo_comptype;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJVOIPTestUtilDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CJVOIPTestUtilDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSessionCreate();
	afx_msg void OnSessionDestroy();
	afx_msg void OnQuit();
	afx_msg void OnChangeCompression();
	afx_msg void OnChangeInputEncoding();
	afx_msg void OnChangeInputRate();
	afx_msg void OnChangeInputType();
	afx_msg void OnChangeOutputRate();
	afx_msg void OnChangeOutputEncoding();
	afx_msg void OnChangeOutputType();
	//}}AFX_MSG
	afx_msg LRESULT OnThreadError(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
private:
	void onChangeSamplingInterval();
	void onChangePortBase();
	void onChangeDestinationIP();
	void onChangeDestinationPort();

	void OnOK();
	void OnCancel();
	void loadSettings();
	void saveSettings();
	void printLog(const char line[]);
	void getSaveFileName(char filename[]);
	bool handleError(int err);
	void updateComponentInfo();
	void addComponentInfo(JVOIPSession::ComponentType comptype);	

	void ThreadFinishedHandler(int threaderr,int voicecallerr = 0,int componenterr = 0);

	static int samprates[];
	static JVOIPSessionParams::SampleEncodingType enctypes[];
	static JVOIPSessionParams::VoiceInputType inputtypes[];
	static JVOIPSessionParams::VoiceOutputType outputtypes[];
	static JVOIPSessionParams::CompressionType comptypes[];
	
	unsigned long destip;
	int destport;

	static CJVOIPTestUtilDlg *wnd;
	static WNDPROC OldEditSampIntHandler;
	static WNDPROC OldEditPortBaseHandler;
	static WNDPROC OldEditDestIPHandler;
	static WNDPROC OldEditDestPortHandler;
	static LRESULT APIENTRY NewEditSampIntHandler(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static LRESULT APIENTRY NewEditPortBaseHandler(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static LRESULT APIENTRY NewEditDestIPHandler(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static LRESULT APIENTRY NewEditDestPortHandler(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

	int err1,err2,err3;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JVOIPTESTUTILDLG_H__DC30BD43_7C8F_4E79_99DD_96DF2EFC055F__INCLUDED_)
