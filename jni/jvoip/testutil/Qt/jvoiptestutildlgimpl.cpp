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

#include "jvoiptestutildlgimpl.h"
#include "jvoiprtptransmission.h"
#include "jvoipsoundcardparams.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qevent.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// These strings are used in the file where we will store the current settings

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
#define STR_INPUTBOOST						"InputBoost"
#define STR_OUTPUTBOOST						"OutputBoost"

// some arrays to easily obtain parameters from the combo boxes

int JVOIPTestUtilDlgImpl::samprates[]={4000,8000,11025,22050,44100};
int JVOIPTestUtilDlgImpl::boosts[]={1,2,4,8};
JVOIPSessionParams::SampleEncodingType JVOIPTestUtilDlgImpl::enctypes[]={JVOIPSessionParams::EightBit,JVOIPSessionParams::SixteenBit};
JVOIPSessionParams::VoiceInputType JVOIPTestUtilDlgImpl::inputtypes[]={JVOIPSessionParams::SoundcardInput,JVOIPSessionParams::NoInput};
JVOIPSessionParams::VoiceOutputType JVOIPTestUtilDlgImpl::outputtypes[]={JVOIPSessionParams::SoundcardOutput,JVOIPSessionParams::NoOutput};
JVOIPSessionParams::CompressionType JVOIPTestUtilDlgImpl::comptypes[]={JVOIPSessionParams::NoCompression,JVOIPSessionParams::ULawEncoding,
						       JVOIPSessionParams::DPCM,JVOIPSessionParams::GSM,JVOIPSessionParams::LPC,JVOIPSessionParams::SilenceSuppression,JVOIPSessionParams::Speex};

// We override the JVOIPSession::ThreadFinishedHandler function. This function
// gets called whenever the main VoIP thread should suddenly stop. If that
// is the case, we will send the following event to our dialog box...

class JVOIPThreadStopEvent : public QEvent
{
public: 
	JVOIPThreadStopEvent(int threaderr,int voicecallerr,int componenterr) : QEvent(QEvent::User)
	{
		threaderror = threaderr;
		voicecallerror = voicecallerr;
		componenterror = componenterr;
	}

	int threadError()				{ return threaderror; }
	int voicecallError()				{ return voicecallerror; }
	int componentError()				{ return componenterror; }
private:
	int threaderror,voicecallerror,componenterror;
};

JVOIPTestUtilDlgImpl::JVOIPTestUtilDlgImpl(QWidget* parent, const char* name, bool modal, WFlags fl)
{
	setFixedSize(width(),height());

	combo_inrate->setCurrentItem(1);
	combo_outrate->setCurrentItem(1);
	combo_inenc->setCurrentItem(0);
	combo_outenc->setCurrentItem(0);
	edit_sampint->setText("20");
	edit_portbase->setText("5000");
	combo_intype->setCurrentItem(0);
	combo_outtype->setCurrentItem(0);
	combo_comptype->setCurrentItem(1);
	edit_destip->setText("127.0.0.1");
	edit_destport->setText("5000");
	combo_inputboost->setCurrentItem(0);
	combo_outputboost->setCurrentItem(0);

	edit_log->setReadOnly(true);
	list_compinfo->setRootIsDecorated(true);

	loadSettings();
	
	QObject::connect(bttn_quit,SIGNAL(clicked()),qApp,SLOT(quit()));
	QObject::connect(bttn_create,SIGNAL(clicked()),this,SLOT(onCreate()));
	QObject::connect(bttn_destroy,SIGNAL(clicked()),this,SLOT(onDestroy()));
	QObject::connect(combo_inrate,SIGNAL(activated(int)),this,SLOT(onChangeInputRate(int)));
	QObject::connect(combo_outrate,SIGNAL(activated(int)),this,SLOT(onChangeOutputRate(int)));
	QObject::connect(combo_inenc,SIGNAL(activated(int)),this,SLOT(onChangeInputEncoding(int)));
	QObject::connect(combo_outenc,SIGNAL(activated(int)),this,SLOT(onChangeOutputEncoding(int)));
	QObject::connect(edit_sampint,SIGNAL(returnPressed()),this,SLOT(onChangeSamplingInterval()));
	QObject::connect(edit_portbase,SIGNAL(returnPressed()),this,SLOT(onChangePortBase()));
	QObject::connect(combo_intype,SIGNAL(activated(int)),this,SLOT(onChangeInputType(int)));
	QObject::connect(combo_outtype,SIGNAL(activated(int)),this,SLOT(onChangeOutputType(int)));
	QObject::connect(combo_comptype,SIGNAL(activated(int)),this,SLOT(onChangeCompressionType(int)));
	QObject::connect(edit_destip,SIGNAL(returnPressed()),this,SLOT(onChangeDestinationIP()));
	QObject::connect(edit_destport,SIGNAL(returnPressed()),this,SLOT(onChangeDestinationPort()));
	QObject::connect(combo_inputboost,SIGNAL(activated(int)),this,SLOT(onChangeInputBoost(int)));
	QObject::connect(combo_outputboost,SIGNAL(activated(int)),this,SLOT(onChangeOutputBoost(int)));
}

