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

#ifndef JVOIPDPCMCOMPRESSION_H

#define JVOIPDPCMCOMPRESSION_H

#include "jvoipconfig.h"

#ifndef JVOIPDISABLE_COMPRESSION_DPCM

#include "jvoipcompressionmodule.h"

#define JVOIPDPCMCOMPRESSION_DEFENCBITS					5
#define JVOIPDPCMCOMPRESSION_MINENCBITS					2
#define JVOIPDPCMCOMPRESSION_MAXENCBITS					7

#define JVOIPDPCMCOMPRESSION_BUFFERSIZE					64000

class JVOIPDPCMCompressionParams : public JVOIPComponentParams
{
public:
	JVOIPDPCMCompressionParams(int encodebits = JVOIPDPCMCOMPRESSION_DEFENCBITS) 		{ numencodebits = encodebits; }
	~JVOIPDPCMCompressionParams() 								{ }
	JVOIPComponentParams *CreateCopy() const						{ return new JVOIPDPCMCompressionParams(numencodebits); }
	int GetNumEncodeBits() const								{ return numencodebits; }
	void SetNumEncodeBits(int n)								{ numencodebits = n; }
private:
	int numencodebits;
};

class JVOIPDPCMCompression : public JVOIPCompressionModule
{
public:
	JVOIPDPCMCompression(JVOIPSession *sess);
	~JVOIPDPCMCompression();
	int InitCompressor(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams);
	int CleanupCompressor();
	int InitDecompressor();
	int CleanupDecompressor();

	int Compress(VoIPFramework::VoiceBlock *vb);
	int Decompress(VoIPFramework::VoiceBlock *vb,VoIPFramework::VOIPuint64 sourceid);

	bool SupportsSampleInterval(int ival);
	int SetSampleInterval(int ival);
	bool SupportsInputSamplingRate(int irate);
	int SetInputSamplingRate(int irate);
	bool SupportsInputBytesPerSample(int inputbytespersample);
	int SetInputBytesPerSample(int inputbytespersample);

	int GetComponentState(JVOIPComponentState **compstate);
	int SetComponentState(JVOIPComponentState *compstate);

	std::string GetComponentName();
	std::string GetComponentDescription();
	std::vector<JVOIPCompParamInfo> *GetComponentParameters() throw (JVOIPException);
private:
	inline void ResetEncoder(VoIPFramework::VoiceBlock *vb);
	inline int EncodeFirstValue();
	inline int GetNextValue(int *value);
	inline int EncodeValue(int diff,int *prevvalue);
	inline int GetEncodeLength();
	inline bool ResetDecoder(VoIPFramework::VoiceBlock *vb);
	inline bool GetFirstSampleValue(int *val);
	inline bool StoreDecodeValue(int val);
	inline bool GetNextSampleDifference(int *diff);
	inline int GetDecodeLength();

	bool compinit;
	int numencodebits,maxvalue,minvalue;
	int numdecodebits;

	unsigned char *data;
	unsigned char buffer[JVOIPDPCMCOMPRESSION_BUFFERSIZE];
	int bytepos,bitpos,datapos,datalen;
	int bytespersample;
	int totallen;
};

#endif // JVOIPDISABLE_COMPRESSION_DPCM

#endif // JVOIPDPCMCOMPRESSION_H

