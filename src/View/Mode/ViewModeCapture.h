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

class Device;
class AudioInput;
class MidiInput;
class AudioAccumulator;
class MidiAccumulator;
class AudioOutput;
class Module;
class SignalChain;
enum class SignalType;
class Track;
class AudioBuffer;
class MidiEventBuffer;

struct SyncPoint {
	int64 pos_play, pos_record;
	int64 samples_skipped_start;
	int delay(int64 samples_played_before_capture);
};



struct CaptureInputData {
	SignalType type;
	Device *device;
	Module *recorder;
	Module *input;
	CaptureInputData();
	CaptureInputData(SignalType type, Module *input, Module *recorder);
	AudioInput *audio_input();
	MidiInput *midi_input();
	AudioAccumulator *audio_recorder();
	MidiAccumulator *midi_recorder();

	int64 samples_played_before_capture = 0;
	Array<SyncPoint> sync_points;

	void start_sync_before(AudioOutput *out);
	void sync(AudioOutput *out);
	int get_sync_delay();
};

struct CaptureTrackData {
	Track *target;
	Module *recorder;
	Module *input;
	Array<int> channel_map;
	CaptureTrackData();
	CaptureTrackData(Track *target, Module *input, Module *recorder);
	SignalType type();
	AudioInput *audio_input();
	MidiInput *midi_input();
	AudioAccumulator *audio_recorder();
	MidiAccumulator *midi_recorder();

	int64 samples_played_before_capture = 0;
	int64 samples_skiped_before_capture = 0;
	Array<SyncPoint> sync_points;

	void insert(int pos);
	void insert_audio(int pos, int delay);
	void insert_midi(int pos, int delay);

	void start_sync_before(AudioOutput *out);
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

	Array<CaptureInputData> inputs;
	Array<CaptureTrackData> data;
	void set_data(const Array<CaptureTrackData> &data);
	SignalChain *chain;
	
	void insert();
};

#endif /* SRC_VIEW_MODE_VIEWMODECAPTURE_H_ */
