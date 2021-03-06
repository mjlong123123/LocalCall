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
#include "jvoiprtptransmission.h"
#include "jvoipcompression.h"
#include "jvoiperrors.h"
#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpsourcedata.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "voiceblock.h"
#include <time.h>
#include <string.h>
#include <iostream>
#ifndef WIN32
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else 
	#include <winsock2.h>
#endif // WIN32

#include "debugnew.h"

#define JVOIPRTPTRANS_JVOIPPAYLOADTYPE						101
#define JVOIPRTPTRANS_HEADEROVERHEAD						(20 /* min IP header */ + 8 /* UDP header */ + 12 /* min RTP header */)
#define JVOIPRTPTRANS_JITTERADJUSTINTERVAL					3 // every three seconds

#define JVOIPRTPTRANS_MAXPACKDIST						128
#define JVOIPRTPTRANS_MAXPACKOUTRANGECOUNT					4

#define JVOIPRTPTRANS_MAXTSDIST							4410000L
#define JVOIPRTPTRANS_MAXTSOUTRANGECOUNT					4

#define JVOIPRTPTRANS_PAYLOADTYPE_PCMU						0
#define JVOIPRTPTRANS_PAYLOADTYPE_GSM						3
#define JVOIPRTPTRANS_PAYLOADTYPE_LPC						7

// Here is the component state info for the RTP transmitter

class JVOIPRTPTransmissionState : public JVOIPComponentState
{
public:
	JVOIPRTPTransmissionState(std::list<JVOIPRTPTransmission::ParticipantInfo*> &particip,
	                          std::list<JVOIPuint32> &mcast,
	                          std::list<JVOIPRTPTransmission::IPPortPair> &destin,
	                          std::list<JVOIPRTPTransmission::IPPortPair> &accept,
	                          std::list<JVOIPRTPTransmission::IPPortPair> &ignore,
	                          JVOIPSessionParams::ReceiveType recvtype,
	                          int curparticipid,
	                          double bytessent,
	                          double bytesreceived) throw (JVOIPException);
	~JVOIPRTPTransmissionState();
	std::list<JVOIPRTPTransmission::ParticipantInfo*> &Participants() 		{ return participants; }
	std::list<JVOIPuint32> &MulticastIPs() 						{ return mcastIPs; }
        std::list<JVOIPRTPTransmission::IPPortPair> &Destinations()			{ return destinations; }
	std::list<JVOIPRTPTransmission::IPPortPair> &AcceptList()			{ return acceptlist; }
	std::list<JVOIPRTPTransmission::IPPortPair> &IgnoreList()			{ return ignorelist; }
	JVOIPSessionParams::ReceiveType ReceiveType() const				{ return receivetype; }
	int CurrentID() const								{ return curid; }
	double BytesSent() const							{ return numbytessent; }
	double BytesReceived() const							{ return numbytesreceived; }
private:
	std::list<JVOIPRTPTransmission::ParticipantInfo*> participants;
	std::list<JVOIPuint32> mcastIPs;
	std::list<JVOIPRTPTransmission::IPPortPair> destinations,acceptlist,ignorelist;
	JVOIPSessionParams::ReceiveType receivetype;
	int curid;
	double numbytessent,numbytesreceived;
};

JVOIPRTPTransmissionState::JVOIPRTPTransmissionState(std::list<JVOIPRTPTransmission::ParticipantInfo*> &particip,
	                          std::list<JVOIPuint32> &mcast,
	                          std::list<JVOIPRTPTransmission::IPPortPair> &destin,
	                          std::list<JVOIPRTPTransmission::IPPortPair> &accept,
	                          std::list<JVOIPRTPTransmission::IPPortPair> &ignore,
	                          JVOIPSessionParams::ReceiveType recvtype,
	                          int curparticipid,
	                          double bytessent,
	                          double bytesreceived) throw (JVOIPException)
{
	std::list<JVOIPRTPTransmission::ParticipantInfo*>::const_iterator it1;
	std::list<JVOIPuint32>::const_iterator it2;
	std::list<JVOIPRTPTransmission::IPPortPair>::const_iterator it3;
	
	for (it1 = particip.begin() ; it1 != particip.end() ; ++it1)
	{
		JVOIPRTPTransmission::ParticipantInfo *p;
		
		p = new JVOIPRTPTransmission::ParticipantInfo(*(*it1));
		if (p == NULL)
			throw JVOIPException(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
		participants.push_back(p);
	}
	
	for (it2 = mcast.begin() ; it2 != mcast.end() ; ++it2)
		mcastIPs.push_back(*it2);
	for (it3 = destin.begin() ; it3 != destin.end() ; ++it3)
		destinations.push_back(*it3);
	for (it3 = accept.begin() ; it3 != accept.end() ; ++it3)
		acceptlist.push_back(*it3);
	for (it3 = ignore.begin() ; it3 != ignore.end() ; ++it3)
		ignorelist.push_back(*it3);

	receivetype = recvtype;
	curid = curparticipid;
	numbytessent = bytessent;
	numbytesreceived = bytesreceived;			
}

JVOIPRTPTransmissionState::~JVOIPRTPTransmissionState()
{
	std::list<JVOIPRTPTransmission::ParticipantInfo*>::const_iterator it;
	
	for (it = participants.begin() ; it != participants.end() ; ++it)
		delete (*it);
	participants.clear();
}

// the parameter class

JVOIPComponentParams *JVOIPRTPTransmissionParams::CreateCopy() const
{
	return new JVOIPRTPTransmissionParams(portbase,autoadjustbuffering,defaultbuffer,minimbuffer,acceptownpackets,localip);
}

// 
// Our own RTPSession instance
// 

class JVOIPRTPTransSession : public RTPSession
{
public:
	JVOIPRTPTransSession(JVOIPRTPTransmission *trans) { transmitter = trans; }
protected:
	void OnNewSource(RTPSourceData *srcdat) { transmitter->RTPJoinHandler(srcdat->GetSSRC()); }
	void OnRemoveSource(RTPSourceData *srcdat) { transmitter->RTPLeaveHandler(srcdat->GetSSRC()); }
private:
	JVOIPRTPTransmission *transmitter;
};

//
// The actual component
//

JVOIPRTPTransmission::JVOIPRTPTransmission(JVOIPSession *sess) throw (JVOIPException) : JVOIPTransmission(sess)
{
	if (participantmutex.Init() < 0)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_CANTINITMUTEX);
	init = false;	
	portbase = 0;
	localip = 0;
	autoadjustbuffering = false;
	acceptownpackets = false;
	defaultbuffer = 0;
}

JVOIPRTPTransmission::~JVOIPRTPTransmission()
{
	Cleanup();
}
	
