#include "jvoipsession.h"
#include "jvoiprtptransmission.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(void)
{
	JVOIPSession session;
	JVOIPSessionParams params;
	JVOIPRTPTransmissionParams rtpparams;
	unsigned long ip;

	ip = inet_addr("127.0.0.1");
	ip = ntohl(ip);

	rtpparams.SetAcceptOwnPackets(true);
	params.SetTransmissionParams(&rtpparams);

	session.Create(params);
	session.AddDestination(ip,5000);
	fgetc(stdin);

	
	// IMPORTANT: The Destroy method MUST be called. Consult the manual...
	session.Destroy();

	return 0;
}
