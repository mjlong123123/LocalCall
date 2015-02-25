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

#include "jvoipfileoutput.h"
#include "jvoipsampleconverter.h"
#include "jvoiperrors.h"
#include "voiceblock.h"

#include "debugnew.h"

JVOIPFileOutput::JVOIPFileOutput(JVOIPSession *sess):JVOIPVoiceOutput(sess)
{
	init = false;
	sndfile = 0;
}

JVOIPFileOutput::~JVOIPFileOutput()
{
	Cleanup();
}

int JVOIPFileOutput::Init(int sampinterval,int outputsamprate,int outputbytespersample,bool stereo,const JVOIPComponentParams *componentparams)
{
	JVOIPFileOutputParams *usrparams;
	int status;

	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;
	
	if (componentparams == NULL)
		return ERR_JVOIPLIB_FILEIO_NOPARAMS;
	else if ((usrparams = dynamic_cast<JVOIPFileOutputParams *>(const_cast<JVOIPComponentParams *>(componentparams))) == NULL)
		return ERR_JVOIPLIB_FILEIO_NOPARAMS;
	
	fname = usrparams->GetFileName();
	filesamprate = usrparams->GetSampleRate();
	filestereo = (usrparams->IsStereo())?2:1;
	enctype = usrparams->GetEncodingType();
	
	SF_INFO sfinfo;

	sfinfo.frames = 0;
	sfinfo.samplerate = filesamprate;
	sfinfo.channels = filestereo;
	sfinfo.format = SF_FORMAT_WAV;
	sfinfo.sections = 0;
	sfinfo.seekable = 0;

	if (enctype == JVOIPFileOutputParams::MuLaw)
		sfinfo.format |= SF_FORMAT_ULAW;
	else if (enctype == JVOIPFileOutputParams::EightBit)
		sfinfo.format |= SF_FORMAT_PCM_S8;
	else if (enctype == JVOIPFileOutputParams::SixteenBit)
		sfinfo.format |= SF_FORMAT_PCM_16;
	else
		return ERR_JVOIPLIB_FILEIO_ILLEGALFORMAT;
	
	sndfile = sf_open(fname.c_str(),SFM_RDWR,&sfinfo);
	if (sndfile == 0)
		return ERR_JVOIPLIB_FILEIO_CANTOPENFILE;

	filesamprate = sfinfo.samplerate;
	if (sfinfo.channels != 1 && sfinfo.channels != 2)
	{
		sf_close(sndfile);
		return ERR_JVOIPLIB_FILEIO_CHANNELSUNSUPPORTED;
	}
	
	if ((sfinfo.format&SF_FORMAT_SUBMASK) == SF_FORMAT_ULAW)
		enctype = JVOIPFileOutputParams::MuLaw;
	else if ((sfinfo.format&SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_S8)
		enctype = JVOIPFileOutputParams::EightBit;
	else if ((sfinfo.format&SF_FORMAT_SUBMASK) == SF_FORMAT_PCM_16)
		enctype = JVOIPFileOutputParams::SixteenBit;
	else
		enctype = JVOIPFileOutputParams::Other;
	
	if (sf_seek(sndfile,0,SEEK_END) < 0)
	{
		sf_close(sndfile);
		return ERR_JVOIPLIB_FILEIO_CANTSEEKFILE;
	}
	
	sampleinterval = sampinterval;
	samprate = outputsamprate;
	bytespersample = outputbytespersample;
	needstereo = stereo;
	
	if ((status = InitBuffer()) < 0)
	{
		sf_close(sndfile);
		return status;
	}

	InitSampleConverter();		
	init = true;
	firsttime = true;			

	return 0;
}

int JVOIPFileOutput::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	sf_close(sndfile);
	ClearBuffer();
	init = false;

	return 0;
}