int JVOIPRTPTransmission::Init(int sampinterval,int inputsamprate,int inputbytespersample,const JVOIPComponentParams *componentparams)
{
	JVOIPRTPTransmissionParams defparams,*usrparams;
	int status;

	if (init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTALREADYINIT;
		
	if (!SupportedSampleRate(inputsamprate))
		return ERR_JVOIPLIB_RTPTRANS_UNSUPPORTEDSAMPLERATE;
	
	samplingrate = inputsamprate;
	bytespersample = inputbytespersample;
	interval = ((double)sampinterval)/1000.0; // convert to seconds

	if (componentparams == NULL)
		usrparams = &defparams;
	else if ((usrparams = dynamic_cast<JVOIPRTPTransmissionParams *>(const_cast<JVOIPComponentParams *>(componentparams))) == NULL)
		usrparams = &defparams;
	
	portbase = usrparams->GetPortBase();
	localip = usrparams->GetLocalIP();
	autoadjustbuffering = usrparams->GetAutoAdjustBuffer();
	defaultbuffer = usrparams->GetDefaultBuffering();
	minimbuffer = usrparams->GetMinimumBuffering();
	acceptownpackets = usrparams->GetAcceptOwnPackets();
	receivetype = JVOIPSessionParams::AcceptAll;
	if ((status = CreateNewRTPSession()) < 0)
		return status;
		
	// obtain the local IP that JRTPLIB uses
	// rtpsess->GetLocalIP(&localip);
		
	curid = 0;
	numbytessent = 0;
	numbytesreceived = 0;
	
	init = true;
	return 0;
}

int JVOIPRTPTransmission::Cleanup()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	rtpsess->BYEDestroy(RTPTime(0.5),0,0);
	delete rtpsess;
	init = false;
	
	ClearParticipantInfo();
	mcastIPs.clear();
	destinations.clear();
	acceptlist.clear();
	ignorelist.clear();
	
	return 0;
}

void JVOIPRTPTransmission::Reset()
{
	if (!init)
		return;

	std::list<ParticipantInfo*>::const_iterator it;
	
	participantmutex.Lock();
	for (it = participants.begin() ; it != participants.end() ; ++it)
	{
		if (!(*it)->isnewsource)
			(*it)->ClearAllPublicFields();
	}
	participantmutex.Unlock();
}

int JVOIPRTPTransmission::StartVoiceSourceIteration()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	rtpsess->BeginDataAccess();
	participantmutex.Lock();
	return 0;
}

void JVOIPRTPTransmission::EndVoiceSourceIteration()
{
	if (!init)
		return;
	rtpsess->EndDataAccess();
	participantmutex.Unlock();
}

bool JVOIPRTPTransmission::GotoFirstVoiceSource()
{
	bool val;
	RTPSourceData *srcdat;
	ParticipantInfo *inf;

	if (!init)
		return false;
	
	val = rtpsess->GotoFirstSourceWithData();
	if (!val)
		return false;
	
	do
	{
		srcdat = rtpsess->GetCurrentSourceInfo();
		inf = GetParticipantInfo(srcdat->GetSSRC());
		if (!inf)
			val = rtpsess->GotoNextSourceWithData();
	} while (!inf && val);
	
	val = (inf)?true:false;
	return val;
}

bool JVOIPRTPTransmission::GotoNextVoiceSource()
{
	bool val;
	RTPSourceData *srcdat;
	ParticipantInfo *inf;

	if (!init)
		return false;
	
	val = rtpsess->GotoNextSourceWithData();
	if (!val)
		return false;
	
	do
	{
		srcdat = rtpsess->GetCurrentSourceInfo();
		inf = GetParticipantInfo(srcdat->GetSSRC());
		if (!inf)
			val = rtpsess->GotoNextSourceWithData();
	} while (!inf && val);
	
	val = (inf)?true:false;
	return val;
}

VoIPFramework::VOIPuint64 JVOIPRTPTransmission::GetVoiceSourceID()
{
	RTPSourceData *srcdat;
	ParticipantInfo *inf;
	VoIPFramework::VOIPuint64 id = 0;
	
	if (!init)
		return 0;
	
	srcdat = rtpsess->GetCurrentSourceInfo();
	if (srcdat)
	{
		inf = GetParticipantInfo(srcdat->GetSSRC());
		if (inf)
			id = inf->ID();
	}
	return id;
}

int JVOIPRTPTransmission::Poll()
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	// Polling is done in the background
	return 0;
}

bool JVOIPRTPTransmission::SourceHasMoreData()
{
	RTPSourceData *srcdat;
	ParticipantInfo *inf;
	bool val = false;
	
	if (!init)
		return 0;
	
	srcdat = rtpsess->GetCurrentSourceInfo();
	if (srcdat)
	{
		inf = GetParticipantInfo(srcdat->GetSSRC());
		if (inf)
			val = srcdat->HasData();
	}
	return val;
}

int JVOIPRTPTransmission::SetSampleOffset(VoIPFramework::VOIPdouble offset)
{
	std::list<ParticipantInfo*>::const_iterator it1;
	std::list<PacknumOffsetPair>::iterator it2;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	cursampleoffset = offset;
	
	// check if we can clean some packet number lists

	participantmutex.Lock();
	
	for (it1 = participants.begin() ; it1 != participants.end() ; ++it1)
	{
		if (!(*it1)->isnewsource)
		{
			it2 = (*it1)->receivedpacknums.begin();
			while (it2 != (*it1)->receivedpacknums.end() && (*it2).offset <= offset)
				it2 = (*it1)->receivedpacknums.erase(it2);
		}
	}

	participantmutex.Unlock();
	
	return 0;
}

bool JVOIPRTPTransmission::AddDestination(JVOIPuint32 destip,JVOIPuint16 destportbase)
{
	std::list<IPPortPair>::const_iterator it;
	bool found,val;

	if (!init)
		return false;
	
	// search to see if the destination exists
	
	for (found = false,it = destinations.begin() ; !found && it != destinations.end() ; ++it)
	{
		if ((*it).ip == destip && (*it).port == destportbase)
			found = true;
	}
	if (!found)
	{
		RTPIPv4Address dest(destip,destportbase);
		
		if (rtpsess->AddDestination(dest) < 0)
			val = false;
		else
		{	
			destinations.push_back(IPPortPair(destip,destportbase));
			val = true;
		}
	}
	else
		val = true;
	return val;
}

bool JVOIPRTPTransmission::DeleteDestination(JVOIPuint32 destip,JVOIPuint16 destportbase)
{
	std::list<IPPortPair>::iterator it;
	bool found,val;

	if (!init)
		return false;
	
	// search to see if the destination exists
	
	for (found = false,it = destinations.begin() ; !found && it != destinations.end() ;      )
	{
		if ((*it).ip == destip && (*it).port == destportbase)
			found = true;
		else
			 ++it;
	}
	if (!found)
		return false;
	
	RTPIPv4Address dest(destip,destportbase);

	if (rtpsess->DeleteDestination(dest) < 0)
		val = false;
	else
	{
		val = true;
		destinations.erase(it);
	}
	return val;
}

void JVOIPRTPTransmission::ClearDestinations()
{
	if (!init)
		return;
	rtpsess->ClearDestinations();
	destinations.clear();
}

bool JVOIPRTPTransmission::SupportsMulticasting()
{
	bool val;

	if (!init)
		return false;
	
	val = rtpsess->SupportsMulticasting();
	return val;
}

bool JVOIPRTPTransmission::JoinMulticastGroup(JVOIPuint32 mcastip)
{
	std::list<JVOIPuint32>::const_iterator it;
	bool found,val;

	if (!init)
		return false;
	
	// check if the IP is already in the list...
	for (found = false,it = mcastIPs.begin() ; !found && it != mcastIPs.end() ; ++it)
	{
		if ((*it) == mcastip)
			found = true;
	}
	
	if (!found)
	{
		RTPIPv4Address mcastgroup(mcastip);
		
		if (rtpsess->JoinMulticastGroup(mcastgroup) < 0)
			val = false;
		else
		{
			mcastIPs.push_back(mcastip);
			val = true;
		}
	}
	else
		val = true;
	return val;
}

