#include "jvoipsession.h"
#include "jvoiprtptransmission.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>

int step = 0;

class Session3D : public JVOIPSession
{
public:
	Session3D() { radius = 0.5; angle = 60; away = true; }
private:
	bool RetrieveOwnPosition(double *xpos,double *ypos,double *zpos,
	                         double *righteardir_x,double *righteardir_y,double *righteardir_z,
				 double *front_x,double *front_y,double *front_z,
				 double *up_x,double *up_y,double *up_z)
	{
		*xpos = 0.0;
		*ypos = 0.0;
		*zpos = 0.0;
		*righteardir_x = 1.0;
		*righteardir_y = 0.0;
		*righteardir_z = 0.0;
		return true;
	}

	bool EncodeOwnPosition(unsigned char encodebuffer[JVOIP_3DINFOBUFFERLEN], int *len)
	{
		encodebuffer[0] = (unsigned char)((angle>>8)&255);
		encodebuffer[1] = (unsigned char)(angle&255);
		*len = 2;
		angle += 1;
		if (angle >= 360)
			angle = 0;
		return true;
	}

	bool DecodePositionalInfo(unsigned char buffer[], int len, double *xpos,double *ypos,double *zpos)
	{
		int ang;
		double radang;

/*		if (away)
		{
			radius += 0.02;
			if (radius > 2)
				away = false;
		}
		else
		{
			radius -= 0.02;
			if (radius < 0.10)
				away = true;
		}
*/
		ang = (((int)buffer[0])<<8)|((int)buffer[1]);
		radang = (3.1415926536/180.0)*((double)ang);
		*xpos = cos(radang)*(double)radius;
		*ypos = sin(radang)*(double)radius;
		*zpos = 0;

		return true;
	}

	double radius;
	int angle;
	bool away;
};

int main(void)
{
	Session3D sess;
	JVOIPSessionParams sp;
	JVOIPRTPTransmissionParams rp;

	rp.SetAcceptOwnPackets(true);
	rp.SetLocalIP(ntohl(inet_addr("127.0.0.1")));
	sp.SetInputSamplingRate(44100);
	sp.SetOutputSamplingRate(44100);
	sp.SetInputSampleEncodingType(JVOIPSessionParams::SixteenBit);
	sp.SetOutputSampleEncodingType(JVOIPSessionParams::SixteenBit);
	sp.SetLocalisationType(JVOIPSessionParams::HRTFLocalisation);
	sp.SetTransmissionParams(&rp);

	sess.Create(sp);
	sess.AddDestination(ntohl(inet_addr("127.0.0.1")),5000);
	fgetc(stdin);

	// IMPORTANT: The destroy function MUST be called. Refer to the manual...
	sess.Destroy();

	return 0;
}
