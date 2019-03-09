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
class PeakMeter;
class AudioRecorder;
class Device;
class Track;
class SignalChain;

class CaptureConsoleModeAudio : public CaptureConsoleMode
{
	InputStreamAudio *input;
	PeakMeter *peak_meter;
	AudioRecorder *recorder;
	Array<Device*> sources;
	Device *chosen_device;
	Track *target;

public:
	CaptureConsoleModeAudio(CaptureConsole *_cc);
	void on_source();
	void set_target(Track *t);
	void enter_parent() override;
	void enter() override;
	void leave() override;
	void pause() override;
	void start() override;
	void stop() override;
	void dump() override;
	bool insert() override;
	int get_sample_count() override;
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEAUDIO_H_ */