bool JVOIPRTPTransmission::LeaveMulticastGroup(JVOIPuint32 mcastip)
{
	std::list<JVOIPuint32>::iterator it;
	bool found,val;

	if (!init)
		return false;
	
	// check if the IP is in the list...
	for (found = false,it = mcastIPs.begin() ; !found && it != mcastIPs.end() ;        )
	{
		if ((*it) == mcastip)
			found = true;
		else
			++it;
	}
	
	if (!found)
		return false;
		
	RTPIPv4Address mcastgroup(mcastip);
	
	if (rtpsess->LeaveMulticastGroup(mcastgroup) < 0)
		val = false;
	else
	{
		val = true;
		mcastIPs.erase(it);
	}
	return val;
}

void JVOIPRTPTransmission::LeaveAllMulticastGroups()
{
	if (!init)
		return;
	rtpsess->LeaveAllMulticastGroups();
	mcastIPs.clear();
}

bool JVOIPRTPTransmission::SupportsReceiveType(JVOIPSessionParams::ReceiveType rt)
{
	if (!init)
		return false;
	switch(rt)
	{
	case JVOIPSessionParams::AcceptAll:
		return true;
	case JVOIPSessionParams::AcceptSome:
		return true;
	case JVOIPSessionParams::IgnoreSome:
		return true;
	}
	return false;
}

bool JVOIPRTPTransmission::SetReceiveType(JVOIPSessionParams::ReceiveType rt)
{
	std::list<IPPortPair>::const_iterator it;

	if (!init)
		return false;
	
	if (receivetype == rt) // nothing changed
		return true;

	receivetype = rt;
	
	rtpsess->ClearIgnoreList();
	rtpsess->ClearAcceptList();
	
	switch(rt)
	{
	case JVOIPSessionParams::AcceptSome:
		{
			rtpsess->SetReceiveMode(RTPTransmitter::AcceptSome);
			for (it = acceptlist.begin() ; it != acceptlist.end() ; ++it)
			{
				if ((*it).port == JVOIP_ALLPORTS)
				{
					RTPIPv4Address dest((*it).ip);
					rtpsess->AddToAcceptList(dest);
				}
				else
				{
					RTPIPv4Address dest((*it).ip,(*it).port);
					rtpsess->AddToAcceptList(dest);
				}
			}
		}
		break;
	case JVOIPSessionParams::IgnoreSome:
		{
			rtpsess->SetReceiveMode(RTPTransmitter::IgnoreSome);
			for (it = ignorelist.begin() ; it != ignorelist.end() ; ++it)
			{
				if ((*it).port == JVOIP_ALLPORTS)
				{
					RTPIPv4Address dest((*it).ip);
					rtpsess->AddToIgnoreList(dest);
				}
				else
				{
					RTPIPv4Address dest((*it).ip,(*it).port);
					rtpsess->AddToIgnoreList(dest);
				}
			}
		}
		break;
	case JVOIPSessionParams::AcceptAll:
	default:
		rtpsess->SetReceiveMode(RTPTransmitter::AcceptAll);
	}

	return true;
}

bool JVOIPRTPTransmission::AddToAcceptList(JVOIPuint32 ip,JVOIPuint16 port /* = JVOIP_ALLPORTS */)
{
	std::list<IPPortPair>::iterator it;
	bool val = false;
	
	if (!init)
		return false;
	
	if (port == JVOIP_ALLPORTS)
	{
		it = acceptlist.begin();
		while(it != acceptlist.end())
		{
			if ((*it).ip == ip && (*it).port == JVOIP_ALLPORTS)
				return true;

			if ((*it).ip == ip)
			{
				// remove the specific IP-port combinations which
				// have 'ip' as address
				if (receivetype == JVOIPSessionParams::AcceptSome)
				{
					RTPIPv4Address dest(ip,(*it).port);
					rtpsess->DeleteFromAcceptList(dest);
				}
				it = acceptlist.erase(it);
			}
			else
				++it;
		}
		
		// now, add the entry
		
		if (receivetype == JVOIPSessionParams::AcceptSome)
		{
			RTPIPv4Address dest(ip);
			rtpsess->AddToAcceptList(dest);
		}
		val = true;
		acceptlist.push_back(IPPortPair(ip,port));
	}
	else // one specific port
	{
		for (it = acceptlist.begin() ; it != acceptlist.end() ; ++it)
		{
			if ((*it).ip == ip && ((*it).port == JVOIP_ALLPORTS || (*it).port == port))
				return true;
		}
		
		// The IP-port pair is not currently in the accept list
		
		if (receivetype == JVOIPSessionParams::AcceptSome)
		{
			RTPIPv4Address dest(ip,port);
			rtpsess->AddToAcceptList(dest);
		}
		val = true;
		acceptlist.push_back(IPPortPair(ip,port));
	}
	return val;
}

bool JVOIPRTPTransmission::DeleteFromAcceptList(JVOIPuint32 ip,JVOIPuint16 port /* = JVOIP_ALLPORTS */)
{
	std::list<IPPortPair>::iterator it;
	
	if (!init)
		return false;
	
	for (it = acceptlist.begin() ; it != acceptlist.end() ; ++it)
	{
		if ((*it).ip == ip && (*it).port == port)
		{
			if (receivetype == JVOIPSessionParams::AcceptSome)
			{
				if (port == JVOIP_ALLPORTS)
				{
					RTPIPv4Address dest(ip);
					rtpsess->DeleteFromAcceptList(dest);
				}
				else
				{
					RTPIPv4Address dest(ip,port);
					rtpsess->DeleteFromAcceptList(dest);
				}
			}	
			acceptlist.erase(it);
			return true;
		}
	}
	return false;
}

void JVOIPRTPTransmission::ClearAcceptList()
{
	if (!init)
		return;
	if (receivetype == JVOIPSessionParams::AcceptSome)
		rtpsess->ClearAcceptList();
	acceptlist.clear();
}

bool JVOIPRTPTransmission::AddToIgnoreList(JVOIPuint32 ip,JVOIPuint16 port /* = JVOIP_ALLPORTS */)
{
	std::list<IPPortPair>::iterator it;
	bool val = false;
	
	if (!init)
		return false;
	
	if (port == JVOIP_ALLPORTS)
	{
		it = ignorelist.begin();
		while(it != ignorelist.end())
		{
			if ((*it).ip == ip && (*it).port == JVOIP_ALLPORTS)
				return true;

			if ((*it).ip == ip)
			{
				// remove the specific IP-port combinations which
				// have 'ip' as address
				
				if (receivetype == JVOIPSessionParams::IgnoreSome)
				{
					RTPIPv4Address dest(ip,(*it).port);
					rtpsess->DeleteFromIgnoreList(dest);
				}
				it = ignorelist.erase(it);
			}
			else
				++it;
		}
		
		// now, add the entry
		
		if (receivetype == JVOIPSessionParams::IgnoreSome)
		{
			RTPIPv4Address dest(ip);
			rtpsess->AddToIgnoreList(dest);
		}
		val = true;
		ignorelist.push_back(IPPortPair(ip,port));
	}
	else // one specific port
	{
		for (it = ignorelist.begin() ; it != ignorelist.end() ; ++it)
		{
			if ((*it).ip == ip && ((*it).port == JVOIP_ALLPORTS || (*it).port == port))
				return true;
		}
		
		// The IP-port pair is not currently in the accept list

		if (receivetype == JVOIPSessionParams::IgnoreSome)
		{
			RTPIPv4Address dest(ip,port);
			rtpsess->AddToIgnoreList(dest);
		}
		val = true;
		ignorelist.push_back(IPPortPair(ip,port));
	}
	return val;
}

bool JVOIPRTPTransmission::DeleteFromIgnoreList(JVOIPuint32 ip,JVOIPuint16 port /* = JVOIP_ALLPORTS */)
{
	std::list<IPPortPair>::iterator it;
	
	if (!init)
		return false;
	
	for (it = ignorelist.begin() ; it != ignorelist.end() ; ++it)
	{
		if ((*it).ip == ip && (*it).port == port)
		{
			if (receivetype == JVOIPSessionParams::IgnoreSome)
			{
				if (port == JVOIP_ALLPORTS)
				{
					RTPIPv4Address dest(ip);
					rtpsess->DeleteFromIgnoreList(dest);
				}
				else
				{
					RTPIPv4Address dest(ip,port);
					rtpsess->DeleteFromIgnoreList(dest);
				}
			}	
			ignorelist.erase(it);
			return true;
		}
	}
	return false;
}

void JVOIPRTPTransmission::ClearIgnoreList()
{
	if (!init)
		return;

	if (receivetype == JVOIPSessionParams::IgnoreSome)
		rtpsess->ClearIgnoreList();
	ignorelist.clear();
}

bool JVOIPRTPTransmission::SupportsSampleInterval(int ival)
{
	if (!init)
		return false;
	return true;
}

bool JVOIPRTPTransmission::SupportsInputSamplingRate(int irate)
{
	if (!init)
		return false;
	return SupportedSampleRate(irate);
}

bool JVOIPRTPTransmission::SupportsInputBytesPerSample(int inputbytespersample)
{
	if (!init)
		return false;
	if (inputbytespersample == 1 || inputbytespersample == 2)
		return true;
	return false;
}

int JVOIPRTPTransmission::SetSampleInterval(int ival)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	interval = ((double)ival)/1000.0; // convert to seconds
	return 0;
}

int JVOIPRTPTransmission::SetInputSamplingRate(int irate)
{
	int status;
	std::list<JVOIPuint32>::const_iterator it1;
	std::list<IPPortPair>::const_iterator it2;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	// NOTE: Since this will have an effect on the timestamp, we probably
	//       should work with a new rtp session...
	
	rtpsess->BYEDestroy(RTPTime(0.5),0,0);
	delete rtpsess;
	
	if ((status = CreateNewRTPSession()) < 0)
	{
		rtpsess = NULL;
		return status;
	}
	
	// Reinstall the destinations, acceptlist, etc.

	for (it1 = mcastIPs.begin() ; it1 != mcastIPs.end() ; ++it1)
	{
		RTPIPv4Address mcastgroup(*it1);
		if ((rtpsess->JoinMulticastGroup(mcastgroup)) < 0)
			return ERR_JVOIPLIB_RTPTRANS_CANTREJOINMULTICASTGROUP;
	}
	for (it2 = destinations.begin() ; it2 != destinations.end() ; ++it2)
	{
		RTPIPv4Address dest((*it2).ip,(*it2).port);
		if ((rtpsess->AddDestination(dest)) < 0)
			return ERR_JVOIPLIB_RTPTRANS_CANTREINSTALLDESTINATIONS;
	}

	if (receivetype == JVOIPSessionParams::AcceptSome)
	{
		for (it2 = acceptlist.begin() ; it2 != acceptlist.end() ; ++it2)
		{
			if ((*it2).port == JVOIP_ALLPORTS)
			{
				RTPIPv4Address dest((*it2).ip);
				rtpsess->AddToAcceptList(dest);
			}
			else
			{
				RTPIPv4Address dest((*it2).ip,(*it2).port);
				rtpsess->AddToAcceptList(dest);
			}
		}
	}
	else if (receivetype == JVOIPSessionParams::IgnoreSome)
	{
		for (it2 = ignorelist.begin() ; it2 != ignorelist.end() ; ++it2)
		{
			if ((*it2).port == JVOIP_ALLPORTS)
			{
				RTPIPv4Address dest((*it2).ip);
				rtpsess->AddToIgnoreList(dest);
			}
			else
			{
				RTPIPv4Address dest((*it2).ip,(*it2).port);
				rtpsess->AddToIgnoreList(dest);
			}	
		}
	}	
	samplingrate = irate;
	return 0;
}

int JVOIPRTPTransmission::SetInputBytesPerSample(int inputbytespersample)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
	// REMINDER: check if this is enough!
	bytespersample = inputbytespersample;
	
	return 0;
}

int JVOIPRTPTransmission::GetComponentState(JVOIPComponentState **compstate)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	
        try
        {
        	JVOIPRTPTransmissionState *s;
        	
		participantmutex.Lock();
        	s = new JVOIPRTPTransmissionState(participants,mcastIPs,destinations,
        					  acceptlist,ignorelist,receivetype,
        					  curid,numbytessent,numbytesreceived);
		participantmutex.Unlock();
        	if (s == NULL)
        		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
        	*compstate = s;
        }
        catch(JVOIPException e)
        {
        	return e.GetErrorNumber();
        }
			
	return 0;
}

