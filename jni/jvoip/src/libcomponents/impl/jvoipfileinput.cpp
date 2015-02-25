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

#include "jvoipconfig.h"

#ifndef JVOIPDISABLE_IO_FILE

#include "jvoipfileinput.h"
#include "jvoiperrors.h"
#include "voiceblock.h"
#include <iostream>

#include "debugnew.h"

JVOIPFileInput::JVOIPFileInput(JVOIPSession *sess) throw (JVOIPException) : JVOIPVoiceInput(sess)
{
	init = false;
	sndfile = 0;
}

JVOIPFileInput::~JVOIPFileInput()
{
	Cleanup();
}

int JVOIPFileInput::Init(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	JVOIPFileInputParams *usrparams;
	int status;

	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;

	if (componentparams == NULL)
		return ERR_JVOIPLIB_FILEIO_NOPARAMS;
	else if ((usrparams = dynamic_cast<JVOIPFileInputParams *>(const_cast<JVOIPComponentParams *>(componentparams))) == NULL)
		return ERR_JVOIPLIB_FILEIO_NOPARAMS;

	fname = usrparams->GetFileName();

	SF_INFO sfinfo;

	sfinfo.frames = 0;
	sfinfo.samplerate = 0;
	sfinfo.channels = 0;
	sfinfo.format = 0;
	sfinfo.sections = 0;
	sfinfo.seekable = 0;
	
	sndfile = sf_open(fname.c_str(),SFM_READ,&sfinfo);
	if (sndfile == 0)
		return ERR_JVOIPLIB_FILEIO_CANTOPENFILE;

	filesamprate = sfinfo.samplerate;
	filestereo = sfinfo.channels;
	if (filestereo != 1 && filestereo != 2)
	{
		sf_close(sndfile);
		return ERR_JVOIPLIB_FILEIO_CHANNELSUNSUPPORTED;
	}
	
	sampleinterval = sampinterval;
	samprate = inputsamprate;
	bytespersample = inputbytespersample;
	
	InitSampleConverter();		
	
	if ((status = thetimer.SetSamplingInterval(sampinterval)) < 0)
	{
		sf_close(sndfile);
		return status;
	}

	init = true;
	firsttime = true;
	return 0;
}

int JVOIPFileInput::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	thetimer.StopTimer();
	init = false;
	sf_close(sndfile);
	return 0;
}

int JVOIPFileInput::StartSampling()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	// NOTE: nothing to do here...
	return 0;
}

void JVOIPFileInput::Reset()
{
	if (!init)
		return;
	thetimer.StopTimer();
	firsttime = true;
}

int JVOIPFileInput::GetSampleBlock(VoIPFramework::VoiceBlock *vb)
{
	unsigned char *buffer,*buffer2;
	float *fbuffer;
	double diff;
	int numsamp,size,count;
	int fnumsamp,fsize;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	if (firsttime)
	{
		firsttime = false;
		dstsampletime = 0;
		wanteddstsampletime = 0;
	}		

	wanteddstsampletime += (double)sampleinterval;
	diff = wanteddstsampletime-dstsampletime;
	diff /= 1000.0; // convert to seconds
	numsamp = (int)(diff*(double)samprate+0.5);
	fnumsamp = (int)(diff*(double)filesamprate+0.5);
	size = numsamp*bytespersample;
	fsize= fnumsamp*filestereo;
	dstsampletime += ((double)numsamp)/((double)samprate)*1000.0;
	
	buffer = new unsigned char[size];
	if (buffer == 0)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	buffer2 = new unsigned char[fsize*2]; // using 2 bytes per sample
	if (buffer2 == 0)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	fbuffer = new float[fsize];
	if (fbuffer == 0)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;

	count = sf_read_float(sndfile,fbuffer,fsize);
	if (count < 0)
		count = 0;
	for ( ;  count < fsize ; count++)
		fbuffer[count] = 0;

	int i,j;

	for (i = 0,j = 0 ; i < fsize ; i++,j += 2)
	{
		int val = (int)((((fbuffer[i]+1.0)/2.0)*65536.0)+0.5);

		if (val > 65535) val = 65535;
		else if (val < 0) val = 0;

		buffer2[j] = (unsigned char)((val>>8)&255);
		buffer2[j+1] = (unsigned char)(val&255);
	}

	int actualsize = sampconvert.Convert(buffer2,fsize*2,buffer,size);
	if (actualsize < size) // we'll have to fill a bit
	{
		if (bytespersample == 1)
		{
			unsigned char b;
				
			b = buffer[actualsize-1];
			for (    ;  actualsize < size ; actualsize++)
				buffer[actualsize] = b;
		}
		else // two bytes per sample
		{
			unsigned char b1,b2;
				
			b1 = buffer[actualsize-2];
			b2 = buffer[actualsize-1];
			for (    ;  actualsize < size ; actualsize += 2)
			{
				buffer[actualsize] = b1;
				buffer[actualsize + 1] = b2;
			}
		}
	}
	
	vb->SetSamples(buffer,size);
	vb->SetNumSamples(numsamp);
	vb->SetStereo(false);
	vb->SetBytesPerSample(bytespersample);
	vb->SetSampleRate(samprate);

	delete [] fbuffer;
	delete [] buffer2;
	
	return 0;
}

