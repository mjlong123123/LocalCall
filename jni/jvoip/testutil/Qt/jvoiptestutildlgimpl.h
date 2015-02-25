/*

  This file is a part of the JVOIPLIB Test Utility
  Copyright (c) 2000-2005 Jori Liesenborgs

  Contact: jori@lumumba.uhasselt.be

  This file was developed at the 'Expertise Centre for Digital  Media' (EDM) 
  in Diepenbeek, Belgium (http://www.edm.uhasselt.be). The EDM is a research 
  institute of the Hasselt University (http://www.uhasselt.be).

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef JVOIPTESTUTILDLGIMPL_H

#define JVOIPTESTUTILDLGIMPL_H

#include "jvoiptestutildlg.h"
#include "jvoipsession.h"
#include "jvoipsessionparams.h"

class QEvent;

class JVOIPTestUtilDlgImpl : public JVOIPTestUtilDlg,JVOIPSession
{
	Q_OBJECT
public:
	JVOIPTestUtilDlgImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
	~JVOIPTestUtilDlgImpl() throw (JVOIPException);
private slots:
	void onCreate();
	void onDestroy();
	void onChangeInputRate(int index);
	void onChangeOutputRate(int index);
	void onChangeInputEncoding(int index);
	void onChangeOutputEncoding(int index);
	void onChangeInputType(int index);
	void onChangeOutputType(int index);
	void onChangeCompressionType(int index);
	void onChangeSamplingInterval();
	void onChangePortBase();
	void onChangeDestinationIP();
	void onChangeDestinationPort();
	void onChangeInputBoost(int index);
	void onChangeOutputBoost(int index);
private:
	void loadSettings();
	void saveSettings();
	void printLog(const char line[]);
	void getSaveFileName(char filename[]);
	bool handleError(int err);
	void updateComponentInfo();
	void addComponentInfo(JVOIPSession::ComponentType comptype);	
	bool event(QEvent *evt);
	
	void ThreadFinishedHandler(int threaderr,int voicecallerr = 0,int componenterr = 0);

	static int samprates[];
	static int boosts[];
	static JVOIPSessionParams::SampleEncodingType enctypes[];
	static JVOIPSessionParams::VoiceInputType inputtypes[];
	static JVOIPSessionParams::VoiceOutputType outputtypes[];
	static JVOIPSessionParams::CompressionType comptypes[];

	unsigned long destip;
	int destport;
};

#endif // JVOIPTESTUTILDLGIMPL_H


