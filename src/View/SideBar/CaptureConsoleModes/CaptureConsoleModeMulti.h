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
class InputStreamAudio;
class InputStreamMidi;
class AudioRecorder;
class MidiRecorder;
class PeakMeter;
class PeakMeterDisplay;

class CaptureConsoleModeMulti: public CaptureConsoleMode
{
	Array<Device*> sources_audio;
	Array<Device*> sources_midi;

	struct CaptureItem
	{
		Track *track;
		InputStreamAudio *input_audio;
		AudioRecorder *recorder_audio;
		MidiRecorder *recorder_midi;
		InputStreamMidi *input_midi;
		PeakMeterDisplay *peak_meter_display;
		PeakMeter *peak_meter;
		Device *device;
		string id_source, id_target, id_type, id_peaks;
	};
	Array<CaptureItem> items;


public:
	CaptureConsoleModeMulti(CaptureConsole *cc);
	virtual ~CaptureConsoleModeMulti();
	//virtual void enter_parent();
	//virtual void leave_parent();
	void enter() override;
	void leave() override;
	void pause() override;
	void start() override;
	void stop() override;
	void dump() override;
	bool insert() override;
	int get_sample_count() override;
	bool is_capturing() override;

	void on_source();
};

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURECONSOLEMODEMULTI_H_ */
