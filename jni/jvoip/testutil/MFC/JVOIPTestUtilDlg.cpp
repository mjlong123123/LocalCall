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

#include "stdafx.h"
#include "JVOIPTestUtil.h"
#include "JVOIPTestUtilDlg.h"
#include "jvoiprtptransmission.h"
#include "jvoipsessionparams.h"
#include <time.h>
#include <stdio.h>
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STR_INPUTSAMPRATE					"InputSamplingRate"
#define STR_OUTPUTSAMPRATE					"OutputSamplingRate"
#define STR_INPUTSAMPENC					"InputSampleEncoding"
#define STR_OUTPUTSAMPENC					"OutputSampleEncoding"
#define STR_SAMPINTERVAL					"SamplingInterval"
#define STR_RTPPORTBASE						"RTPPortBase"
#define STR_INPUTTYPE						"InputType"
#define STR_OUTPUTTYPE						"OutputType"
#define STR_COMPTYPE						"CompressionType"
#define	STR_DESTIP						"DestinationIP"
#define STR_DESTPORT						"DestinationPort"

#define WM_JVOIPLIB_THREADERR					(WM_USER+42)

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJVOIPTestUtilDlg dialog


int CJVOIPTestUtilDlg::samprates[]={4000,8000,11025,22050,44100};
JVOIPSessionParams::SampleEncodingType CJVOIPTestUtilDlg::enctypes[]={JVOIPSessionParams::EightBit,JVOIPSessionParams::SixteenBit};
JVOIPSessionParams::VoiceInputType CJVOIPTestUtilDlg::inputtypes[]={JVOIPSessionParams::SoundcardInput,JVOIPSessionParams::NoInput};
JVOIPSessionParams::VoiceOutputType CJVOIPTestUtilDlg::outputtypes[]={JVOIPSessionParams::SoundcardOutput,JVOIPSessionParams::NoOutput};
JVOIPSessionParams::CompressionType CJVOIPTestUtilDlg::comptypes[]={JVOIPSessionParams::NoCompression,JVOIPSessionParams::ULawEncoding,
						       JVOIPSessionParams::DPCM,JVOIPSessionParams::GSM,JVOIPSessionParams::LPC};

CJVOIPTestUtilDlg *CJVOIPTestUtilDlg::wnd;
WNDPROC CJVOIPTestUtilDlg::OldEditSampIntHandler;
WNDPROC CJVOIPTestUtilDlg::OldEditPortBaseHandler;
WNDPROC CJVOIPTestUtilDlg::OldEditDestIPHandler;
WNDPROC CJVOIPTestUtilDlg::OldEditDestPortHandler;

CJVOIPTestUtilDlg::CJVOIPTestUtilDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CJVOIPTestUtilDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CJVOIPTestUtilDlg)
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	wnd = this;

	WSADATA dat;

	WSAStartup(0x0202,&dat);
}

CJVOIPTestUtilDlg::~CJVOIPTestUtilDlg()
{
	WSACleanup();
}

void CJVOIPTestUtilDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CJVOIPTestUtilDlg)
	DDX_Control(pDX, IDC_LIST_COMPINFO, list_compinf);
	DDX_Control(pDX, IDC_EDIT_SAMPINT, edit_sampint);
	DDX_Control(pDX, IDC_EDIT_PORTBASE, edit_portbase);
	DDX_Control(pDX, IDC_EDIT_LOG, edit_log);
	DDX_Control(pDX, IDC_EDIT_DESTPORT, edit_destport);
	DDX_Control(pDX, IDC_EDIT_DESTIP, edit_destip);
	DDX_Control(pDX, IDC_COMBO_OUTTYPE, combo_outtype);
	DDX_Control(pDX, IDC_COMBO_OUTRATE, combo_outrate);
	DDX_Control(pDX, IDC_COMBO_OUTENC, combo_outenc);
	DDX_Control(pDX, IDC_COMBO_INTYPE, combo_intype);
	DDX_Control(pDX, IDC_COMBO_INRATE, combo_inrate);
	DDX_Control(pDX, IDC_COMBO_INENC, combo_inenc);
	DDX_Control(pDX, IDC_COMBO_COMPTYPE, combo_comptype);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CJVOIPTestUtilDlg, CDialog)
	//{{AFX_MSG_MAP(CJVOIPTestUtilDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CREATE, OnSessionCreate)
	ON_BN_CLICKED(IDC_BUTTON_DESTROY, OnSessionDestroy)
	ON_BN_CLICKED(IDC_BUTTON_QUIT, OnQuit)
	ON_CBN_SELCHANGE(IDC_COMBO_COMPTYPE, OnChangeCompression)
	ON_CBN_SELCHANGE(IDC_COMBO_INENC, OnChangeInputEncoding)
	ON_CBN_SELCHANGE(IDC_COMBO_INRATE, OnChangeInputRate)
	ON_CBN_SELCHANGE(IDC_COMBO_INTYPE, OnChangeInputType)
	ON_CBN_SELCHANGE(IDC_COMBO_OUTRATE, OnChangeOutputRate)
	ON_CBN_SELCHANGE(IDC_COMBO_OUTENC, OnChangeOutputEncoding)
	ON_CBN_SELCHANGE(IDC_COMBO_OUTTYPE, OnChangeOutputType)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_JVOIPLIB_THREADERR,OnThreadError)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJVOIPTestUtilDlg message handlers

BOOL CJVOIPTestUtilDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// Set some default values

	combo_inrate.SetCurSel(1);
	combo_outrate.SetCurSel(1);
	combo_inenc.SetCurSel(0);
	combo_outenc.SetCurSel(0);
	edit_sampint.SetWindowText("20");
	edit_portbase.SetWindowText("5000");
	combo_intype.SetCurSel(0);
	combo_outtype.SetCurSel(0);
	combo_comptype.SetCurSel(1);
	edit_destip.SetWindowText("127.0.0.1");
	edit_destport.SetWindowText("5000");
	
	loadSettings();

	// create the headers of the listbox

	list_compinf.InsertColumn(0,"Component name",LVCFMT_LEFT,140,-1);
	list_compinf.InsertColumn(1,"Component parameters",LVCFMT_LEFT,366,-1);
	
	// Nasty stuff

	HWND hWnd;

	hWnd = ::GetDlgItem(GetSafeHwnd(),IDC_EDIT_SAMPINT);
	OldEditSampIntHandler = (WNDPROC) SetWindowLong(hWnd,GWL_WNDPROC,(LONG)NewEditSampIntHandler);

	hWnd = ::GetDlgItem(GetSafeHwnd(),IDC_EDIT_PORTBASE);
	OldEditPortBaseHandler = (WNDPROC) SetWindowLong(hWnd,GWL_WNDPROC,(LONG)NewEditPortBaseHandler);
	
	hWnd = ::GetDlgItem(GetSafeHwnd(),IDC_EDIT_DESTIP);
	OldEditDestIPHandler = (WNDPROC) SetWindowLong(hWnd,GWL_WNDPROC,(LONG)NewEditDestIPHandler);
	
	hWnd = ::GetDlgItem(GetSafeHwnd(),IDC_EDIT_DESTPORT);
	OldEditDestPortHandler = (WNDPROC) SetWindowLong(hWnd,GWL_WNDPROC,(LONG)NewEditDestPortHandler);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CJVOIPTestUtilDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CJVOIPTestUtilDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CJVOIPTestUtilDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CJVOIPTestUtilDlg::OnSessionCreate() 
{
	JVOIPSessionParams sessparams;
	JVOIPRTPTransmissionParams rtpparams;
	int status;
	char buf[1025];
	int val;

	if (JVOIPSession::IsActive())
	{
		printLog("Session already active");
		return;
	}
	
	// obtain the values from the combo boxes and edit fields and store
	// them in the session parameters.
	
	sessparams.SetInputSamplingRate(samprates[combo_inrate.GetCurSel()]);
	sessparams.SetOutputSamplingRate(samprates[combo_outrate.GetCurSel()]);
	sessparams.SetInputSampleEncodingType(enctypes[combo_inenc.GetCurSel()]);
	sessparams.SetOutputSampleEncodingType(enctypes[combo_outenc.GetCurSel()]);
	
	edit_sampint.GetWindowText(buf,1024);
	val = 0;
	sscanf(buf,"%d",&val);
	sessparams.SetSampleInterval(val);

	sessparams.SetVoiceInputType(inputtypes[combo_intype.GetCurSel()]);
	sessparams.SetVoiceOutputType(outputtypes[combo_outtype.GetCurSel()]);
	sessparams.SetCompressionType(comptypes[combo_comptype.GetCurSel()]);

	// set some values for our RTP transmission module

	rtpparams.SetAcceptOwnPackets(true);
	
	edit_portbase.GetWindowText(buf,1024);
	val = 0;
	sscanf(buf,"%d",&val);
	rtpparams.SetPortBase(val);

	sessparams.SetTransmissionParams(&rtpparams);

	// try to create the session

	status = JVOIPSession::Create(sessparams);
	if (status < 0)
	{
		printLog(JVOIPGetErrorString(status).c_str());
		return;
	}

	// save the destination IP and port and try to add this destination

	edit_destip.GetWindowText(buf,1024);
	destip = ntohl(inet_addr(buf));

	edit_destport.GetWindowText(buf,1024);
	destport = 0;
	sscanf(buf,"%d",&destport);

	status = JVOIPSession::AddDestination(destip,destport);
	if (status < 0)
	{
		JVOIPSession::Destroy();
		printLog(JVOIPGetErrorString(status).c_str());
		return;
	}
	printLog("Created VoIP session");
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::OnSessionDestroy() 
{
	JVOIPSession::Destroy();
	printLog("Destroyed VoIP session");
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::OnQuit() 
{
	saveSettings();
	JVOIPSession::Destroy();
	CDialog::OnOK();
}

void CJVOIPTestUtilDlg::OnOK() 
{
}

void CJVOIPTestUtilDlg::OnCancel() 
{
	saveSettings();
	CDialog::OnCancel();
}

void CJVOIPTestUtilDlg::OnChangeCompression() 
{
	char line[4096];
	char compname[256];
	int index;

	if (!JVOIPSession::IsActive())
		return;

	index = combo_comptype.GetCurSel();
	if (handleError(JVOIPSession::SetCompressionType(comptypes[index],NULL)))
		return;
	switch(comptypes[index])
	{
	case JVOIPSessionParams::NoCompression:
		strcpy(compname,"Raw PCM data");
		break;
	case JVOIPSessionParams::ULawEncoding:
		strcpy(compname,"Mu-law encoding");
		break;
	case JVOIPSessionParams::DPCM:
		strcpy(compname,"DPCM");
		break;
	case JVOIPSessionParams::GSM:
		strcpy(compname,"GSM");
		break;
	case JVOIPSessionParams::LPC:
		strcpy(compname,"LPC");
		break;
	default:
		strcpy(compname,"???");
	}
	sprintf(line,"Compression type set to %s",compname);
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::OnChangeInputEncoding() 
{
	char line[4096];
	int index;

	if (!JVOIPSession::IsActive())
		return;
	index = combo_inenc.GetCurSel();
	if (handleError(JVOIPSession::SetInputSampleEncodingType(enctypes[index])))
		return;
	sprintf(line,"Input encoding set to %d bits",(enctypes[index] == JVOIPSessionParams::EightBit)?8:16);
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::OnChangeInputRate() 
{
	char line[4096];
	int index;

	if (!JVOIPSession::IsActive())
		return;
	index = combo_inrate.GetCurSel();
	if (handleError(JVOIPSession::SetInputSamplingRate(samprates[index])))
		return;
	sprintf(line,"Input sampling rate set to %d",samprates[index]);
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::OnChangeInputType() 
{
	char line[4096];
	int index;

	if (!JVOIPSession::IsActive())
		return;
	index = combo_intype.GetCurSel();
	if (handleError(JVOIPSession::SetVoiceInputType(inputtypes[index],NULL)))
		return;
	sprintf(line,"Input encoding set to '%s'",(inputtypes[index] == JVOIPSessionParams::NoInput)?"No input":"Soundcard input");
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::OnChangeOutputRate() 
{
	char line[4096];
	int index;

	if (!JVOIPSession::IsActive())
		return;
	index = combo_outrate.GetCurSel();
	if (handleError(JVOIPSession::SetOutputSamplingRate(samprates[index])))
		return;
	sprintf(line,"Output sampling rate set to %d",samprates[index]);
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::OnChangeOutputEncoding() 
{
	char line[4096];
	int index;

	if (!JVOIPSession::IsActive())
		return;
	index = combo_outenc.GetCurSel();
	if (handleError(JVOIPSession::SetOutputSampleEncodingType(enctypes[index])))
		return;
	sprintf(line,"Output encoding set to %d bits",(enctypes[index] == JVOIPSessionParams::EightBit)?8:16);
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::OnChangeOutputType() 
{
	char line[4096];
	int index;

	if (!JVOIPSession::IsActive())
		return;
	index = combo_outtype.GetCurSel();
	if (handleError(JVOIPSession::SetVoiceOutputType(outputtypes[index],NULL)))
		return;
	sprintf(line,"Output encoding set to '%s'",(outputtypes[index] == JVOIPSessionParams::NoOutput)?"No output":"Soundcard output");
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::loadSettings()
{
	char filename[4096];
	char line[4096];
	FILE *f;

	getSaveFileName(filename);
	printLog("Attempting to load settings");
	f = fopen(filename,"rt");
	if (f == NULL)
	{
		sprintf(line,"Unable to open settings file %s, using defaults",filename);
		printLog(line);
		return;
	}

	while (fgets(line,4095,f) != NULL)
	{
		char option[4096],arg[4096];
		int val;
		
		if (sscanf(line,"%s %s",option,arg) == 2)
		{
			sscanf(arg,"%d",&val);		
			
			if (strcmp(option,STR_INPUTSAMPRATE) == 0)
				combo_inrate.SetCurSel(val);
			else if (strcmp(option,STR_OUTPUTSAMPRATE) == 0)
				combo_outrate.SetCurSel(val);
			else if (strcmp(option,STR_INPUTSAMPENC) == 0)
				combo_inenc.SetCurSel(val);
			else if (strcmp(option,STR_OUTPUTSAMPENC) == 0)
				combo_outenc.SetCurSel(val);
			else if (strcmp(option,STR_SAMPINTERVAL) == 0)
				edit_sampint.SetWindowText(arg);
			else if (strcmp(option,STR_RTPPORTBASE) == 0)
				edit_portbase.SetWindowText(arg);
			else if (strcmp(option,STR_INPUTTYPE) == 0)
				combo_intype.SetCurSel(val);
			else if (strcmp(option,STR_OUTPUTTYPE) == 0)
				combo_outtype.SetCurSel(val);
			else if (strcmp(option,STR_COMPTYPE) == 0)
				combo_comptype.SetCurSel(val);
			else if (strcmp(option,STR_DESTIP) == 0)
				edit_destip.SetWindowText(arg);
			else if (strcmp(option,STR_DESTPORT) == 0)
				edit_destport.SetWindowText(arg);
		}
	}
	fclose(f);
	printLog("Settings loaded");
}

void CJVOIPTestUtilDlg::saveSettings()
{
	char filename[4096];
	char buf[1025];
	FILE *f;

	getSaveFileName(filename);
	f = fopen(filename,"wt");
	if (f == NULL)
	{
		printLog("Can't write settings!");
		return;
	}

	fprintf(f,"%s %d\n",STR_INPUTSAMPRATE,combo_inrate.GetCurSel());
	fprintf(f,"%s %d\n",STR_OUTPUTSAMPRATE,combo_outrate.GetCurSel());
	fprintf(f,"%s %d\n",STR_INPUTSAMPENC,combo_inenc.GetCurSel());
	fprintf(f,"%s %d\n",STR_OUTPUTSAMPENC,combo_outenc.GetCurSel());
	edit_sampint.GetWindowText(buf,1024);
	fprintf(f,"%s %s\n",STR_SAMPINTERVAL,buf);
	edit_portbase.GetWindowText(buf,1024);
	fprintf(f,"%s %s\n",STR_RTPPORTBASE,buf);
	fprintf(f,"%s %d\n",STR_INPUTTYPE,combo_intype.GetCurSel());
	fprintf(f,"%s %d\n",STR_OUTPUTTYPE,combo_outtype.GetCurSel());
	fprintf(f,"%s %d\n",STR_COMPTYPE,combo_comptype.GetCurSel());
	edit_destip.GetWindowText(buf,1024);
	fprintf(f,"%s %s\n",STR_DESTIP,buf);
	edit_destport.GetWindowText(buf,1024);
	fprintf(f,"%s %s\n",STR_DESTPORT,buf);
	fclose(f);
}

void CJVOIPTestUtilDlg::printLog(const char line[])
{
	char logline[4096];
	struct tm *loctime;
	time_t thetime;

	thetime = time(NULL);
	loctime = localtime(&thetime);
	sprintf(logline,"%02d:%02d:%02d %s\r\n",loctime->tm_hour,loctime->tm_min,loctime->tm_sec,line);

	int lines = edit_log.GetLineCount();
	int charnumber = edit_log.LineIndex(lines-1);
	charnumber += edit_log.LineLength(lines-1); //Select the last character	
	if (charnumber>-1)
	{
		edit_log.SetSel(0,-1);
		edit_log.SetSel(charnumber,charnumber);
		edit_log.ReplaceSel(LPCTSTR(logline)); //Replace selection with new string
	}
}

void CJVOIPTestUtilDlg::getSaveFileName(char filename[])
{
	char buf[1024];

	GetTempPath(1023,buf);
	strcpy(filename,buf);
	strcat(filename,"jvoiptestutil.ini");
}

bool CJVOIPTestUtilDlg::handleError(int err)
{
	if (err >= 0)
	{
		updateComponentInfo();
		return false;
	}
	printLog(JVOIPGetErrorString(err).c_str());
	return true;
}

void CJVOIPTestUtilDlg::updateComponentInfo()
{
	list_compinf.DeleteAllItems();

	if (!JVOIPSession::IsActive())
		return;

	addComponentInfo(JVOIPSession::VoiceInput);	
	addComponentInfo(JVOIPSession::VoiceOutput);	
	addComponentInfo(JVOIPSession::Compression);	
	addComponentInfo(JVOIPSession::Mixer);	
	addComponentInfo(JVOIPSession::Transmission);	
}

void CJVOIPTestUtilDlg::addComponentInfo(JVOIPSession::ComponentType comptype)
{
	std::vector<JVOIPCompParamInfo> *params = NULL;
	std::vector<JVOIPCompParamInfo>::const_iterator it;
	int index;

	index = list_compinf.InsertItem(0,JVOIPSession::GetComponentName(comptype).c_str());
	list_compinf.SetItemText(index,1,JVOIPSession::GetComponentDescription(comptype).c_str());
	JVOIPSession::GetComponentParameters(comptype,&params);
	if (params == NULL)
		return;
	for (it = params->begin() ; it != params->end() ; ++it)
	{
		char line[4096];

		sprintf(line,"    + %s",(*it).GetParameterName().c_str());
		index = list_compinf.InsertItem(index+1,line);
		list_compinf.SetItemText(index,1,(*it).GetParameterValue().c_str());
	}
	delete params;
}	

void CJVOIPTestUtilDlg::ThreadFinishedHandler(int threaderr,int voicecallerr,int componenterr)
{
	if (threaderr >= 0)
		return;

	err1 = threaderr;
	err2 = voicecallerr;
	err3 = componenterr;
	PostMessage(WM_JVOIPLIB_THREADERR,0,0);
}

LRESULT APIENTRY CJVOIPTestUtilDlg::NewEditSampIntHandler(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_CHAR && wParam == VK_RETURN)
	{
		wnd->onChangeSamplingInterval();
		return 0;
	}
	return ::CallWindowProc(OldEditSampIntHandler,hwnd,uMsg,wParam,lParam);
}

LRESULT APIENTRY CJVOIPTestUtilDlg::NewEditPortBaseHandler(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_CHAR && wParam == VK_RETURN)
	{
		wnd->onChangePortBase();
		return 0;
	}
	return ::CallWindowProc(OldEditSampIntHandler,hwnd,uMsg,wParam,lParam);
}

LRESULT APIENTRY CJVOIPTestUtilDlg::NewEditDestIPHandler(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_CHAR && wParam == VK_RETURN)
	{
		wnd->onChangeDestinationIP();
		return 0;
	}
	return ::CallWindowProc(OldEditSampIntHandler,hwnd,uMsg,wParam,lParam);
}

LRESULT APIENTRY CJVOIPTestUtilDlg::NewEditDestPortHandler(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_CHAR && wParam == VK_RETURN)
	{
		wnd->onChangeDestinationPort();
		return 0;
	}
	return ::CallWindowProc(OldEditSampIntHandler,hwnd,uMsg,wParam,lParam);
}

void CJVOIPTestUtilDlg::onChangeSamplingInterval()
{
	char line[4096];
	int val;

	if (!JVOIPSession::IsActive())
		return;

	edit_sampint.GetWindowText(line,4095);
	sscanf(line,"%d",&val);
	
	if (handleError(JVOIPSession::SetSampleInterval(val)))
		return;
	sprintf(line,"Sample interval set to %d ms",val);
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::onChangePortBase()
{
	JVOIPRTPTransmissionParams rtpparams;
	char line[4096];
	int val;

	if (!JVOIPSession::IsActive())
		return;

	// to change the port base, we have to install a new RTP transmission module
	// which is then initialized with the new value.
	edit_portbase.GetWindowText(line,4095);
	sscanf(line,"%d",&val);

	rtpparams.SetAcceptOwnPackets(true);
	rtpparams.SetPortBase(val);
	if (handleError(JVOIPSession::SetTransmissionType(JVOIPSessionParams::RTP,&rtpparams)))
		return;
	JVOIPSession::ClearDestinations();
	JVOIPSession::AddDestination(destip,destport);
	sprintf(line,"RTP portbase set to %d",val);
	printLog(line);
	updateComponentInfo();
}

void CJVOIPTestUtilDlg::onChangeDestinationIP()
{
	char line[1024];

	if (!JVOIPSession::IsActive())
		return;

	edit_destip.GetWindowText(line,4095);

	destip = ntohl(inet_addr(line));
	JVOIPSession::ClearDestinations();
	JVOIPSession::AddDestination(destip,destport);
	sprintf(line,"Destination address is now %d.%d.%d.%d:%d",
		(int)((destip>>24)&0xFF),(int)((destip>>16)&0xFF),
		(int)((destip>>8)&0xFF),(int)((destip)&0xFF),destport);
	printLog(line);
}

void CJVOIPTestUtilDlg::onChangeDestinationPort()
{
	char line[1024];
	int val;

	if (!JVOIPSession::IsActive())
		return;
		
	edit_destport.GetWindowText(line,4095);
	sscanf(line,"%d",&val);

	destport = val;
	JVOIPSession::ClearDestinations();
	JVOIPSession::AddDestination(destip,destport);
	sprintf(line,"Destination address is now %d.%d.%d.%d:%d",
		(int)((destip>>24)&0xFF),(int)((destip>>16)&0xFF),
		(int)((destip>>8)&0xFF),(int)((destip)&0xFF),destport);
	printLog(line);
}

LRESULT CJVOIPTestUtilDlg::OnThreadError(WPARAM wParam, LPARAM lParam)
{
	char line[4096];
		
	printLog("VoIP thread exited unexpectedly:");
	sprintf(line,"        VoIP Thread error: %s",JVOIPGetErrorString(err1).c_str());
	printLog(line);
	sprintf(line,"        VoiceCall error: %s",VoIPFramework::GetVoiceCallErrorString(err2));
	printLog(line);
			
	// NOTE: because we are certain that we didn't use any
	//       user defined components, we can use the JVOIPGetErrorString
	//       function to interpret the component error.
	//       If we did use user defined components, the returned
	//       error could also be user defined and the 
	//       JVOIPGetErrorString function would return incorrect
	//       information...
	sprintf(line,"        Component error: %s",JVOIPGetErrorString(err3).c_str());
	printLog(line);
	updateComponentInfo();	
	return 0;
}
