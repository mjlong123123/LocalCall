/*

    This file is a part of JVOIPLIB, a library designed to facilitate
    the use of Voice over IP (VoIP).

    Copyright (C) 2000-2005  Jori Liesenborgs (jori@lumumba.uhasselt.be)

    This library (JVOIPLIB) is based upon work done for my thesis at
    the School for Knowledge Technology (Belgium/The Netherlands)

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

#ifndef JVOIPDISABLE_COMPRESSION_SPEEX

#include "jvoipspeexcompression.h"
#include "jvoiperrors.h"
#include <stdio.h>
#include <string.h>
#include <list>
#ifdef WIN32
	#include <winsock2.h>
#else
	#include <sys/time.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif // WIN32

#include "debugnew.h"

#define JVOIPSPEEXCOMPRESSION_SAMPLEINTERVAL			20
#define JVOIPSPEEXCOMPRESSION_SAMPLERATE			8000
#define JVOIPSPEEXCOMPRESSION_EIGHTBITSCALE			256
#define JVOIPSPEEXCOMPRESSION_CHECKDELAY			(60*5)
#define JVOIPSPEEXCOMPRESSION_TIMEOUTDELAY			(60*9)

JVOIPSpeexCompression::JVOIPSpeexCompression(JVOIPSession *sess) : JVOIPCompressionModule(sess)
{
	compinit = false;
	stopthreadmutex.Init();
	statemutex.Init();
}

JVOIPSpeexCompression::~JVOIPSpeexCompression()
{
	CleanupCompressor();
	CleanupDecompressor();
}

int JVOIPSpeexCompression::InitCompressor(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	if (compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;

	if ((sampinterval <= 0) || (sampinterval != JVOIPSPEEXCOMPRESSION_SAMPLEINTERVAL))
		return ERR_JVOIPLIB_SPEEXCOMP_UNSUPPORTEDSAMPLEINTERVAL;

	if (inputsamprate != JVOIPSPEEXCOMPRESSION_SAMPLERATE)
		return ERR_JVOIPLIB_SPEEXCOMP_UNSUPPORTEDSAMPLERATE;

	speex_bits_init(&ownbits);
	ownstate = speex_encoder_init(&speex_nb_mode);

	int val = 2;
	speex_encoder_ctl(ownstate,SPEEX_SET_QUALITY,&val);

	compinit = true;
	return 0;
}

int JVOIPSpeexCompression::CleanupCompressor()
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	speex_bits_destroy(&ownbits);
	speex_encoder_destroy(ownstate);
	compinit = false;
	return 0;
}

int JVOIPSpeexCompression::InitDecompressor()
{
	stopthread = false;
	if (Start() < 0) // Start the thread which cleans the Speex state info map from time to time
		return ERR_JVOIPLIB_SPEEXCOMP_CANTSTARTTHREAD;
	return 0;
}

int JVOIPSpeexCompression::CleanupDecompressor()
{
	// We have to clean up the Speex states

	std::map<VoIPFramework::VOIPuint64,SpeexStateInfo *>::const_iterator it;

	StopThread();

	for (it = speexstates.begin() ; it != speexstates.end() ; ++it)
		delete it->second; // delete the SpeexStateInfo part;
	speexstates.clear();
	return 0;
}

int JVOIPSpeexCompression::Compress(VoIPFramework::VoiceBlock *vb)
{
	unsigned char *data;
	int datalen,numsamples,i,j;
	int bytespersample;
	float input[160];
	
	data = vb->GetSamples(false);
	datalen = vb->GetNumBytes();
	numsamples = vb->GetNumSamples();
	if (numsamples != 160)
	{
		// TODO: for debugging!
		std::cerr << "JVOIPSpeexCompression: not 160 samples!" << std::endl;
		vb->Clear();
		return 0;
	}
	bytespersample = vb->GetBytesPerSample();

	if (bytespersample == 1)
	{
		for (i = 0 ; i < 160 ; i++)
			input[i] = (((float)data[i])-128.0)*256.0;
	}
	else
	{
		for (i = 0,j = 0 ; i < 160 ; i++,j += 2)
			input[i] = ((float)(((((unsigned short)data[j])<<8)|((unsigned short)data[j+1]))))-32768.0;
	}

	speex_bits_reset(&ownbits);
	speex_encode(ownstate,input,&ownbits);
	int framesize = speex_bits_nbytes(&ownbits);
	unsigned char *frame = new unsigned char[framesize];
	speex_bits_write(&ownbits,(char *)frame,framesize);
	
	vb->SetSamples(frame,framesize);

	return 0;
}

int JVOIPSpeexCompression::Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid)
{
	unsigned char *frame;
	unsigned char *speech;
	int framelen,i,j;
	SpeexStateInfo *stateinf;

	frame = vb->GetSamples(false);
	if (frame == NULL)
		return 0;
	framelen = vb->GetNumBytes();

	// Obtain the Speex state info for this participant. This is done by the
	// map. 
	
	statemutex.Lock();
	
	stateinf = speexstates[sourceid];
	if (stateinf == NULL)
	{
		stateinf = new SpeexStateInfo();
		if (stateinf == NULL)
		{
			statemutex.Unlock();
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		}
		speexstates[sourceid] = stateinf;
	}
	else	
		stateinf->lasttime = time(NULL);

	float output[160];
	
	speex_bits_read_from(&(stateinf->bits),(char *)frame,framelen);
	speex_decode(stateinf->state,&(stateinf->bits),output);
	
	speech = new unsigned char[160*2];

	for (i = 0,j = 0 ; i < 160 ; i++,j += 2)
	{
		unsigned short val = (unsigned short)(output[i]+32768.0);
		speech[j] = (unsigned char)((val>>8)&0xFF);
		speech[j+1] = (unsigned char)(val&0xFF);
	}

	statemutex.Unlock();
	
	// Now, we simply have to store the decoded samples in the voiceblock

	vb->SetSamples(speech,160*2);
	vb->SetNumSamples(160);
	vb->SetBytesPerSample(2);
	
	return 0;
}

bool JVOIPSpeexCompression::SupportsSampleInterval(int ival)
{
	if (!compinit)
		return false;
	if ((ival <= 0) || (ival != JVOIPSPEEXCOMPRESSION_SAMPLEINTERVAL))
		return false;
	return true;
}

int JVOIPSpeexCompression::SetSampleInterval(int ival)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if ((ival <= 0) || (ival != JVOIPSPEEXCOMPRESSION_SAMPLEINTERVAL))
		return ERR_JVOIPLIB_SPEEXCOMP_UNSUPPORTEDSAMPLEINTERVAL;
	return 0;
}

bool JVOIPSpeexCompression::SupportsInputSamplingRate(int irate)
{
	if (!compinit)
		return false;
	if (irate != JVOIPSPEEXCOMPRESSION_SAMPLERATE)
		return false;
	return true;
}

int JVOIPSpeexCompression::SetInputSamplingRate(int irate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (irate != JVOIPSPEEXCOMPRESSION_SAMPLERATE)
		return ERR_JVOIPLIB_SPEEXCOMP_UNSUPPORTEDSAMPLERATE;
	return 0;
}

bool JVOIPSpeexCompression::SupportsInputBytesPerSample(int inputbytespersample)
{
	if (!compinit)
		return false;
	return true;
}

int JVOIPSpeexCompression::SetInputBytesPerSample(int inputbytespersample)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

int JVOIPSpeexCompression::GetComponentState(JVOIPComponentState **compstate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	*compstate = NULL;
	return 0;
}

int JVOIPSpeexCompression::SetComponentState(JVOIPComponentState *compstate)
{
	if (!compinit)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	return 0;
}

std::string JVOIPSpeexCompression::GetComponentName()
{
	return std::string("JVOIPSpeexCompression");
}

std::string JVOIPSpeexCompression::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal Speex compression module");
}

std::vector<JVOIPCompParamInfo> *JVOIPSpeexCompression::GetComponentParameters() throw (JVOIPException)
{
	return NULL;
}

void *JVOIPSpeexCompression::Thread()
{
	JThread::ThreadStarted();
	
	bool stop;
	time_t prevchecktime;

	prevchecktime = time(NULL);

	stopthreadmutex.Lock();
	stop = stopthread;
	stopthreadmutex.Unlock();
	while (!stop)
	{
		time_t curtime;
		struct timeval tv;
		
		tv.tv_sec = 0;
		tv.tv_usec = 100000; // 100 millisecods -> make sure we don't have to wait too long
#ifndef WIN32
		select(FD_SETSIZE,NULL,NULL,NULL,&tv);
#else
		Sleep((DWORD)(((double)tv.tv_sec)*1000.0+((double)tv.tv_usec)/1000.0));
#endif // WIN32

		curtime = time(NULL);
		if ((curtime - prevchecktime) > JVOIPSPEEXCOMPRESSION_CHECKDELAY)
		{
			std::map<VoIPFramework::VOIPuint64,SpeexStateInfo *>::iterator it;
			std::list<VoIPFramework::VOIPuint64> sourceids;
			std::list<VoIPFramework::VOIPuint64>::const_iterator it2;
			
			statemutex.Lock();
			it = speexstates.begin();
			while (it != speexstates.end())
			{
				if ((curtime - it->second->lasttime) > JVOIPSPEEXCOMPRESSION_TIMEOUTDELAY)
				{
					// NOTE: we cannot simply call an 'erase'
					// function because the map::erase function
					// does not return a new iterator. So, we'll
					// store the keys which need to be erased
					// in a list and delete all entries afterwards
					// Also note that we do need to erase the
					// SpeexStateInfo* here
					delete it->second; 
					sourceids.push_back(it->first);
				}
				it++;
			}
			for (it2 = sourceids.begin() ; it2 != sourceids.end() ; ++it2)
				speexstates.erase(*it2);
			statemutex.Unlock();
		
			prevchecktime = curtime;
		}

		stopthreadmutex.Lock();
		stop = stopthread;
		stopthreadmutex.Unlock();
	}

	return NULL;
}

void JVOIPSpeexCompression::StopThread()
{
	time_t t = time(NULL);

	stopthreadmutex.Lock();
	stopthread = true;
	stopthreadmutex.Unlock();
	while (IsRunning() && (time(NULL) - t) < 5)
		;
	if (IsRunning())
	{
		std::cerr << "JVOIPSpeexCompression::StopThread -- warning: having to kill thread" << std::endl;
		Kill();
	}
}

#endif // JVOIPDISABLE_COMPRESSION_SPEEX

