#ifndef _CUSTOM_RTP_SESSION
#define _CUSTOM_RTP_SESSION

#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpsourcedata.h"

class JavaRtp;

class CustomRTPSession : public RTPSession
{
public:
    CustomRTPSession(JavaRtp * jrtp)
    {
        mJavaRtp = jrtp;
    };
    CustomRTPSession() {};
    ~CustomRTPSession()
    {
        mJavaRtp = NULL;
    };

protected:
    void OnPollThreadStep();
    void ProcessRTPPacket(const RTPSourceData &srcdat,const RTPPacket &rtppack);
    JavaRtp * mJavaRtp;

};

#endif
