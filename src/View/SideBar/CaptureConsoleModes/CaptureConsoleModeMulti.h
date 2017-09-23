/*
 * CaptureConsoleModeMulti.h
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_

#include "CaptureConsoleMode.h"
#include "../../../lib/base/base.h"

class Device;

class CaptureConsoleModeMulti: public CaptureConsoleMode
{
	Array<Device*> sources_audio;
	Array<Device*> sources_midi;

	int size;


public:
	CaptureConsoleModeMulti(CaptureConsole *cc);
	virtual ~CaptureConsoleModeMulti();
	virtual void enterParent();
	virtual void leaveParent();
	virtual void enter(){}
	virtual void leave(){}
	virtual void pause(){}
	virtual void start(){}
	virtual void stop(){}
	virtual void dump(){}
	virtual bool insert(){ return false; }
	virtual int getSampleCount(){ return 0; }
	virtual bool isCapturing(){ return false; }
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_ */