int JVOIPRTPTransmission::SetComponentState(JVOIPComponentState *compstate)
{
	std::list<ParticipantInfo*>::const_iterator it1;
	std::list<JVOIPuint32>::const_iterator it2;
	std::list<IPPortPair>::const_iterator it3;
	JVOIPRTPTransmissionState *state;
	
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;
	if (compstate == NULL)
		return ERR_JVOIPLIB_GENERAL_ILLEGALSTATEPARAMETER;
	state = dynamic_cast<JVOIPRTPTransmissionState *>(compstate);
	if (state == NULL)
		return ERR_JVOIPLIB_GENERAL_ILLEGALSTATEPARAMETER;
		
	ClearParticipantInfo();
	mcastIPs.clear();
	destinations.clear();
	acceptlist.clear();
	ignorelist.clear();
	rtpsess->LeaveAllMulticastGroups();
	rtpsess->ClearDestinations();
	rtpsess->ClearAcceptList();
	rtpsess->ClearIgnoreList();

	switch(state->ReceiveType())
	{
	case JVOIPSessionParams::AcceptSome:
		rtpsess->SetReceiveMode(RTPTransmitter::AcceptSome);
		break;
	case JVOIPSessionParams::IgnoreSome:
		rtpsess->SetReceiveMode(RTPTransmitter::IgnoreSome);
	case JVOIPSessionParams::AcceptAll:
		break;
	default:
		rtpsess->SetReceiveMode(RTPTransmitter::AcceptAll);
	}

	participantmutex.Lock();
	for (it1 = state->Participants().begin() ; it1 != state->Participants().end() ; ++it1)
	{
		ParticipantInfo *p;
		
		p = new ParticipantInfo(*(*it1));
		if (p == NULL)
		{
			participantmutex.Unlock();
			return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
		}
		participants.push_back(p);
	}
	participantmutex.Unlock();
	
	for (it2 = state->MulticastIPs().begin() ; it2 != state->MulticastIPs().end() ; ++it2)
	{
		RTPIPv4Address mcastgroup(*it2);
		if (rtpsess->JoinMulticastGroup(mcastgroup) < 0)
			return ERR_JVOIPLIB_RTPTRANS_CANTREJOINMULTICASTGROUP;
		mcastIPs.push_back(*it2);
	}
	
	for (it3 = state->Destinations().begin() ; it3 != state->Destinations().end() ; ++it3)
	{
		RTPIPv4Address dest((*it3).ip,(*it3).port);
		if (rtpsess->AddDestination(dest) < 0)
			return ERR_JVOIPLIB_RTPTRANS_CANTREINSTALLDESTINATIONS;
		destinations.push_back(*it3);
	}

	switch(state->ReceiveType())
	{
	case JVOIPSessionParams::AcceptSome:
		rtpsess->SetReceiveMode(RTPTransmitter::AcceptSome);
		break;
	case JVOIPSessionParams::IgnoreSome:
		rtpsess->SetReceiveMode(RTPTransmitter::IgnoreSome);
		break;
	case JVOIPSessionParams::AcceptAll:
	default:
		rtpsess->SetReceiveMode(RTPTransmitter::AcceptAll);
	}

	for (it3 = state->AcceptList().begin() ; it3 != state->AcceptList().end() ; ++it3)
	{
		if (state->ReceiveType() == JVOIPSessionParams::AcceptSome)
		{
			if ((*it3).port == JVOIP_ALLPORTS)
			{
				RTPIPv4Address dest((*it3).ip);
				rtpsess->AddToAcceptList(dest);
			}
			else
			{
				RTPIPv4Address dest((*it3).ip,(*it3).port);
				rtpsess->AddToAcceptList(dest);
			}
		}
		acceptlist.push_back(*it3);
	}
	for (it3 = state->IgnoreList().begin() ; it3 != state->IgnoreList().end() ; ++it3)
	{
		if (state->ReceiveType() == JVOIPSessionParams::IgnoreSome)
		{
			if ((*it3).port == JVOIP_ALLPORTS)
			{
				RTPIPv4Address dest((*it3).ip);
				rtpsess->AddToIgnoreList(dest);
			}
			else
			{
				RTPIPv4Address dest((*it3).ip,(*it3).port);
				rtpsess->AddToIgnoreList(dest);
			}
		}
		ignorelist.push_back(*it3);
	}

	receivetype = state->ReceiveType();
	curid = state->CurrentID();
	numbytessent = state->BytesSent();
	numbytesreceived = state->BytesReceived();
	return 0;
}

std::string JVOIPRTPTransmission::GetComponentName()
{
	return std::string("JVOIPRTPTransmission");
}

std::string JVOIPRTPTransmission::GetComponentDescription()
{
	return std::string("JVOIPLIB Internal RTP transmitter (uses JRTPLIB)");
}

std::vector<JVOIPCompParamInfo> *JVOIPRTPTransmission::GetComponentParameters() throw (JVOIPException)
{
	std::vector<JVOIPCompParamInfo> *paraminfo;
	char str[256];
	struct in_addr ad;
	
	paraminfo = new std::vector<JVOIPCompParamInfo>(5);
	if (paraminfo == NULL)
		throw JVOIPException(ERR_JVOIPLIB_GENERAL_OUTOFMEM);
	
	// set the local portbase
	sprintf(str,"%d",portbase);
	(*paraminfo)[0].SetParameterName(std::string("RTP portbase"));
	(*paraminfo)[0].SetParameterValue(std::string(str));
	
	// set the local IP address
	ad.s_addr = htonl(localip);
	(*paraminfo)[1].SetParameterName(std::string("Local IP"));
	(*paraminfo)[1].SetParameterValue(std::string(inet_ntoa(ad)));
	
	// auto buffering
	(*paraminfo)[2].SetParameterName(std::string("Auto buffering"));
	if (autoadjustbuffering)
		(*paraminfo)[2].SetParameterValue(std::string("Yes"));
	else
		(*paraminfo)[2].SetParameterValue(std::string("No"));
	
	// default buffering
	sprintf(str,"%.3f",(float)defaultbuffer);
	(*paraminfo)[3].SetParameterName(std::string("Default buffering"));
	(*paraminfo)[3].SetParameterValue(std::string(str));
	
	// accept own packets
	(*paraminfo)[4].SetParameterName(std::string("Accept own packets"));
	if (acceptownpackets)
		(*paraminfo)[4].SetParameterValue(std::string("Yes"));
	else
		(*paraminfo)[4].SetParameterValue(std::string("No"));
	
	return paraminfo;
}

int JVOIPRTPTransmission::CreateNewRTPSession()
{
	int errcode;

	RTPSessionParams sessparams;
	RTPUDPv4TransmissionParams transparams;

	if (sessparams.SetUsePollThread(true) < 0)
		return ERR_JVOIPLIB_RTPTRANS_CANTINITRTPSESSION;

	switch(receivetype)
	{
	case JVOIPSessionParams::AcceptSome:
		sessparams.SetReceiveMode(RTPTransmitter::AcceptSome);
		break;
	case JVOIPSessionParams::IgnoreSome:
		sessparams.SetReceiveMode(RTPTransmitter::IgnoreSome);
		break;
	case JVOIPSessionParams::AcceptAll:
	default:
		sessparams.SetReceiveMode(RTPTransmitter::AcceptAll);
	}

	sessparams.SetMaximumPacketSize(JVOIPRTPTRANS_MAXPACKSIZE);
	sessparams.SetOwnTimestampUnit(1.0/((double)samplingrate));
	sessparams.SetAcceptOwnPackets(acceptownpackets);
	transparams.SetPortbase(portbase);
	if (localip != 0)
	{
		std::list<u_int32_t> iplist;

		iplist.push_back(localip);
		transparams.SetLocalIPList(iplist);
	}
	
	rtpsess = new JVOIPRTPTransSession(this);
	if (rtpsess == NULL)
		return ERR_JVOIPLIB_GENERAL_OUTOFMEM;
	
	if ((errcode = rtpsess->Create(sessparams,&transparams)) < 0)
	{
		delete rtpsess;
		return ERR_JVOIPLIB_RTPTRANS_CANTINITRTPSESSION;
	}
	
	return 0;
}

bool JVOIPRTPTransmission::SupportedSampleRate(int rate)
{
	if (rate != 4000 && rate != 8000 && rate != 11025 && rate != 22050 && rate != 44100)
		return false;
	return true;
}

void JVOIPRTPTransmission::ClearParticipantInfo()
{
	std::list<ParticipantInfo*>::const_iterator it;
	
	participantmutex.Lock();
	for (it = participants.begin() ; it != participants.end() ; ++it)
		delete (*it);
	participants.clear();
	participantmutex.Unlock();
}

