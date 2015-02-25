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

#ifndef JVOIPSOUNDDEVICEBLOCK_H

#define JVOIPSOUNDDEVICEBLOCK_H

#include "jvoipconfig.h"
#include <jmutex.h>
#include <stdlib.h>

class JVOIPSoundDeviceBlock
{
public:
	JVOIPSoundDeviceBlock() { data = NULL; datalen = 0; done = true; flagmutex = NULL; next = NULL; }
	~JVOIPSoundDeviceBlock() { }
	inline bool IsDone();
	
	unsigned char *data;
	int datalen;
private:
	bool done;
	JMutex *flagmutex;

	JVOIPSoundDeviceBlock *next;
	
	friend class JVOIPSoundDevDrvThread;
	friend class JVOIPSoundDevDrvInputThread;
	friend class JVOIPSoundDevDrvOutputThread;
};

inline bool JVOIPSoundDeviceBlock::IsDone()
{
        bool isdone;
	
	if (!flagmutex)
		return true;
	flagmutex->Lock();
	isdone = done;
	flagmutex->Unlock();
	return isdone;
}

#endif // JVOIPSOUNDDEVICEBLOCK_H
