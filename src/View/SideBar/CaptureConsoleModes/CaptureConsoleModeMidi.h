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
class MidiRecorder;

class CaptureConsoleModeMidi : public CaptureConsoleMode
{
	InputStreamMidi *input;
	Array<Device*> sources;
	Device *chosen_device;
	const Track *target;
	Synthesizer *preview_synth;
	PeakMeter *peak_meter;
	OutputStream *preview_stream;
	MidiRecorder *recorder;



public:
	CaptureConsoleModeMidi(CaptureConsole *cc);
	void on_source();
	void on_target();
	void set_target(const Track *t);
	void enter() override;
	void leave() override;
	void allow_change_device(bool allow) override;
};



#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMIDI_H_ */
