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

#ifndef JVOIPFILEOUTPUT_H

#define JVOIPFILEOUTPUT_H

#include "jvoipconfig.h"

#ifndef JVOIPDISABLE_IO_FILE

#include "jvoipvoiceoutput.h"
#include "jvoipcomponentparams.h"
#include "jvoipsampleconverter.h"
#include <string>
#include <sndfile.h>

class JVOIPFileOutputParams : public JVOIPComponentParams
{
public:
	enum EncodingType { SixteenBit, EightBit, MuLaw, Other };
	
	JVOIPFileOutputParams(const std::string &filename,int samplerate = 8000,EncodingType t = MuLaw,bool stereo = false) 
										{fname = filename; samprate = samplerate; enctype = t; isstereo = stereo; }
	~JVOIPFileOutputParams()						{ }
	
	void SetFileName(const std::string &filename)				{ fname = filename; }
	std::string GetFileName() const						{ return fname; }
	void SetSampleRate(int samplerate)					{ samprate = samplerate; }
	int GetSampleRate() const						{ return samprate; }
	void SetStereo(bool stereo)						{ isstereo = stereo; }
	bool IsStereo() const							{ return isstereo; }
	void SetEncodingType(EncodingType t)					{ enctype = t; }
	EncodingType GetEncodingType() const					{ return enctype; }

	JVOIPComponentParams *CreateCopy() const				{ return new JVOIPFileOutputParams(fname,samprate,enctype,isstereo); }
private:
	std::string fname;
	int samprate;
	EncodingType enctype;
	bool isstereo;
};

class JVOIPFileOutput : public JVOIPVoiceOutput
{
public:
	JVOIPFileOutput(JVOIPSession *sess);
	~JVOIPFileOutput();
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
	std::vector<JVOIPCompParamInfo> *GetComponentParameters() throw (JVOIPException);
private:
	int InitBuffer();
	void ClearBuffer();
	void InitSampleConverter();
	
	SNDFILE *sndfile;
	std::string fname;
	JVOIPFileOutputParams::EncodingType enctype;
	JVOIPSampleConverter sampconvert;
	bool init,firsttime;

	unsigned char *buffer;
	float *fbuffer;
	int blocksize;
	
	int filesamprate,filebytespersample,filestereo;
	int sampleinterval,samprate,bytespersample;
	bool needstereo;
	
	double wantedplaytime,playtime;
};

#endif // JVOIPDISABLE_IO_FILE

#endif // JVOIPFILEOUTPUT_H