int JVOIPFileOutput::Play(VoIPFramework::VoiceBlock *vb)
{
	bool playsilence;
	unsigned char *data;
	double diff;
	int numsamp,size;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	playsilence = false;

	if ((data = vb->GetSamples(false)) == NULL)
		playsilence = true;
	if (vb->IsStereo() != needstereo)
		playsilence = true;
	else if (vb->GetBytesPerSample() != bytespersample)
		playsilence = true;
	if ((int)vb->GetSampleRate() != samprate)
		playsilence = true;
			
	if (firsttime)
	{
		firsttime = false;
		wantedplaytime = 0;
		playtime = 0;
	}
	
	wantedplaytime += (double)sampleinterval;
	diff = wantedplaytime-playtime;
	diff /= 1000.0; // convert to seconds
	numsamp = (int)(diff*(double)filesamprate+0.5);
	size = numsamp*filestereo*2;
	playtime += ((double)numsamp)/((double)filesamprate)*1000.0;

	if (playsilence)
	{
		for (int i = 0 ; i < size ; i += 2)
		{
			buffer[i] = 127;
			buffer[i+1] = 255;
		}
	}
	else
	{
		int actualsize;

		actualsize = sampconvert.Convert(data,vb->GetNumBytes(),buffer,size);
		if (actualsize < size)
		{
			if (filestereo == 1)
			{
				unsigned char b1,b2;

				b1 = buffer[actualsize-2];
				b2 = buffer[actualsize-1];
				for (     ; actualsize < size ; actualsize += 2)
				{
					buffer[actualsize] = b1;
					buffer[actualsize+1] = b2;
				}
			}
			else // stereo
			{
				unsigned char b1,b2,b3,b4;

				b1 = buffer[actualsize-4];
				b2 = buffer[actualsize-3];
				b3 = buffer[actualsize-2];
				b4 = buffer[actualsize-1];
				for (     ; actualsize < size ; actualsize += 4)
				{
					buffer[actualsize] = b1;
					buffer[actualsize+1] = b2;
					buffer[actualsize+2] = b3;
					buffer[actualsize+3] = b4;
				}
			}
		}
	}

	int i,fsize;
	
	for (i = 0,fsize = 0 ; i < size ; i += 2)
	{
		fbuffer[fsize] = ((((float)buffer[i])*256.0+((float)buffer[i+1]))/(65536.0/2.0))-1.0;
		fsize++;
	}

	if (sf_write_float(sndfile,fbuffer,fsize) != fsize)
	{
		sf_close(sndfile);
		init = false;
		return ERR_JVOIPLIB_FILEIO_CANTWRITE;
	}
	
	return 0;
}

void JVOIPFileOutput::Reset()
{
	int status;

	if (!init)
		return;
	
	firsttime = true;
	ClearBuffer();
	if ((status = InitBuffer()) < 0)
	{
		sf_close(sndfile);
		init = false;
		std::cerr << "JVOIPFileOutput: couldn't init buffers during Reset, device closed" << std::endl;
 		return;
	}
	InitSampleConverter();	
}

bool JVOIPFileOutput::SupportsSampleInterval(int ival)
{
	return true;
}

bool JVOIPFileOutput::SupportsOutputSamplingRate(int orate)
{
	return true;
}

bool JVOIPFileOutput::SupportsOutputBytesPerSample(int outputbytespersample)
{
	return true;
}

