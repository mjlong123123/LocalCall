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

#ifndef JVOIPSOUNDCARDOUTPUT_H

#define JVOIPSOUNDCARDOUTPUT_H

#include "jvoipconfig.h"
#include "jvoipvoiceoutput.h"
#include "jvoipsampleconverter.h"
#include "voiceblock.h"
#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>

class JVOIPSoundcardOutput : public JVOIPVoiceOutput
{
public:
	JVOIPSoundcardOutput(JVOIPSession *sess);
	~JVOIPSoundcardOutput();
	int Init(int sampinterval,int outputsamprate,int outputbytespersample,bool stereo,const JVOIPComponentParams *componentparams);
	int Cleanup();
	int Play(VoIPFramework::VoiceBlock *vb);
	void Reset();
	bool SupportsSampleInterval(int ival);
	int SetSampleInterval(int ival);
	bool SupportsOutputSamplingRate(int orate);
	int SetOutputSamplingRate(int orate);
	bool SupportsOutputBytesPerSample(int outputbytespersample);
	int SetOutputBytesPerSample(int outputbytespersample);
	int SetStereo(bool s);
	
	int GetComponentState(JVOIPComponentState **compstate);
	int SetComponentState(JVOIPComponentState *compstate);
	
	std::string GetComponentName();
	std::string GetComponentDescription();
	std::vector<JVOIPCompParamInfo> *GetComponentParameters();
private:
	int InitBuffers();
	void ClearBuffers();
	void InitSampleConverter();
	int OpenSoundDevice(int samplerate);
	int CalculateDriveSampleRate(int requestedrate);
	int RequestSamplingRate(int rate);

	HWAVEOUT sounddev;
	JVOIPSampleConverter sampconvert;
	bool init,firsttime,cycled;

	WAVEHDR *sampleblocks;
	int numblocks,blocksize;
	int blockpos;
	
	int drvsamprate,drvbytespersample,drvstereo;
	int sampleinterval,samprate,bytespersample;
	bool needstereo;
	
	double wantedplaytime,playtime;
};

#endif // JVOIPSOUNDCARDOUTPUT_H
