/*
 * CaptureTrackData.h
 *
 *  Created on: Mar 29, 2021
 *      Author: michi
 */

#ifndef SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURETRACKDATA_H_
#define SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURETRACKDATA_H_

#include "../../../lib/base/base.h"
#include "../../../lib/base/optional.h"
#include "../../../lib/pattern/Observable.h"


namespace hui {
	class Panel;
}

namespace tsunami {

class Device;
class AudioInput;
class MidiInput;
class AudioAccumulator;
class MidiAccumulator;
class AudioOutput;
class AudioChannelSelector;
class PeakMeterDisplay;
class PeakMeter;
class Synthesizer;
class Module;
class SignalChain;
enum class SignalType;
class Track;


struct SyncPoint {
	int64 pos_play, pos_record;
	int64 samples_skipped_start;
	int delay(int64 samples_played_before_capture);
};

struct CaptureTrackData : public obs::Node<VirtualBase> {

	SignalType type();

	AudioInput *audio_input();
	MidiInput *midi_input();
	AudioAccumulator *audio_recorder();
	MidiAccumulator *midi_recorder();

	base::optional<int64> samples_played_before_capture;
	int64 samples_skiped_before_capture = 0;
	Array<SyncPoint> sync_points;

	void insert(int pos);
	void insert_audio(int pos, int delay);
	void insert_midi(int pos, int delay);

	void start_sync_before(AudioOutput *out);
	void sync(AudioOutput *out);
	int get_sync_delay();

	Track *track = nullptr;
	Device *get_device();
	bool enabled = false;
	bool allowing_edit = true;
	hui::Panel *panel = nullptr;
	SignalChain *chain = nullptr;

	Module *input = nullptr;
	AudioChannelSelector *channel_selector = nullptr;
	PeakMeterDisplay *peak_meter_display = nullptr;
	PeakMeter *peak_meter = nullptr;
	Module *accumulator = nullptr;
	Module *backup = nullptr;
	Synthesizer *synth = nullptr;
	string id_group, id_grid, id_source, id_active, id_peaks, id_mapper;
	Array<int> channel_map();

	void set_device(Device *dev);
	void set_map(const Array<int> &map);
	void enable(bool enabled);
	void allow_edit(bool allow);
	void accumulate(bool acc);

	void add_into_signal_chain(SignalChain *chain, Device *preferred_device = nullptr);
};

}

#endif /* SRC_VIEW_SIDEBAR_CAPTURECONSOLEMODES_CAPTURETRACKDATA_H_ */
