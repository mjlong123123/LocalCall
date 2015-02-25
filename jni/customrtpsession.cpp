
#include <jni.h>
#include <android/log.h>
#include "customrtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtperrors.h"
#include "customrtp.h"

const char* const LOG_TAG = "RTP_SESSION";

void CustomRTPSession::OnPollThreadStep()
{
    BeginDataAccess();

    // check incoming packets
    if (GotoFirstSourceWithData())
    {
        do
        {
            RTPPacket *pack;
            RTPSourceData *srcdat;

            srcdat = GetCurrentSourceInfo();

            while ((pack = GetNextPacket()) != NULL)
            {
                ProcessRTPPacket(*srcdat,*pack);
                DeletePacket(pack);
            }
        }
        while (GotoNextSourceWithData());
    }

    EndDataAccess();
}

void CustomRTPSession::ProcessRTPPacket(const RTPSourceData &srcdat,const RTPPacket &rtppack)
{
    // You can inspect the packet and the source's info here
    uint32_t ip;
    uint16_t port;
    uint8_t  * buffer = NULL;
    short buffer11[160];
    size_t size = 0;
    int i = 0;
    uint8_t type = 0;
    uint32_t Sequencenumber = 0;
    uint8_t temp8 = 0;
    uint16_t temp16 = 0;
    uint32_t temp32 = 0;
    uint32_t count = 0;
    struct in_addr inaddr;
    jclass tempclass;
    jmethodID tempmethodid;
    jstring msg;
    jbyteArray buffer1;

    JNIEnv *env;
    int ree = 0;
    jshortArray bufret;


    if (srcdat.GetRTPDataAddress() != 0)
    {
        const RTPIPv4Address *addr = (const RTPIPv4Address *)(srcdat.GetRTPDataAddress());
        ip = addr->GetIP();
        port = addr->GetPort();
    }
    else if (srcdat.GetRTCPDataAddress() != 0)
    {
        const RTPIPv4Address *addr = (const RTPIPv4Address *)(srcdat.GetRTCPDataAddress());
        ip = addr->GetIP();
        port = addr->GetPort()-1;
    }

    inaddr.s_addr = htonl(ip);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket ip %s", inet_ntoa(inaddr));
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket port %d", port);
    buffer = rtppack.GetPayloadData();
    size = rtppack.GetPayloadLength();
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "read data from rtp size : %d",size);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "read data from rtp buffer[0] : %d",buffer[0]);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "read data from rtp buffer[1] : %d",buffer[1]);


    if(mJavaRtp != NULL)
    {
        CustomSpeex * speex;
        speex = mJavaRtp->getCustomSpeex();
        if(speex != NULL)
        {
            ree = speex->decode((char *)buffer, buffer11, speex->getFrameSize());

        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "read data from rtp decode ree : %d",ree);
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "read data from rtp decode buffer11[0] : %d",buffer11[0]);
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "read data from rtp decode buffer11[1] : %d",buffer11[1]);
		
            mJavaRtp->callJavaCallBack(0,buffer11,speex->getFrameSize());
        }
		
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "speex : %d",(int)speex);
    }
    else
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "mJavaRtp is null");
    }
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket data end");

    type = rtppack.GetPayloadType();
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket type %d",type);
    if(rtppack.HasExtension())
    {
        Sequencenumber = rtppack.GetExtendedSequenceNumber();
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket SequenceNumber %d",Sequencenumber);
        temp16 = rtppack.GetExtensionID();
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket ExtensionID %d",temp16);
        size = rtppack.GetExtensionLength();
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket ExtensionLength %d",size);
        buffer = rtppack.GetExtensionData();
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket dataext start");
        for(i = 0; i < size && buffer != NULL; i++)
        {
            __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket Extension buffer[%d] %d",i, buffer[i]);
        }
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket dataext end");
    }
    temp32 = rtppack.GetSSRC();
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket GetSSRC  %d",temp32);
    count =     rtppack.GetCSRCCount();
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket GetCSRCCount  %d",count);
    for(i = 0; i < count; i++)
    {
        temp32 = rtppack.GetCSRC(i);
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket GetCSRC[%d]  %d",i,temp32);
    }
    temp32 = rtppack.GetTimestamp();
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "ProcessRTPPacket GetTimestamp  %d",temp32);
}

