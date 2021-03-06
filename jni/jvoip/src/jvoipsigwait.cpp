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
#include "jvoipsigwait.h"
#include "jvoiperrors.h"

#ifdef WIN32
	#include <string.h>
	#include <fcntl.h>
	#include <io.h>
#else // unix like functions
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
#endif // WIN32

#include "debugnew.h"

JVOIPSignalWait::JVOIPSignalWait()
{
	init = false;
}

JVOIPSignalWait::~JVOIPSignalWait()
{
	if (!init)
		return;
#ifndef WIN32
	close(sigpipe[0]);
	close(sigpipe[1]);
#else
	CloseHandle(readhandle);
	CloseHandle(writehandle);
#endif // WIN32
}

int JVOIPSignalWait::Init()
{
	if (init)
		return ERR_JVOIPLIB_SIGWAIT_ALREADYINIT;
	if (!countmutex.IsInitialized() && countmutex.Init() < 0)
		return ERR_JVOIPLIB_GENERAL_CANTINITMUTEX;
	if (!waitmutex.IsInitialized() && waitmutex.Init() < 0)
		return ERR_JVOIPLIB_GENERAL_CANTINITMUTEX;
#ifdef WIN32
	SECURITY_ATTRIBUTES attr;

	attr.nLength = sizeof(SECURITY_ATTRIBUTES);
	attr.lpSecurityDescriptor = NULL;
	attr.bInheritHandle = TRUE;

	if (!CreatePipe(&readhandle,&writehandle,&attr,4096))
		return ERR_JVOIPLIB_SIGWAIT_CANTCREATEPIPE;
#else // unix like function
	if (pipe(sigpipe) == -1)
		return ERR_JVOIPLIB_SIGWAIT_CANTCREATEPIPE;
#endif // WIN32
	init = true;
	iswaiting = false;
	count = 0;
	return 0;
}

int JVOIPSignalWait::WaitForSignal()
{
	if (!init)
		return ERR_JVOIPLIB_SIGWAIT_NOTINIT;
	
	countmutex.Lock();
	
	if (count > 0)
	{
		count--;
		countmutex.Unlock();
		return 0;
	}
	waitmutex.Lock();
	countmutex.Unlock();
	iswaiting = true;
	waitmutex.Unlock();

#ifndef WIN32
	read(sigpipe[0],dummybuf,1);
#else
	DWORD numread;

	ReadFile(readhandle,dummybuf,1,&numread,NULL);
#endif // WIN32
	return 0;
}

int JVOIPSignalWait::Signal()
{
	if (!init)
		return ERR_JVOIPLIB_SIGWAIT_NOTINIT;

	countmutex.Lock();
	waitmutex.Lock();
	if (iswaiting)
	{
#ifndef WIN32
		write(sigpipe[1],"*",1);
#else
		DWORD numwritten;

		WriteFile(writehandle,"*",1,&numwritten,NULL);
#endif // WIN32
		iswaiting = false;
	}
	else
		count++;
	
	waitmutex.Unlock();
	countmutex.Unlock();

	return 0;
}

int JVOIPSignalWait::ClearSignalBuffers()
{
	if (!init)
		return ERR_JVOIPLIB_SIGWAIT_NOTINIT;
	
	countmutex.Lock();
	waitmutex.Lock();
	if (iswaiting)
	{
#ifndef WIN32
		write(sigpipe[1],"*",1);
#else
		DWORD numwritten;

		WriteFile(writehandle,"*",1,&numwritten,NULL);
#endif // WIN32
	}
	count = 0;
	iswaiting = false;
	waitmutex.Unlock();
	countmutex.Unlock();
	return 0;
}
