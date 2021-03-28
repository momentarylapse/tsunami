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
class Track;
class AudioInput;
class MidiInput;
class AudioRecorder;
class MidiRecorder;
class PeakMeter;
class PeakMeterDisplay;

class CaptureConsoleModeMulti: public CaptureConsoleMode {
	Array<Device*> sources_audio;
	Array<Device*> sources_midi;


public:
	CaptureConsoleModeMulti(CaptureConsole *cc);
	void enter() override;
	void leave() override;
	void allow_change_device(bool allow) override;

	void on_source();
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_ */
