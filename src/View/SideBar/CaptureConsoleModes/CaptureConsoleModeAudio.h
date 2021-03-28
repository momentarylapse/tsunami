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
	Array<Device*> sources;

public:
	CaptureConsoleModeAudio(CaptureConsole *_cc);
	void on_source();
	void set_target(Track *t);
	void enter() override;
	void leave() override;
	void allow_change_device(bool allow) override;

	void update_device_list();
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEAUDIO_H_ */