inline JVOIPRTPTransmission::ParticipantInfo *JVOIPRTPTransmission::GetParticipantInfo(unsigned long ssrc)
{
	std::list<ParticipantInfo*>::const_iterator it;
	
	for (it = participants.begin() ; it != participants.end() ; ++it)
	{
		if ((*it)->SSRC() == ssrc)
			return (*it);
	}
	return NULL;
}

void JVOIPRTPTransmission::RTPJoinHandler(unsigned long ssrc)
{
	std::list<ParticipantInfo*>::iterator it;
	ParticipantInfo *newparticip;
	bool done;

	participantmutex.Lock();
	it = participants.begin();
	done = false;
	while (!done && it != participants.end())
	{
		if (ssrc <= (*it)->SSRC())
			done = true;
		else
			++it;
	}
		
	newparticip = new ParticipantInfo(++curid,ssrc);
	if (newparticip == NULL) // Out of memory
	{
		participantmutex.Unlock();
		return;
	}
	
	if (!done) // insert at the back of the array
		participants.push_back(newparticip);
	else // insert before position i
	{
		if (ssrc != (*it)->SSRC())
			participants.insert(it,newparticip);
	}
	participantmutex.Unlock();
}

void JVOIPRTPTransmission::RTPLeaveHandler(unsigned long ssrc)
{
	std::list<ParticipantInfo*>::iterator it;
	bool done;

	participantmutex.Lock();
	it = participants.begin();
	done = false;
	while (!done && it != participants.end())
	{
		if (ssrc <= (*it)->SSRC())
			done = true;
		else
			++it;
	}
		
	if (done)
	{
		if (ssrc == (*it)->SSRC())
		{
			delete (*it);
			participants.erase(it);
		}
	}
	participantmutex.Unlock();
}

int JVOIPRTPTransmission::SendBlock(VoIPFramework::VoiceBlock *vb)
{
	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	if ((int)vb->GetSampleRate() != samplingrate || vb->GetBytesPerSample() != bytespersample)
		return ERR_JVOIPLIB_GENERAL_VOICEBLOCKINFODOESNTMATCHSETTINGS;

	if (!vb->IsSilence())
	{
		int datalen;
		unsigned char payloadtype;
		bool marker;
		
		datalen = EncodeData(vb,&payloadtype,&marker);
		if (datalen < 0)
			return datalen; // error

		if (rtpsess->SendPacket(sendbuf,datalen,payloadtype,false,vb->GetNumSamples()) < 0)
			return ERR_JVOIPLIB_RTPTRANS_CANTSENDPACKET;
		
		numbytessent += datalen + JVOIPRTPTRANS_HEADEROVERHEAD;
	}
	else
	{
		if (rtpsess->IncrementTimestamp(vb->GetNumSamples()) < 0)
			return ERR_JVOIPLIB_RTPTRANS_CANTINCREMENTTIMESTAMP;
	}
	
	return 0;
}

int JVOIPRTPTransmission::EncodeData(VoIPFramework::VoiceBlock *vb,unsigned char *payloadtype,bool *marker)
{
	bool specificpayloadtype = false;
       	int len;
	
	// Tests for particular payloadtypes
	
	len = 0;
	if (!vb->Has3DInfo())
	{
		if (!vb->IsStereo() && vb->GetSampleRate() == 8000 &&
		    vb->GetMajorCompressionID() == JVOIPCOMPRESSIONTYPE_MAJOR_ULAWCOMPRESSION)
		{
			*payloadtype = JVOIPRTPTRANS_PAYLOADTYPE_PCMU;
			specificpayloadtype = true;
		}
		else if (!vb->IsStereo() && vb->GetSampleRate() == 8000 &&
		         vb->GetMajorCompressionID() == JVOIPCOMPRESSIONTYPE_MAJOR_GSMCOMPRESSION)
        	{
			*payloadtype = JVOIPRTPTRANS_PAYLOADTYPE_GSM;
			specificpayloadtype = true;
		}
		else if (!vb->IsStereo() && vb->GetSampleRate() == 8000 &&
		         vb->GetMajorCompressionID() == JVOIPCOMPRESSIONTYPE_MAJOR_LPCCOMPRESSION)
        	{
			*payloadtype = JVOIPRTPTRANS_PAYLOADTYPE_LPC;
			specificpayloadtype = true;
		}
	}	

        if (specificpayloadtype)
        {
        	if ((len = vb->GetNumBytes()) >= JVOIPRTPTRANS_MAXPACKSIZE)
        		return ERR_JVOIPLIB_RTPTRANS_PACKETTOOLARGE;
        	memcpy(sendbuf,vb->GetSamples(false),len);
        }
        else
        {
        	unsigned char infobyte = 0;

		*payloadtype = JVOIPRTPTRANS_JVOIPPAYLOADTYPE;

		// encode the sampling rate

		switch(samplingrate)
		{
		case 4000:
			infobyte = 0;
			break;
		case 8000:
			infobyte = 1;
			break;
		case 11025:
			infobyte = 2;
			break;
		case 22050:
			infobyte = 3;
			break;
		case 44100:
			infobyte = 4;
			break;
		default:
			std::cerr << "JVOIPRTPTransmission::EncodeData: Invalid sampling rate!" << std::endl;
		}

		// set 3D info flag
		if (vb->Has3DInfo())
			infobyte |= (1<<4);

		// set bytes per sample flag
		if (vb->GetBytesPerSample() != 1)
			infobyte |= (1<<5);

		// NOTE: we don't have to say anything about stereo since at this point, we assume
		//       it's mono!

		sendbuf[len++] = infobyte;

		// fill in 3D info
		if (vb->Has3DInfo())
		{
			sendbuf[len++] = vb->GetInfo3DLength();
			memcpy(sendbuf+len,vb->GetInfo3D(false),vb->GetInfo3DLength());
			len += vb->GetInfo3DLength();
		}

		// set compression info
		sendbuf[len++] = vb->GetMajorCompressionID();
		sendbuf[len++] = vb->GetMinorCompressionID();

		if ((len + vb->GetNumBytes()) > JVOIPRTPTRANS_MAXPACKSIZE)
			return ERR_JVOIPLIB_RTPTRANS_PACKETTOOLARGE;

		memcpy(sendbuf+len,vb->GetSamples(false),vb->GetNumBytes());
		len += vb->GetNumBytes();
        }
	return len;
}

