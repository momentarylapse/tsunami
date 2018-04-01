/*
 * CaptureConsoleModeAudio.h
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEAUDIO_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEAUDIO_H_

#include "CaptureConsoleMode.h"
#include "../../../lib/base/base.h"

class InputStreamAudio;
class AudioSucker;
class PeakMeter;
class Device;
class Track;

class CaptureConsoleModeAudio : public CaptureConsoleMode
{
	InputStreamAudio *input;
	PeakMeter *peak_meter;
	AudioSucker *sucker;
	Array<Device*> sources;
	Device *chosen_device;
	Track *target;

public:
	CaptureConsoleModeAudio(CaptureConsole *_cc);
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

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEAUDIO_H_ */