bool JVOIPFileInput::SupportsSampleInterval(int ival)
{
	if (!init)
		return false;
	return true;
}

bool JVOIPFileInput::SupportsInputSamplingRate(int irate)
{
	if (!init)
		return false;
	return true;
}

bool JVOIPFileInput::SupportsInputBytesPerSample(int inputbytespersample)
{
	if (!init)
		return false;
	return true;
}

int JVOIPFileInput::SetSampleInterval(int ival)
{
	int status;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	Reset();
	sampleinterval = ival;
	if ((status = thetimer.SetSamplingInterval(ival)) < 0)
	{
		sf_close(sndfile);
		init = false;
		return status;
	}
	InitSampleConverter();
	return 0;
}

int JVOIPFileInput::SetInputSamplingRate(int irate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (irate == samprate)
		return 0;
	
	Reset();
	samprate = irate;
	InitSampleConverter();	
	return 0;
}

int JVOIPFileInput::SetInputBytesPerSample(int inputbytespersample)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (inputbytespersample == bytespersample)
		return 0;

	Reset();
	bytespersample = inputbytespersample;
	InitSampleConverter();	
	return 0;
}

int JVOIPFileInput::GetComponentState(JVOIPComponentState **compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	sf_count_t pos = sf_seek(sndfile,0,SEEK_CUR);
	if (pos < 0)
		return ERR_JVOIPLIB_FILEIO_CANTSEEKFILE;
	
	JVOIPFileInputState *state = new JVOIPFileInputState(pos);
	if (state == 0)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	*compstate = state;

	return 0;
}

int JVOIPFileInput::SetComponentState(JVOIPComponentState *compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	JVOIPFileInputState *state;
	
	if (compstate == 0)
		return ERR_JVOIPLIB_GENERAL_ILLEGALSTATEPARAMETER;
	else if ((state = dynamic_cast<JVOIPFileInputState *>(compstate)) == 0)
		return ERR_JVOIPLIB_GENERAL_ILLEGALSTATEPARAMETER;

	if (sf_seek(sndfile,state->GetFramePosition(),SEEK_SET) < 0)
		return ERR_JVOIPLIB_FILEIO_CANTSEEKFILE;

	return 0;
}

std::string JVOIPFileInput::GetComponentName()
{
	return std::string("JVOIPFileInput");
}

std::string JVOIPFileInput::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal file input module (using libsndfile)");
}

std::vector<JVOIPCompParamInfo> *JVOIPFileInput::GetComponentParameters() throw (JVOIPException)
{
	std::vector<JVOIPCompParamInfo> *paraminfo;
	
	paraminfo = new std::vector<JVOIPCompParamInfo>(1);
	if (paraminfo == NULL)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
	(*paraminfo)[0].SetParameterName(std::string("File name"));
	(*paraminfo)[0].SetParameterValue(fname);

	return paraminfo;
}

void JVOIPFileInput::InitSampleConverter()
{
	int srcrate,srcbytespersample,dstrate,dstbytespersample;
	bool srcstereo,srcsigned,srcLE,dststereo,dstsigned,dstLE;

        srcrate = filesamprate;
        srcbytespersample = 2;
        srcstereo = (filestereo == 2)?true:false;
        srcsigned = false;
        srcLE = false;

        dstrate = samprate;
        dstbytespersample = bytespersample;
        dststereo = false;
        dstsigned = false;
        dstLE = false;
        	
	sampconvert.SetConversionParams(srcrate,srcstereo,srcbytespersample,srcsigned,srcLE,
                                        dstrate,dststereo,dstbytespersample,dstsigned,dstLE);
}

#endif // JVOIPDISABLE_IO_FILE