int JVOIPRTPTransmission::GetSampleBlock(VoIPFramework::VoiceBlock *vb)
{
	RTPPacket *pack;
	RTPSourceData *srcdat;
	unsigned long jitter,ssrc;
	ParticipantInfo *partinf;
	VoIPFramework::VOIPdouble offset;
	VoIPFramework::VOIPuint64 packnum;
	std::list<PacknumOffsetPair>::iterator it;
	time_t curtime;

	if (!init)
		return ERR_JVOIPLIB_GENERAL_COMPONENTNOTINIT;

	// Get the necessary data from the RTP session

	pack = rtpsess->GetNextPacket();
	srcdat = rtpsess->GetCurrentSourceInfo();
	ssrc = srcdat->GetSSRC();
	jitter = srcdat->INF_GetJitter();

	// Retrieve participant info

	partinf = GetParticipantInfo(ssrc);
	if (partinf == NULL) // participant NOT found, ignore data
	{
		vb->Clear();
		delete pack;
		return 0;
	}

	// set jitter info
	curtime = time(NULL);
	if (partinf->prevjittertime == 0 || (curtime - partinf->prevjittertime) > JVOIPRTPTRANS_JITTERADJUSTINTERVAL)
	{
		partinf->prevjittertime = curtime;
		partinf->lastknownjitter = jitter;
	}

	// Check if we have already processed this packet

	packnum = 0;
	if (!partinf->isnewsource)
	{
		while (pack && AlreadyProcessedPacket(pack,partinf,&packnum))
		{
			numbytesreceived += pack->GetPayloadLength() + JVOIPRTPTRANS_HEADEROVERHEAD;
			delete pack;

			pack = rtpsess->GetNextPacket();
		}
		if (!pack)
		{
			vb->Clear();
			return 0;
		}
	}

	// Ok, now we have a packet we may process, so we'll process it :-)

	if (!FillInVoiceBlock(vb,pack,partinf))
	{
		vb->Clear();
		delete pack;
		return 0;
	}

	// set info for new participants
	if (partinf->isnewsource)
	{
		partinf->isnewsource = false;
		partinf->lastknownjitter = jitter; // NOTE: this value is set again in case the participant info was cleared AlreadyProcessedPacket
		partinf->samplingrate = vb->GetSampleRate();
		partinf->buffering = (VoIPFramework::VOIPdouble)defaultbuffer;
		partinf->basepacknum = pack->GetExtendedSequenceNumber();
		partinf->basetimestamp = pack->GetTimestamp();
		partinf->basetimeoffset = cursampleoffset;

		vb->SetSampleOffset(cursampleoffset+defaultbuffer);
		
		// store the timestamp unit: this is needed for accurate
		// jitter calculations.
		double tsunit;

		tsunit = 1.0/((double)vb->GetSampleRate());
		srcdat->SetTimestampUnit(tsunit);
	}
	else
	{
		if (!CalculateTimeOffset(vb,pack,partinf,&offset))
		{
			vb->Clear();
			delete pack;
			return 0;
		}
		else
			vb->SetSampleOffset(offset);
	}
	
	// add the packet number-offset pair to the list
	
	it = partinf->receivedpacknums.begin();
	while (it != partinf->receivedpacknums.end() && packnum > (*it).packnum)
		++it;
	
	if (it == partinf->receivedpacknums.end())
		partinf->receivedpacknums.push_back(PacknumOffsetPair(packnum,vb->GetSampleOffset()));
	else
		partinf->receivedpacknums.insert(it,PacknumOffsetPair(packnum,vb->GetSampleOffset()));

	delete pack;

	return 0;
}

bool JVOIPRTPTransmission::AlreadyProcessedPacket(RTPPacket *pack,JVOIPRTPTransmission::ParticipantInfo *partinf,VoIPFramework::VOIPuint64 *packetnumber)
{
	bool found;
	std::list<PacknumOffsetPair>::iterator it;
	VoIPFramework::VOIPuint64 curpacknum,tmppacknum;
	unsigned long extpacknum;

	extpacknum = pack->GetExtendedSequenceNumber() - partinf->basepacknum;
	
	if (extpacknum > partinf->prevpacknum)
	{
		if ((extpacknum - partinf->prevpacknum) > JVOIPRTPTRANS_MAXPACKDIST)
		{
			if ((partinf->prevpacknum - extpacknum) <= JVOIPRTPTRANS_MAXPACKDIST)
			{
				if (partinf->packnumcycles == 0)
				{
					// in this case, we have to ignore the packet!
					return true;
				}
				else
				{
					curpacknum = (partinf->packnumcycles-1);
					curpacknum <<= 32;
					tmppacknum = extpacknum;
					curpacknum |= tmppacknum;
				}
			}
			else
			{
				// packet number out of range
				if ( ++(partinf->packnumoutrangecount) >= JVOIPRTPTRANS_MAXPACKOUTRANGECOUNT)
				{
					// reset connection for this participant and assume this packet
					// is the first one received
					partinf->ClearAllPublicFields();
					*packetnumber = 0;
					return false;
				}
			}
		}
		else
		{
			curpacknum = partinf->packnumcycles;
			curpacknum <<= 32;
			tmppacknum = extpacknum;
			curpacknum |= tmppacknum;
		}
	}
	else if (extpacknum < partinf->prevpacknum)
	{
		if ((extpacknum - partinf->prevpacknum) > JVOIPRTPTRANS_MAXPACKDIST)
		{
			if ((partinf->prevpacknum - extpacknum) <= JVOIPRTPTRANS_MAXPACKDIST)
			{
				curpacknum = partinf->packnumcycles;
				curpacknum <<= 32;
				tmppacknum = extpacknum;
				curpacknum |= tmppacknum;
			}
			else
			{
				// packet number out of range
				if ( ++(partinf->packnumoutrangecount) >= JVOIPRTPTRANS_MAXPACKOUTRANGECOUNT)
				{
					// reset connection for this participant and assume this packet
					// is the first one received
					partinf->ClearAllPublicFields();
					*packetnumber = 0;
					return false;
				}
			}
		}
		else
		{
			partinf->packnumcycles++;
			curpacknum = partinf->packnumcycles;
			curpacknum <<= 32;
			tmppacknum = extpacknum;
			curpacknum |= tmppacknum;
		}
	}
	else // same ext packet number as previous packet number
		return true;

        partinf->prevpacknum = extpacknum;
	*packetnumber = curpacknum;
			
	// search the participant's list for this packet number	
	
	it =  partinf->receivedpacknums.begin();
	found = false;
	while (!found && it != partinf->receivedpacknums.end() && (*it).packnum <= curpacknum)
	{
		if ((*it).packnum == curpacknum)
			found = true;
		else
			++it;
	}

	return found;
}

bool JVOIPRTPTransmission::FillInVoiceBlock(VoIPFramework::VoiceBlock *vb,RTPPacket *pack,ParticipantInfo *partinf)
{
	// set some default values
	vb->Clear();
	vb->SetIsSilence(false);

	if (pack->GetPayloadType() != JVOIPRTPTRANS_JVOIPPAYLOADTYPE)
	{
		// check specific encodings

		unsigned char *data;
		int len;

		len = pack->GetPayloadLength();
		switch(pack->GetPayloadType())
		{
		case JVOIPRTPTRANS_PAYLOADTYPE_PCMU:
			vb->SetSampleRate(8000);
			vb->SetNumSamples(len);
			vb->SetMajorCompressionID(JVOIPCOMPRESSIONTYPE_MAJOR_ULAWCOMPRESSION);
			break;
		case JVOIPRTPTRANS_PAYLOADTYPE_GSM:
			vb->SetSampleRate(8000);
			vb->SetNumSamples(160);
			vb->SetMajorCompressionID(JVOIPCOMPRESSIONTYPE_MAJOR_GSMCOMPRESSION);
			break;
		case JVOIPRTPTRANS_PAYLOADTYPE_LPC:
			vb->SetSampleRate(8000);
			vb->SetNumSamples(160);
			vb->SetMajorCompressionID(JVOIPCOMPRESSIONTYPE_MAJOR_LPCCOMPRESSION);
			break;
		default:
			return false;
		}

		data = new unsigned char[len];
		if (data == NULL) // out of memory
			return false;

		memcpy(data,pack->GetPayloadData(),len);
		vb->SetSamples(data,len);
	}
	else
	{
		unsigned char *data,infobyte,*samples;
		int pos,len;
		bool has3dinfo;

		data = pack->GetPayloadData();
		len = pack->GetPayloadLength();
		if (len <= 0)
			return false;

		pos = 0;

		infobyte = data[pos++];

		// test for 3D info
		if (infobyte & (1<<4))
			has3dinfo = true;
		else
			has3dinfo = false;

		// test bytes per sample
		if (infobyte & (1<<5))
			vb->SetBytesPerSample(2);

		// test samplingrate
		switch(infobyte & 0x0F)
		{
		case 0:
			vb->SetSampleRate(4000);
			break;
		case 1:
			vb->SetSampleRate(8000);
			break;
		case 2:
			vb->SetSampleRate(11025);
			break;
		case 3:
			vb->SetSampleRate(22050);
			break;
		case 4:
			vb->SetSampleRate(44100);
			break;
		default:
			return false;
		}

		// get 3D info if necessary
		if (has3dinfo)
		{
			int info3dlen;
			unsigned char *info3d;

			if (pos >= len)
				return false;

			info3dlen = (int)data[pos++];
			if ((len-pos) < info3dlen)
				return false;

			info3d = new unsigned char[info3dlen];
			if (info3d == NULL) // out of memory
				return false;

			memcpy(info3d,data+pos,info3dlen);
			vb->SetInfo3D(info3d,info3dlen);
			pos += info3dlen;
		}

		if ((len-pos) < 2)
			return false;

		// set compression info
		vb->SetMajorCompressionID((int)data[pos++]);
		vb->SetMinorCompressionID((int)data[pos++]);

		// the rest is the data itself
		if (pos >= len)
			return false;
		
		samples = new unsigned char[len-pos];
		if (samples == NULL)
			return false;
		
		memcpy(samples,data+pos,len-pos);
		vb->SetSamples(samples,len-pos);
	}
	
	if (!partinf->isnewsource && (int)vb->GetSampleRate() != partinf->samplingrate) // sampling rate changed!
	{
		// reset this participants info
		partinf->ClearAllPublicFields();
		return false;
	}
	return true;
}

bool JVOIPRTPTransmission::CalculateTimeOffset(VoIPFramework::VoiceBlock *vb,RTPPacket *pack,ParticipantInfo *partinf,VoIPFramework::VOIPdouble *offset)
{
	VoIPFramework::VOIPuint64 samplenum,tmpnum;
	VoIPFramework::VOIPdouble timeoffset,jitterbuffer,addbuffer;
	unsigned long curtimestamp;
	
	samplenum = 0;
	curtimestamp = pack->GetTimestamp() - partinf->basetimestamp;
	if (curtimestamp > partinf->prevtimestamp)
	{
		if ((curtimestamp - partinf->prevtimestamp) > JVOIPRTPTRANS_MAXTSDIST)
		{
			if ((partinf->prevtimestamp - curtimestamp) <= JVOIPRTPTRANS_MAXTSDIST)
			{
				if (partinf->timestampcycles == 0)
				{
					// in this case, we have to ignore the packet!
					
					return false;
				}
				else
				{
					samplenum = (partinf->timestampcycles-1);
					samplenum <<= 32;
					tmpnum = curtimestamp;
					samplenum |= tmpnum;
				}
			}
			else
			{
				// timestamp out of range
				if ( ++(partinf->timestampoutrangecount) >= JVOIPRTPTRANS_MAXTSOUTRANGECOUNT)
				{
					// reset connection for this participant
					partinf->ClearAllPublicFields();
					return false;
				}
			}
		}
		else
		{
			samplenum = partinf->timestampcycles;
			samplenum <<= 32;
			tmpnum = curtimestamp;
			samplenum |= tmpnum;
		}
	}
	else if (curtimestamp < partinf->prevtimestamp)
	{
		if ((curtimestamp - partinf->prevtimestamp) > JVOIPRTPTRANS_MAXTSDIST)
		{
			if ((partinf->prevtimestamp - curtimestamp) <= JVOIPRTPTRANS_MAXTSDIST)
			{
				samplenum = partinf->timestampcycles;
				samplenum <<= 32;
				tmpnum = curtimestamp;
				samplenum |= tmpnum;
			}
			else
			{
				// timestamp out of range
				if ( ++(partinf->timestampoutrangecount) >= JVOIPRTPTRANS_MAXTSOUTRANGECOUNT)
				{
					// reset connection for this participant
					partinf->ClearAllPublicFields();
					return false;
				}
			}
		}
		else
		{
			partinf->timestampcycles++;
			samplenum = partinf->timestampcycles;
			samplenum <<= 32;
			tmpnum = curtimestamp;
			samplenum |= tmpnum;
		}
	}
	else // we already processed this packet
		return false;
#ifdef WIN32		
	// Visual C++ 6 can't do the conversion from uint64
	timeoffset = VoIPFramework::GetVOIPdouble(samplenum);
#else
	timeoffset = (VoIPFramework::VOIPdouble)samplenum;
#endif // WIN32
	timeoffset /= ((VoIPFramework::VOIPdouble)partinf->samplingrate); // convert to seconds
	timeoffset += partinf->basetimeoffset;

	if (autoadjustbuffering)
	{
		// add buffering to compensate jitter
		jitterbuffer = (VoIPFramework::VOIPdouble)partinf->lastknownjitter;
		jitterbuffer *= 2.0; // buffer a bit more than the actual jitter
		jitterbuffer /= ((VoIPFramework::VOIPdouble)partinf->samplingrate);  // convert to seconds

		addbuffer = jitterbuffer;

		// we must also buffer at least one sample interval, otherwise there might
		// be glitches

		if (addbuffer < interval)
			addbuffer += interval;
	}
	else
		addbuffer = defaultbuffer;

	if (addbuffer < minimbuffer)
		addbuffer = minimbuffer;

	timeoffset += addbuffer;

	// allow packets within a one second delay:
	// if it's large enough, something might still be played back
	// ideally, we should know the amount of samples in a packet, but because the
	// data is possibly compressed, this information is not available at this point

	if (timeoffset < (cursampleoffset - 1.0))
	{
		if ( ++(partinf->timestampoutrangecount) >= JVOIPRTPTRANS_MAXTSOUTRANGECOUNT)
		{
			// reset connection for this participant
			partinf->ClearAllPublicFields();
		}
		return false;
	}
	else if (timeoffset < cursampleoffset)
	{
		// Here, we'll still let the block pass, but we do have to increment
		// the timestampoutrange count. Otherwise, if the timestamp keeps
		// ending up in the 1 second range used above, and the block does
		// not contain enough information (more than 1 sec), it is possible
		// that nothing will be played back from this source: the mixer
		// will discard all packets...

		if ( ++(partinf->timestampoutrangecount) >= JVOIPRTPTRANS_MAXTSOUTRANGECOUNT)
		{
			// reset connection for this participant
			partinf->ClearAllPublicFields();
			return false;
			
		}
	}
	
	*offset = timeoffset;
	return true;
}
