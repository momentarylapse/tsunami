/*
 * CaptureTrackData.cpp
 *
 *  Created on: Mar 29, 2021
 *      Author: michi
 */

#include "CaptureTrackData.h"
#include "../../../Data/base.h"
#include "../../../Data/Track.h"
#include "../../../Data/Song.h"
#include "../../../Data/TrackLayer.h"
#include "../../../Module/SignalChain.h"
#include "../../../Session.h"
#include "../../../Device/Stream/AudioInput.h"
#include "../../../Device/Stream/MidiInput.h"
#include "../../../Device/Stream/AudioOutput.h"
#include "../../../Module/Audio/AudioAccumulator.h"
#include "../../../Module/Midi/MidiAccumulator.h"




CaptureTrackData::CaptureTrackData() : CaptureTrackData(nullptr, nullptr, nullptr) {}
CaptureTrackData::CaptureTrackData(Track *_target, Module *_input, Module *_recorder) {
	target = _target;
	input = _input;
	recorder = _recorder;
}

SignalType CaptureTrackData::type() {
	return target->type;
}

AudioInput *CaptureTrackData::audio_input() {
	return (AudioInput*)input;
}

MidiInput *CaptureTrackData::midi_input() {
	return (MidiInput*)input;
}

AudioAccumulator *CaptureTrackData::audio_recorder() {
	return (AudioAccumulator*)recorder;
}

MidiAccumulator *CaptureTrackData::midi_recorder() {
	return (MidiAccumulator*)recorder;
}

void CaptureTrackData::insert_midi(int s_start, int delay) {

	int i0 = s_start + delay;

	// insert data
	target->layers[0]->insert_midi_data(i0, midi_events_to_notes(midi_recorder()->buffer));
}


bool layer_available(TrackLayer *l, const Range &r) {
	for (auto &b: l->buffers)
		if (b.range().overlaps(r))
			return false;
	return true;
}

TrackLayer *find_or_create_available_layer(Track *target, const Range &r) {
	for (auto *l: weak(target->layers))
		if (layer_available(l, r))
			return l;
	return target->add_layer();
}

void CaptureTrackData::insert_audio(int s_start, int delay) {
	Song *song = target->song;

	int i0 = s_start + delay;

	AudioBuffer &buf = audio_recorder()->buf;

	// insert data
	Range r = Range(i0, buf.length);
	song->begin_action_group();

	TrackLayer *layer = find_or_create_available_layer(target, r);

	AudioBuffer tbuf;
	auto *a = layer->edit_buffers(tbuf, r);

	/*if (hui::Config.getInt("Input.Mode", 0) == 1)
		tbuf.add(buf, 0, 1.0f, 0);
	else*/
		tbuf.set(buf, 0, 1.0f);
	layer->edit_buffers_finish(a);
	song->end_action_group();
}

void CaptureTrackData::insert(int pos) {
	int delay = get_sync_delay();
	target->song->session->debug("input", format("latency: %d samples", delay));
	if (type() == SignalType::AUDIO) {
		insert_audio(pos, delay);
	} else if (type() == SignalType::MIDI) {
		insert_midi(pos, delay);
	}
}

void CaptureTrackData::start_sync_before(AudioOutput *out) {
	sync_points.clear();
	samples_played_before_capture = out->samples_played();
	samples_skiped_before_capture = audio_recorder()->samples_skipped;
}

void CaptureTrackData::sync(AudioOutput *out) {
	if (type() == SignalType::AUDIO) {
		SyncPoint p;
		p.pos_play = out->samples_played();
		p.pos_record = audio_input()->samples_recorded();
		p.samples_skipped_start = audio_recorder()->samples_skipped;
		sync_points.add(p);
	}
}

int SyncPoint::delay(int64 samples_played_before_capture) {
	return (pos_play - samples_played_before_capture) - (pos_record - samples_skipped_start);
}

int CaptureTrackData::get_sync_delay() {
	if (sync_points.num == 0)
		return 0;
	int d = 0;
	for (auto &p: sync_points)
		d += p.delay(samples_played_before_capture);
	d /= sync_points.num;
	/*if (fabs(d) > 50000)
		return 0;*/
	return d;
}