JVOIPTestUtilDlgImpl::~JVOIPTestUtilDlgImpl() throw (JVOIPException)
{
	saveSettings();

	// IMPORTANT: The destroy function of the parent class must be explicitly called!
	//            Depending on the destructor is impossible due to inheritance issues.
	//            Refer to the manual for more information.

	JVOIPSession::Destroy();	
}

void JVOIPTestUtilDlgImpl::printLog(const char line[])
{
	QString qline;
	struct tm *loctime;
	time_t thetime;
	char timeinfo[256];
	
	thetime = time(NULL);
	loctime = localtime(&thetime);
	sprintf(timeinfo,"%02d:%02d:%02d ",loctime->tm_hour,loctime->tm_min,loctime->tm_sec);
	
	qline += timeinfo;
	qline += line;
	edit_log->insertLine(qline);
	edit_log->setCursorPosition(edit_log->numLines(),1);
}

void JVOIPTestUtilDlgImpl::loadSettings()
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
				combo_inrate->setCurrentItem(val);
			else if (strcmp(option,STR_OUTPUTSAMPRATE) == 0)
				combo_outrate->setCurrentItem(val);
			else if (strcmp(option,STR_INPUTSAMPENC) == 0)
				combo_inenc->setCurrentItem(val);
			else if (strcmp(option,STR_OUTPUTSAMPENC) == 0)
				combo_outenc->setCurrentItem(val);
			else if (strcmp(option,STR_SAMPINTERVAL) == 0)
				edit_sampint->setText(arg);
			else if (strcmp(option,STR_RTPPORTBASE) == 0)
				edit_portbase->setText(arg);
			else if (strcmp(option,STR_INPUTTYPE) == 0)
				combo_intype->setCurrentItem(val);
			else if (strcmp(option,STR_OUTPUTTYPE) == 0)
				combo_outtype->setCurrentItem(val);
			else if (strcmp(option,STR_COMPTYPE) == 0)
				combo_comptype->setCurrentItem(val);
			else if (strcmp(option,STR_DESTIP) == 0)
				edit_destip->setText(arg);
			else if (strcmp(option,STR_DESTPORT) == 0)
				edit_destport->setText(arg);
			else if (strcmp(option,STR_INPUTBOOST) == 0)
				combo_inputboost->setCurrentItem(val);
			else if (strcmp(option,STR_OUTPUTBOOST) == 0)
				combo_outputboost->setCurrentItem(val);
		}
	}
	fclose(f);
	printLog("Settings loaded");
}

void JVOIPTestUtilDlgImpl::saveSettings()
{
	char filename[4096];
	FILE *f;

	getSaveFileName(filename);
	f = fopen(filename,"wt");
	if (f == NULL)
	{
		printLog("Can't write settings!");
		return;
	}

	fprintf(f,"%s %d\n",STR_INPUTSAMPRATE,combo_inrate->currentItem());
	fprintf(f,"%s %d\n",STR_OUTPUTSAMPRATE,combo_outrate->currentItem());
	fprintf(f,"%s %d\n",STR_INPUTSAMPENC,combo_inenc->currentItem());
	fprintf(f,"%s %d\n",STR_OUTPUTSAMPENC,combo_outenc->currentItem());
	fprintf(f,"%s %d\n",STR_SAMPINTERVAL,edit_sampint->text().toInt());
	fprintf(f,"%s %d\n",STR_RTPPORTBASE,edit_portbase->text().toInt());
	fprintf(f,"%s %d\n",STR_INPUTTYPE,combo_intype->currentItem());
	fprintf(f,"%s %d\n",STR_OUTPUTTYPE,combo_outtype->currentItem());
	fprintf(f,"%s %d\n",STR_COMPTYPE,combo_comptype->currentItem());
	fprintf(f,"%s %s\n",STR_DESTIP,(const char *)(edit_destip->text()));
	fprintf(f,"%s %d\n",STR_DESTPORT,edit_destport->text().toInt());
	fprintf(f,"%s %d\n",STR_INPUTBOOST,combo_inputboost->currentItem());
	fprintf(f,"%s %d\n",STR_OUTPUTBOOST,combo_outputboost->currentItem());
	fclose(f);
}

