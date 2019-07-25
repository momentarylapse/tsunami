/*
 * ViewModeCapture.h
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODECAPTURE_H_
#define SRC_VIEW_MODE_VIEWMODECAPTURE_H_

#include "ViewModeDefault.h"
#include "../../Data/Range.h"
#include "../../lib/base/base.h"

class AudioInput;
class MidiInput;
class AudioRecorder;
class MidiRecorder;
class AudioOutput;
class Module;
class SignalChain;
enum class SignalType;
class Track;
class AudioBuffer;
class MidiEventBuffer;

struct SyncPoint {
	int64 pos_play, pos_record;
};

struct CaptureTrackData {
	Track *target;
	Module *recorder;
	Module *input;
	CaptureTrackData();
	CaptureTrackData(Track *target, Module *input, Module *recorder);
	SignalType type();
	AudioInput *audio_input();
	MidiInput *midi_input();
	AudioRecorder *audio_recorder();
	MidiRecorder *midi_recorder();

	int64 samples_played_before_capture = 0;
	int64 samples_recorded_before_start = 0;
	int64 samples_skipped_start = 0;
	Array<SyncPoint> sync_points;

	void insert(int pos);
	void insert_audio(int pos, int delay);
	void insert_midi(int pos, int delay);

	void start_sync_before(AudioOutput *out);
	void start_sync_after();
	void end_sync();
	void sync(AudioOutput *out);
	int get_sync_delay();
};

class ViewModeCapture : public ViewModeDefault {
public:
	ViewModeCapture(AudioView *view);
	virtual ~ViewModeCapture();

	void on_start() override;
	void on_end() override;

	void on_left_button_down() override {}
	void on_left_double_click() override {}

	void draw_post(Painter *c) override;


	virtual Set<Track*> prevent_playback() override;

	Array<CaptureTrackData> data;
	void set_data(const Array<CaptureTrackData> &data);
	SignalChain *chain;
	
	void insert();
};

#endif /* SRC_VIEW_MODE_VIEWMODECAPTURE_H_ */
