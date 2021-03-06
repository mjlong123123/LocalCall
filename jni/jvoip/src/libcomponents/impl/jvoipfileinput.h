/*

    This file is a part of JVOIPLIB, a library designed to facilitate
    the use of Voice over IP (VoIP).

    Copyright (C) 2000-2005  Jori Liesenborgs (jori@lumumba.uhasselt.be)

    This library (JVOIPLIB) is based upon work done for my thesis at
    the School for Knowledge Technology (Belgium/The Netherlands)

    This file was developed at the 'Expertise Centre for Digital
    Media' (EDM) in Diepenbeek, Belgium (http://www.edm.uhasselt.be).
    The EDM is a research institute of the Hasselt University
    (http://www.uhasselt.be).

    The full GNU Library General Public License can be found in the
    file LICENSE.LGPL which is included in the source code archive.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA

*/

#ifndef JVOIPFILEINPUT_H

#define JVOIPFILEINPUT_H

#include "jvoipconfig.h"

#ifndef JVOIPDISABLE_IO_FILE

#include "jvoipvoiceinput.h"
#include "jvoipsimpletimer.h"
#include "jvoipcomponentparams.h"
#include "jvoipsampleconverter.h"
#include <sndfile.h>

class JVOIPFileInputParams : public JVOIPComponentParams
{
public:
	JVOIPFileInputParams(const std::string &filename) 			{ fname = filename; }
	~JVOIPFileInputParams()							{ }
	std::string GetFileName() const						{ return fname; }
	JVOIPComponentParams *CreateCopy() const				{ return new JVOIPFileInputParams(fname); }
private:
	std::string fname;
};

class JVOIPFileInputState : public JVOIPComponentState
{
public:
	JVOIPFileInputState(sf_count_t framepos)				{ fpos = framepos; }
	~JVOIPFileInputState()							{ }
	sf_count_t GetFramePosition() const					{ return fpos; }
private:
	sf_count_t fpos;
};

class JVOIPFileInput : public  JVOIPVoiceInput
{
public:
	JVOIPFileInput(JVOIPSession *sess) throw (JVOIPException);
	~JVOIPFileInput();
	int Init(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams);
	int Cleanup();
	int StartSampling();
	void Reset();
	int GetSampleBlock(VoIPFramework::VoiceBlock *vb);
	JVOIPSamplingTimer *GetSamplingTimer() 						{ return &thetimer; }
	bool SupportsSampleInterval(int ival);
	bool SupportsInputSamplingRate(int irate);
	bool SupportsInputBytesPerSample(int inputbytespersample);
	int SetSampleInterval(int ival);
	int SetInputSamplingRate(int irate);
	int SetInputBytesPerSample(int inputbytespersample);

	int GetComponentState(JVOIPComponentState **compstate);
	int SetComponentState(JVOIPComponentState *compstate);
	
	std::string GetComponentName();
	std::string GetComponentDescription();
	std::vector<JVOIPCompParamInfo> *GetComponentParameters() throw (JVOIPException);
private:
	void InitSampleConverter();

	SNDFILE *sndfile;
	std::string fname;
	bool init,firsttime;
	JVOIPSimpleTimer thetimer;

	JVOIPSampleConverter sampconvert;
	int filesamprate,filestereo;
	int sampleinterval,bytespersample,samprate;
	
	double dstsampletime,wanteddstsampletime;
};

#endif // JVOIPDISABLE_IO_FILE

#endif // JVOIPFILEINPUT_H

