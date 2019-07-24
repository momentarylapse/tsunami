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

class AudioInput;
class PeakMeter;
class Device;
class Track;
class SignalChain;

class CaptureConsoleModeAudio : public CaptureConsoleMode {
public:
	AudioInput *input;
	PeakMeter *peak_meter;
	Array<Device*> sources;
	Track *target;

public:
	CaptureConsoleModeAudio(CaptureConsole *_cc);
	void on_source();
	void set_target(Track *t);
	void enter() override;
	void leave() override;
	void allow_change_device(bool allow) override;

	void update_device_list();

	void start_sync_before() override;
	void start_sync_after() override;
	void sync() override;
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEAUDIO_H_ */
