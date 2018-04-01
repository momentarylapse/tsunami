/*
 * CaptureConsoleModeMidi.h
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMIDI_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMIDI_H_

#include "CaptureConsoleMode.h"
#include "../../../lib/base/base.h"

class InputStreamMidi;
class Device;
class Track;
class Synthesizer;
class OutputStream;
class PeakMeter;

class CaptureConsoleModeMidi : public CaptureConsoleMode
{
	InputStreamMidi *input;
	Array<Device*> sources;
	Device *chosen_device;
	Track *target;
	Synthesizer *preview_synth;
	PeakMeter *peak_meter;
	OutputStream *preview_stream;



public:
	CaptureConsoleModeMidi(CaptureConsole *cc);
	void onSource();
	void onTarget();
	void setTarget(Track *t);
	virtual void enterParent();
	virtual void enter();
	virtual void leave();
	virtual void pause();
	virtual void start();
	virtual void stop();
	virtual void dump();
	virtual bool insert();
	virtual int getSampleCount();
	virtual bool isCapturing();
};



#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMIDI_H_ */
