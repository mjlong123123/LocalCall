#include "jvoipsession.h"
#include "jvoiprtptransmission.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

void check(int s)
{
	if (s >= 0)
		return;
	printf("Error %d\n",s);
	exit(-1);
}

int main(int argc,char *argv[])
{
	JVOIPSession sess;
	JVOIPSessionParams params;
	JVOIPRTPTransmissionParams rtpparams;
	time_t prevtime,curtime;
	char dst[1024];
	int i,ratenum;
	bool done = false;
	int rates[]={4000,8000,11025,22050,44100};

	printf("Enter destination IP:\n");
	scanf("%s",dst);
	rtpparams.SetAcceptOwnPackets(true);
	params.SetTransmissionParams(&rtpparams);
	params.SetOutputSamplingRate(44100);
	params.SetInputSampleEncodingType(JVOIPSessionParams::SixteenBit);
	params.SetOutputSampleEncodingType(JVOIPSessionParams::SixteenBit);
//	params.SetCompressionType(JVOIPSessionParams::DPCM);
	check(sess.Create(params));
	check(sess.AddDestination(ntohl(inet_addr(dst)),5000));

	ratenum = 1;
	params.SetInputSamplingRate(rates[ratenum]);
	while(!done)
	{
		int c;

		c = fgetc(stdin);
		if (c == '+' && ratenum < 4)
		{
			ratenum++;
			printf("Setting rate to %d\n",rates[ratenum]);
			sess.SetInputSamplingRate(rates[ratenum]);
                }
		else if (c == '-' && ratenum > 0)
		{
			ratenum--;
			printf("Setting rate to %d\n",rates[ratenum]);
			sess.SetInputSamplingRate(rates[ratenum]);
                }
		else if (c == 'q')
			done = true;
	}

	// IMPORTANT: The Destroy function MUST be called! Refer to the manual for more info...
	check(sess.Destroy());
	
	printf("Session destroyed...\n");
	return 0;
}
