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
	void on_source();
	void on_target();
	void set_target(Track *t);
	virtual void enter_parent() override;
	virtual void enter() override;
	virtual void leave() override;
	virtual void pause() override;
	virtual void start() override;
	virtual void stop() override;
	virtual void dump() override;
	virtual bool insert() override;
	virtual int get_sample_count() override;
};



#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMIDI_H_ */
