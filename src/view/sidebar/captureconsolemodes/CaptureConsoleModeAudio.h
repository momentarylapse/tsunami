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

namespace tsunami {

class AudioInput;
class PeakMeter;
class Device;
class Track;
class SignalChain;

class CaptureConsoleModeAudio : public CaptureConsoleMode {
public:
	explicit CaptureConsoleModeAudio(CaptureConsole *_cc);

	void set_target(Track *t);
	void enter() override;
	void allow_change_device(bool allow) override;
};

}

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEAUDIO_H_ */
