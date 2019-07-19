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

CaptureTrackData::CaptureTrackData() : CaptureTrackData(nullptr, nullptr) {}
CaptureTrackData::CaptureTrackData(Track *_target, Module *_recorder) {
	target = _target;
	recorder = _recorder;
}

SignalType CaptureTrackData::type() {
	return target->type;
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
			auto *rec = (AudioRecorder*)d.recorder;

			view->buffer_painter->set_context(l->area);
			view->buffer_painter->set_color(view->colors.capture_marker);

			std::lock_guard<std::mutex> lock(rec->mtx_buf);
			view->update_peaks_now(rec->buf);
			view->buffer_painter->draw_buffer(c, rec->buf, offset);
		} else if (d.type() == SignalType::MIDI) {
			auto *rec = (MidiRecorder*)d.recorder;
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


bool layer_available(TrackLayer *l, const Range &r) {
	for (auto &b: l->buffers)
		if (b.range().overlaps(r))
			return false;
	return true;
}

void ViewModeCapture::insert_midi(Track *target, const MidiEventBuffer &midi, int delay) {
	int s_start = view->get_playback_selection(true).start();

	int i0 = s_start + delay;

	// insert data
	target->layers[0]->insert_midi_data(i0, midi_events_to_notes(midi).duplicate());
}


void ViewModeCapture::insert_audio(Track *target, const AudioBuffer &buf, int delay) {
	Song *song = target->song;

	int s_start = view->get_playback_selection(true).start();
	int i0 = s_start + delay;

	// insert data
	Range r = Range(i0, buf.length);
	song->begin_action_group();

	TrackLayer *layer = nullptr;
	for (TrackLayer *l: target->layers)
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

void ViewModeCapture::insert() {
	song->begin_action_group();
	for (auto &d: data) {
		if (d.type() == SignalType::AUDIO) {
			auto *rec = (AudioRecorder*)d.recorder;
			insert_audio(d.target, rec->buf, 0);
		} else if (d.type() == SignalType::MIDI) {
			auto *rec = (MidiRecorder*)d.recorder;
			insert_midi(d.target, rec->buffer, 0);
		}
	}
	song->end_action_group();
}