void JVOIPTestUtilDlgImpl::getSaveFileName(char filename[])
{
	int len;
	
	strcpy(filename,getenv("HOME"));
	len = strlen(filename);
	if (len > 0)
	{
		if (filename[len-1] != '/')
			strcat(filename,"/");
	}
	strcat(filename,".jvoiptestutilrc");
}

void JVOIPTestUtilDlgImpl::ThreadFinishedHandler(int threaderr,int voicecallerr,int componenterr)
{
	if (threaderr != 0)
	{
		// At this point, something has gone wrong and the main VoIP
		// thread is about to stop.
		// Note that this function is called from the main VoIP thread,
		// so we're not in the same thread here as in the rest of the
		// dialog box. To handle the error in the dialog box thread,
		// we'll simply post an event to it...
		
		qApp->postEvent(this,new JVOIPThreadStopEvent(threaderr,voicecallerr,componenterr));
	}
}

void JVOIPTestUtilDlgImpl::onCreate()
{
	JVOIPSessionParams sessparams;
	JVOIPRTPTransmissionParams rtpparams;
	JVOIPSoundcardParams sndinparams,sndoutparams;
	int status;
	
	if (JVOIPSession::IsActive())
	{
		printLog("Session already active");
		return;
	}
	
	// obtain the values from the combo boxes and edit fields and store
	// them in the session parameters.
	
	sessparams.SetInputSamplingRate(samprates[combo_inrate->currentItem()]);
	sessparams.SetOutputSamplingRate(samprates[combo_outrate->currentItem()]);
	sessparams.SetInputSampleEncodingType(enctypes[combo_inenc->currentItem()]);
	sessparams.SetOutputSampleEncodingType(enctypes[combo_outenc->currentItem()]);
	sessparams.SetSampleInterval(edit_sampint->text().toInt());
	sessparams.SetVoiceInputType(inputtypes[combo_intype->currentItem()]);
	sessparams.SetVoiceOutputType(outputtypes[combo_outtype->currentItem()]);
	sessparams.SetCompressionType(comptypes[combo_comptype->currentItem()]);

	// set some values for our RTP transmission module

	rtpparams.SetAcceptOwnPackets(true);
	rtpparams.SetPortBase(edit_portbase->text().toInt());
	sessparams.SetTransmissionParams(&rtpparams);

	// set the multiply factors
	
	sndinparams.SetMultiplyFactor(boosts[combo_inputboost->currentItem()]);
	sessparams.SetVoiceInputParams(&sndinparams);
	sndoutparams.SetMultiplyFactor(boosts[combo_outputboost->currentItem()]);
	sessparams.SetVoiceOutputParams(&sndoutparams);

	// try to create the session

	status = JVOIPSession::Create(sessparams);
	if (status < 0)
	{
		printLog(JVOIPGetErrorString(status).c_str());
		return;
	}

	// save the destination IP and port and try to add this destination

	destip = ntohl(inet_addr((const char *)(edit_destip->text())));
	destport = edit_destport->text().toInt();
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

void JVOIPTestUtilDlgImpl::onDestroy()
{
	JVOIPSession::Destroy();
	printLog("Destroyed VoIP session");
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeInputRate(int index)
{
	char line[4096];

	if (!JVOIPSession::IsActive())
		return;
	if (handleError(JVOIPSession::SetInputSamplingRate(samprates[index])))
		return;
	sprintf(line,"Input sampling rate set to %d",samprates[index]);
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeOutputRate(int index)
{
	char line[4096];

	if (!JVOIPSession::IsActive())
		return;
	if (handleError(JVOIPSession::SetOutputSamplingRate(samprates[index])))
		return;
	sprintf(line,"Output sampling rate set to %d",samprates[index]);
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeInputEncoding(int index)
{
	char line[4096];

	if (!JVOIPSession::IsActive())
		return;
	if (handleError(JVOIPSession::SetInputSampleEncodingType(enctypes[index])))
		return;
	sprintf(line,"Input encoding set to %d bits",(enctypes[index] == JVOIPSessionParams::EightBit)?8:16);
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeOutputEncoding(int index)
{
	char line[4096];

	if (!JVOIPSession::IsActive())
		return;
	if (handleError(JVOIPSession::SetOutputSampleEncodingType(enctypes[index])))
		return;
	sprintf(line,"Output encoding set to %d bits",(enctypes[index] == JVOIPSessionParams::EightBit)?8:16);
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeInputType(int index)
{
	char line[4096];
	JVOIPSoundcardParams sndinparams;

	if (!JVOIPSession::IsActive())
		return;

	sndinparams.SetMultiplyFactor(boosts[combo_inputboost->currentItem()]);
	if (handleError(JVOIPSession::SetVoiceInputType(inputtypes[index],&sndinparams)))
		return;
	sprintf(line,"Input encoding set to '%s'",(inputtypes[index] == JVOIPSessionParams::NoInput)?"No input":"Soundcard input");
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeOutputType(int index)
{
	char line[4096];
	JVOIPSoundcardParams sndoutparams;

	if (!JVOIPSession::IsActive())
		return;

	sndoutparams.SetMultiplyFactor(boosts[combo_outputboost->currentItem()]);
	if (handleError(JVOIPSession::SetVoiceOutputType(outputtypes[index],&sndoutparams)))
		return;
	sprintf(line,"Output encoding set to '%s'",(outputtypes[index] == JVOIPSessionParams::NoOutput)?"No output":"Soundcard output");
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeCompressionType(int index)
{
	char line[4096];
	char compname[256];

	if (!JVOIPSession::IsActive())
		return;
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
	case JVOIPSessionParams::SilenceSuppression:
		strcpy(compname,"Silence Suppression");
		break;
	case JVOIPSessionParams::Speex:
		strcpy(compname,"Speex");
		break;
	default:
		strcpy(compname,"???");
	}
	sprintf(line,"Compression type set to %s",compname);
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeSamplingInterval()
{
	char line[4096];

	if (!JVOIPSession::IsActive())
		return;
	if (handleError(JVOIPSession::SetSampleInterval(edit_sampint->text().toInt())))
		return;
	sprintf(line,"Sample interval set to %d ms",edit_sampint->text().toInt());
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangePortBase()
{
	JVOIPRTPTransmissionParams rtpparams;
	char line[4096];

	if (!JVOIPSession::IsActive())
		return;

	// to change the port base, we have to install a new RTP transmission module
	// which is then initialized with the new value.

	rtpparams.SetAcceptOwnPackets(true);
	rtpparams.SetPortBase(edit_portbase->text().toInt());
	if (handleError(JVOIPSession::SetTransmissionType(JVOIPSessionParams::RTP,&rtpparams)))
		return;
	JVOIPSession::ClearDestinations();
	JVOIPSession::AddDestination(destip,destport);
	sprintf(line,"RTP portbase set to %d",edit_portbase->text().toInt());
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeDestinationIP()
{
	char line[1024];

	if (!JVOIPSession::IsActive())
		return;

	destip = ntohl(inet_addr((const char *)(edit_destip->text())));
	JVOIPSession::ClearDestinations();
	JVOIPSession::AddDestination(destip,destport);
	sprintf(line,"Destination address is now %d.%d.%d.%d:%d",
		(int)((destip>>24)&0xFF),(int)((destip>>16)&0xFF),
		(int)((destip>>8)&0xFF),(int)((destip)&0xFF),destport);
	printLog(line);
}

void JVOIPTestUtilDlgImpl::onChangeDestinationPort()
{
	char line[1024];

	if (!JVOIPSession::IsActive())
		return;
		
	destport = edit_destport->text().toInt();
	JVOIPSession::ClearDestinations();
	JVOIPSession::AddDestination(destip,destport);
	sprintf(line,"Destination address is now %d.%d.%d.%d:%d",
		(int)((destip>>24)&0xFF),(int)((destip>>16)&0xFF),
		(int)((destip>>8)&0xFF),(int)((destip)&0xFF),destport);
	printLog(line);
}

void JVOIPTestUtilDlgImpl::onChangeInputBoost(int index)
{
	char line[4096];
	JVOIPSoundcardParams sndinparams;

	if (!JVOIPSession::IsActive())
		return;

	sndinparams.SetMultiplyFactor(boosts[index]);
	
	if (handleError(JVOIPSession::SetVoiceInputType(inputtypes[combo_intype->currentItem()],&sndinparams)))
		return;
	sprintf(line,"Input boost set to %d",boosts[index]);
	printLog(line);
	updateComponentInfo();
}

void JVOIPTestUtilDlgImpl::onChangeOutputBoost(int index)
{
	char line[4096];
	JVOIPSoundcardParams sndoutparams;

	if (!JVOIPSession::IsActive())
		return;

	sndoutparams.SetMultiplyFactor(boosts[index]);
	
	if (handleError(JVOIPSession::SetVoiceOutputType(outputtypes[combo_outtype->currentItem()],&sndoutparams)))
		return;
	sprintf(line,"Output boost set to %d",boosts[index]);
	printLog(line);
	updateComponentInfo();
}

bool JVOIPTestUtilDlgImpl::handleError(int err)
{
	if (err >= 0)
	{
		updateComponentInfo();
		return false;
	}
	printLog(JVOIPGetErrorString(err).c_str());
	return true;
}

void JVOIPTestUtilDlgImpl::updateComponentInfo()
{
	list_compinfo->clear();

	if (!JVOIPSession::IsActive())
		return;

	addComponentInfo(JVOIPSession::VoiceInput);	
	addComponentInfo(JVOIPSession::VoiceOutput);	
	addComponentInfo(JVOIPSession::Compression);	
	addComponentInfo(JVOIPSession::Mixer);	
	addComponentInfo(JVOIPSession::Transmission);	
}

void JVOIPTestUtilDlgImpl::addComponentInfo(JVOIPSession::ComponentType comptype)
{
	QListViewItem *compitem;
	std::vector<JVOIPCompParamInfo> *params = NULL;
	std::vector<JVOIPCompParamInfo>::const_iterator it;
	
	compitem = new QListViewItem(list_compinfo,QString(JVOIPSession::GetComponentName(comptype).c_str()),QString(JVOIPSession::GetComponentDescription(comptype).c_str()));
	JVOIPSession::GetComponentParameters(comptype,&params);
	if (params == NULL)
		return;
	for (it = params->begin() ; it != params->end() ; ++it)
	{
		QListViewItem *compparam;

		compparam = new QListViewItem(compitem,QString((*it).GetParameterName().c_str()),QString((*it).GetParameterValue().c_str()));
	}
	delete params;
	compitem->setOpen(true);
}	

bool JVOIPTestUtilDlgImpl::event(QEvent *evt)
{
	if (evt->type() == QEvent::User)
	{
		JVOIPThreadStopEvent *tsevt;

		tsevt = dynamic_cast<JVOIPThreadStopEvent *>(evt);
		if (tsevt != NULL)
		{
			char line[4096];
		
			printLog("VoIP thread exited unexpectedly:");
			sprintf(line,"        VoIP Thread error: %s",JVOIPGetErrorString(tsevt->threadError()).c_str());
			printLog(line);
			sprintf(line,"        VoiceCall error: %s",VoIPFramework::GetVoiceCallErrorString(tsevt->voicecallError()));
			printLog(line);
			
			// NOTE: because we are certain that we didn't use any
			//       user defined components, we can use the JVOIPGetErrorString
			//       function to interpret the component error.
			//       If we did use user defined components, the returned
			//       error could also be user defined and the 
			//       JVOIPGetErrorString function would return incorrect
			//       information...
			sprintf(line,"        Component error: %s",JVOIPGetErrorString(tsevt->componentError()).c_str());
			printLog(line);
			updateComponentInfo();	
			return true;
		}
	}
	return JVOIPTestUtilDlg::event(evt);
}
