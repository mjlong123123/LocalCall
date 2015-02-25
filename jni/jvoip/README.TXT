------------------------------------------------------------------------------
                                                              October 20, 2005


                        Notes about JVOIPLIB (v1.4.1)



LIBRARY LOCATION AND CONTACT
============================

Normally, you should be able to download the latest version of the library
from this url:
	http://research.edm.uhasselt.be/jori/jvoiplib/jvoiplib.html

If you have questions about the library, you can mail me at:
	jori@lumumba.uhasselt.be


DISCLAIMER & COPYRIGHT
======================

For the full copyright statement and disclaimer, please refer to the file
LICENSE.LGPL which is included in the source code archive of the library.
In short, it's this:

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
	     
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

INSTALLATION NOTES
==================

* To compile the library on a unix-like platform, just start the configure
  script. Afterwards you just need to run 'make'. Note that you also need
  jthread-1.1.2 and jrtplib-3.3.0 to make the library work. To disable
  components, you can use the --disable-X flags of the configure script.
  
* To compile the library on a MS-Windows platform using Visual C++, you can
  use the project files included in the archive. You will have to change
  the path to the jthread-1.1.2 and jrtplib-3.3.0 headers in the project 
  settings. If you plan to use JVOIPLIB in an MFC application, you should 
  build the library with the option 'using MFC in a shared library'.
  Otherwise, you can build the library without MFC support. To disable
  certain components, you can adjust some lines in src/jvoipconfig_win.h
  before compiling the library. Also note that jrtplib has to be compiled 
  with JThread support enabled. 
 
ACKNOWLEDGEMENTS
================

Many thanks go to Bill Gardner and Keith Martin: thanks to their measurements
of Head-Related Impulse Responses, I was able to make a decent 3D sound 
implementation.

I would also like to thank Jutta Degener and Carsten Bormann for writing
their GSM codec.

Thanks also go to Ron Frederick for writing his LPC implementation.

David Allouche found a serious bug in the library. Many thanks for pointing
this out and suggesting possible workarounds.

