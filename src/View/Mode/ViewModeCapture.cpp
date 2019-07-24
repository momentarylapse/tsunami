/*
 * ViewModeCapture.cpp
 *
 *  Created on: 14.02.2016
 *      Author: michi
 */

#include "ViewModeCapture.h"
#include "../AudioView.h"
#include "../Node/AudioViewLayer.h"
#include "../Painter/BufferPainter.h"
#include "../../Data/base.h"
#include "../../Data/Track.h"
#include "../../Data/Song.h"
#include "../../Data/TrackLayer.h"
#include "../../Module/Audio/AudioRecorder.h"
#include "../../Module/Midi/MidiRecorder.h"
#include "../../Module/SignalChain.h"
#include "../SideBar/SideBar.h"
#include "../../Session.h"
#include "../../Device/Stream/AudioInput.h"
#include "../../Device/Stream/MidiInput.h"
#include "../../Device/Stream/AudioOutput.h"

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

AudioRecorder *CaptureTrackData::audio_recorder() {
	return (AudioRecorder*)recorder;
}

MidiRecorder *CaptureTrackData::midi_recorder() {
	return (MidiRecorder*)recorder;
}

void CaptureTrackData::insert_midi(int s_start, int delay) {

	int i0 = s_start + delay;

	// insert data
	target->layers[0]->insert_midi_data(i0, midi_events_to_notes(midi_recorder()->buffer).duplicate());
}


bool layer_available(TrackLayer *l, const Range &r) {
	for (auto &b: l->buffers)
		if (b.range().overlaps(r))
			return false;
	return true;
}

void CaptureTrackData::insert_audio(int s_start, int delay) {
	Song *song = target->song;

	int i0 = s_start + delay;

	AudioBuffer &buf = audio_recorder()->buf;

	// insert data
	Range r = Range(i0, buf.length);
	song->begin_action_group();

	TrackLayer *layer = nullptr;
	for (auto *l: target->layers)
		if (layer_available(l, r)) {
			layer = l;
			break;
		}
	if (!layer)
		layer = target->add_layer();

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
	samples_played_before_capture = out->samples_played();
}

void CaptureTrackData::start_sync_after() {
	if (type() == SignalType::AUDIO) {
		samples_recorded_before_start = audio_input()->samples_recorded();
		samples_skipped_start = audio_recorder()->samples_skipped;
	}
}

void CaptureTrackData::sync(AudioOutput *out) {
	if (type() == SignalType::AUDIO) {
		SyncPoint p;
		p.pos_play = out->samples_played();
		p.pos_record = audio_input()->samples_recorded();
		sync_points.add(p);
	}
}

int CaptureTrackData::get_sync_delay() {
	if (sync_points.num == 0)
		return 0;
	int d = 0;
	for (auto &p: sync_points)
		d += (p.pos_play - samples_played_before_capture) - (p.pos_record - samples_skipped_start);
	return d / sync_points.num;
}


ViewModeCapture::ViewModeCapture(AudioView *view) :
	ViewModeDefault(view)
{
	side_bar_console = SideBar::CAPTURE_CONSOLE;
	chain = nullptr;
}

ViewModeCapture::~ViewModeCapture() {
	set_data({});
}

void ViewModeCapture::on_start() {
}

void ViewModeCapture::on_end() {
}

void ViewModeCapture::draw_post(Painter *c) {
	// capturing preview
	
	if (!chain)
		return;

	int offset = view->get_playback_selection(true).offset;
	for (auto &d: data) {
		auto *l = view->get_layer(d.target->layers[0]);
		if (d.type() == SignalType::AUDIO) {
			auto *rec = d.audio_recorder();

			view->buffer_painter->set_context(l->area);
			view->buffer_painter->set_color(view->colors.capture_marker);

			std::lock_guard<std::mutex> lock(rec->mtx_buf);
			view->update_peaks_now(rec->buf);
			view->buffer_painter->draw_buffer(c, rec->buf, offset);
		} else if (d.type() == SignalType::MIDI) {
			auto *rec = d.midi_recorder();
			std::lock_guard<std::mutex> lock(rec->mtx_buf);
			l->draw_midi(c, midi_events_to_notes(rec->buffer), true, offset);
		}
	}
	
	int l = chain->command(ModuleCommand::ACCUMULATION_GET_SIZE, 0);
	view->draw_time_line(c, offset + l, view->colors.capture_marker, false, true);
}

Set<Track*> ViewModeCapture::prevent_playback() {
	Set<Track*> prev;
	for (auto &d: data)
		prev.add(d.target);
	return prev;
}

void ViewModeCapture::set_data(const Array<CaptureTrackData> &_data) {
	data = _data;
}


void ViewModeCapture::insert() {
	song->begin_action_group();
	for (auto &d: data)
		d.insert(view->get_playback_selection(true).start());
	song->end_action_group();
}