int JVOIPFileOutput::SetSampleInterval(int ival)
{
	int status;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (ival == sampleinterval)
		return 0;

	sampleinterval = ival;
	firsttime = true;
	ClearBuffer();
	if ((status = InitBuffer()) < 0)
	{
		sf_close(sndfile);
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPFileOutput::SetOutputSamplingRate(int orate)
{
	int status;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (orate == samprate)
		return 0;
	
	samprate = orate;
	firsttime = true;
	ClearBuffer();
	if ((status = InitBuffer()) < 0)
	{
		sf_close(sndfile);
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPFileOutput::SetOutputBytesPerSample(int outputbytespersample)
{
	int status;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (outputbytespersample == bytespersample)
		return 0;
	
	bytespersample = outputbytespersample;
	firsttime = true;
	ClearBuffer();
	if ((status = InitBuffer()) < 0)
	{
		sf_close(sndfile);
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPFileOutput::SetStereo(bool s)
{
	int status;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (s == needstereo)
		return 0;
	
	needstereo = s;
	firsttime = true;
	ClearBuffer();
	if ((status = InitBuffer()) < 0)
	{
		sf_close(sndfile);
		init = false;
		return status;
	}
	InitSampleConverter();	
	return 0;
}

int JVOIPFileOutput::GetComponentState(JVOIPComponentState **compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	*compstate = NULL;
	return 0;
}

int JVOIPFileOutput::SetComponentState(JVOIPComponentState *compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

std::string JVOIPFileOutput::GetComponentName()
{
	return std::string("JVOIPFileOutput");
}

std::string JVOIPFileOutput::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal file output module (using libsndfile)");
}

std::vector<JVOIPCompParamInfo> *JVOIPFileOutput::GetComponentParameters() throw (JVOIPException)
{
	std::vector<JVOIPCompParamInfo> *paraminfo;
	char str[256];
	
	paraminfo = new std::vector<JVOIPCompParamInfo>(4);
	if (paraminfo == NULL)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
	(*paraminfo)[0].SetParameterName(std::string("File name"));
	(*paraminfo)[0].SetParameterValue(fname);
	
	sprintf(str,"%d",filesamprate);
	(*paraminfo)[1].SetParameterName(std::string("Sample rate"));
	(*paraminfo)[1].SetParameterValue(std::string(str));

	(*paraminfo)[2].SetParameterName(std::string("Stereo"));
	if (filestereo == 1)
		(*paraminfo)[2].SetParameterValue(std::string("No"));
	else
		(*paraminfo)[2].SetParameterValue(std::string("Yes"));

	(*paraminfo)[3].SetParameterName(std::string("Encoding"));
	if (enctype == JVOIPFileOutputParams::MuLaw)
		(*paraminfo)[3].SetParameterValue(std::string("Mu-law"));
	else if (enctype == JVOIPFileOutputParams::EightBit)
		(*paraminfo)[3].SetParameterValue(std::string("Signed eight bit"));
	else if (enctype == JVOIPFileOutputParams::SixteenBit)
		(*paraminfo)[3].SetParameterValue(std::string("Signed sixteen bit"));
	else
		(*paraminfo)[3].SetParameterValue(std::string("Other"));

	return paraminfo;
}

void JVOIPFileOutput::InitSampleConverter()
{
	int srcrate,srcbytespersample,dstrate,dstbytespersample;
	bool srcstereo,srcsigned,srcLE,dststereo,dstsigned,dstLE;

        dstrate = filesamprate;
        dstbytespersample = 2;
        dststereo = (filestereo == 2)?true:false;
        dstsigned = false;
        dstLE = false;

        srcrate = samprate;
        srcbytespersample = bytespersample;
        srcstereo = needstereo;
        srcsigned = false;
        srcLE = false;
        	
	sampconvert.SetConversionParams(srcrate,srcstereo,srcbytespersample,srcsigned,srcLE,
                                        dstrate,dststereo,dstbytespersample,dstsigned,dstLE);
}

#define JVOIPFILEOUTPUT_EXTRABLOCKSIZE	4096

int JVOIPFileOutput::InitBuffer()
{
	blocksize = (int)((((double)filesamprate*(double)sampleinterval)/1000.0)*2.0*(double)filestereo+0.5);
	blocksize += JVOIPFILEOUTPUT_EXTRABLOCKSIZE;
	
	buffer = new unsigned char [blocksize];
	fbuffer = new float [blocksize/filestereo];

	return 0;
}

void JVOIPFileOutput::ClearBuffer()
{
	delete [] buffer;
	delete [] fbuffer;
}

#endif // JVOIPDISABLE_IO_FILE

