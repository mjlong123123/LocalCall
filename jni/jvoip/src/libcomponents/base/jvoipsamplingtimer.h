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

#ifndef JVOIPSAMPLINGTIMER_H

#define JVOIPSAMPLINGTIMER_H

#include "jvoipconfig.h"
#include "samplingtimer.h"

class JVOIPSignalWait;

// NOTE: the destruction of JVOIPSamplingTimer must also cause the destruction of the
//       associated JVOIPSignalWait

class JVOIPSamplingTimer : public VoIPFramework::SamplingTimer
{
public:
	JVOIPSamplingTimer() { }
	virtual ~JVOIPSamplingTimer() { }
	virtual JVOIPSignalWait *GetSignalWaiter() = 0;
};

#endif // JVOIPSAMPLINGTIMER_H
